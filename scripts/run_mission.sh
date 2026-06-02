#!/bin/bash
# 用法: ./scripts/run_mission.sh [config_path]
set -e
BINARY="$(dirname "$0")/../build/go2_inspection"
CONFIG="${1:-$(dirname "$0")/../config/mission.yaml}"

if [ ! -f "$BINARY" ]; then
    echo "Binary not found, building first..."
    "$(dirname "$0")/build.sh"
fi

echo "==> Starting mission: $CONFIG"
sudo "$BINARY" "$CONFIG"
