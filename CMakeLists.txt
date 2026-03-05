cmake_minimum_required(VERSION 3.20)
project(comdare-db-licensing-server VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(comdare-db-licensing-server INTERFACE)
target_include_directories(comdare-db-licensing-server INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_library(comdare::db-licensing-server ALIAS comdare-db-licensing-server)

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
endif()

install(TARGETS comdare-db-licensing-server EXPORT comdare-db-licensing-server-targets)
install(DIRECTORY include/ DESTINATION include)
