#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ -n "${COMDARE_BUILDSYSTEM_PATH:-}" ]]; then
  BUILDSYSTEM_DIR="$COMDARE_BUILDSYSTEM_PATH"
elif [[ -f "$SCRIPT_DIR/../../Products/rc-buildsystem-construct/Layer1-Foundation/rc-buildsystem-core/scripts/interface.sh" ]]; then
  BUILDSYSTEM_DIR="$SCRIPT_DIR/../../Products/rc-buildsystem-construct/Layer1-Foundation/rc-buildsystem-core"
elif [[ -f "$SCRIPT_DIR/third_party/rc-buildsystem-core/scripts/interface.sh" ]]; then
  BUILDSYSTEM_DIR="$SCRIPT_DIR/third_party/rc-buildsystem-core"
fi
INTERFACE="$BUILDSYSTEM_DIR/scripts/interface.sh"

if [[ ! -f "$INTERFACE" ]]; then
  echo "[BEP][ERROR] BuildSystem interface not found: $INTERFACE" >&2
  exit 1
fi

exec "$INTERFACE" --consumer-root "$SCRIPT_DIR" "$@"
