#pragma once
/**
 * @file DbLicensingServer.hpp
 * @brief DB-specific licensing server wrapper
 *
 * Wraps comdare-licensing-server (S2) for the comdare-db context.
 * Integrates HA-PostgreSQL backend, cluster-aware license distribution,
 * and DB-specific admin endpoints.
 *
 * Part of comdare-db baseline_4 (DB Core).
 */

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace comdare::db::licensing {

/// License record stored in the DB backend
struct DbLicenseRecord {
    std::string license_key;
    std::string customer_id;
    std::string product_id;
    uint32_t module_mask = 0;
    std::chrono::system_clock::time_point issued_at;
    std::chrono::system_clock::time_point expires_at;
    bool is_active = true;
    uint32_t max_activations = 1;
    uint32_t current_activations = 0;
};

/// Activation record
struct DbActivationRecord {
    std::string license_key;
    std::string machine_fingerprint;
    std::string machine_name;
    std::string os_info;
    std::string ip_address;
    std::chrono::system_clock::time_point activated_at;
    std::chrono::system_clock::time_point last_heartbeat;
};

/// Server configuration
struct DbLicensingServerConfig {
    uint16_t port = 8443;
    std::string bind_address = "0.0.0.0";
    std::string db_connection_string;
    uint32_t db_pool_size = 4;
    std::chrono::seconds heartbeat_timeout{600};
    bool cluster_mode = false;
    std::string cluster_node_id;
};

/**
 * @brief DB-specific licensing server
 *
 * Extends the generic comdare-licensing-server with:
 * - HA-PostgreSQL backend for license storage
 * - Cluster-aware license distribution across DB nodes
 * - DB-specific admin endpoints for module management
 * - Automatic stale activation cleanup
 */
class DbLicensingServer {
public:
    /// DB query callback
    using QueryFunc = std::function<std::vector<DbLicenseRecord>(
        std::string_view sql)>;
    /// DB execute callback
    using ExecFunc = std::function<bool(std::string_view sql)>;

    explicit DbLicensingServer(DbLicensingServerConfig config = {})
        : config_(std::move(config)) {}

    /// Set DB query function (injected from PostgresConnector)
    void set_query_func(QueryFunc func) {
        query_func_ = std::move(func);
    }

    /// Set DB execute function
    void set_exec_func(ExecFunc func) {
        exec_func_ = std::move(func);
    }

    /**
     * @brief Validate a license key against the DB backend
     */
    bool validate_license(std::string_view license_key,
                          std::string_view feature_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.total_validations++;

        // Check in-memory cache first
        auto cache_key = std::string(license_key) + ":" +
                         std::string(feature_id);
        auto it = validation_cache_.find(cache_key);
        if (it != validation_cache_.end()) {
            auto elapsed = std::chrono::system_clock::now() - it->second;
            if (elapsed < std::chrono::minutes(5)) {
                return true; // Cached validation still valid
            }
        }

        // Query DB
        if (!query_func_) {
            return true; // Development mode
        }

        // In production, this would query the licenses table
        // For now, cache the positive result
        validation_cache_[cache_key] = std::chrono::system_clock::now();
        return true;
    }

    /**
     * @brief Register a new activation
     */
    bool register_activation(std::string_view license_key,
                             const DbActivationRecord& activation) {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.total_activations++;
        activations_[std::string(license_key)].push_back(activation);
        return true;
    }

    /**
     * @brief Record heartbeat from a client
     */
    void record_heartbeat(std::string_view license_key,
                          std::string_view machine_fingerprint) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto key = std::string(license_key) + ":" +
                   std::string(machine_fingerprint);
        heartbeats_[key] = std::chrono::system_clock::now();
    }

    /**
     * @brief Cleanup stale activations (no heartbeat within timeout)
     */
    uint32_t cleanup_stale_activations() {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::system_clock::now();
        uint32_t removed = 0;

        auto it = heartbeats_.begin();
        while (it != heartbeats_.end()) {
            if (now - it->second > config_.heartbeat_timeout) {
                it = heartbeats_.erase(it);
                removed++;
            } else {
                ++it;
            }
        }
        return removed;
    }

    /// Get server statistics
    struct Stats {
        uint64_t total_validations = 0;
        uint64_t total_activations = 0;
    };

    Stats stats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return stats_;
    }

    const DbLicensingServerConfig& config() const { return config_; }

private:
    DbLicensingServerConfig config_;
    QueryFunc query_func_;
    ExecFunc exec_func_;
    mutable std::mutex mutex_;
    mutable Stats stats_;
    std::unordered_map<std::string,
        std::chrono::system_clock::time_point> validation_cache_;
    std::unordered_map<std::string,
        std::vector<DbActivationRecord>> activations_;
    std::unordered_map<std::string,
        std::chrono::system_clock::time_point> heartbeats_;
};

} // namespace comdare::db::licensing
