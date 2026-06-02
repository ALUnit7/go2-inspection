#!/bin/bash
# 将项目同步到 Jetson 并远程编译
# 用法: ./scripts/deploy_jetson.sh [jetson_ip]
set -e
JETSON_IP="${1:-192.168.123.18}"
JETSON_USER="unitree"
REMOTE_DIR="/home/unitree/go2_inspection"

echo "==> Syncing to ${JETSON_USER}@${JETSON_IP}:${REMOTE_DIR}"
rsync -avz --exclude build --exclude .git --exclude models/*.onnx     "$(dirname "$0")/../" "${JETSON_USER}@${JETSON_IP}:${REMOTE_DIR}/"

echo "==> Remote build"
ssh "${JETSON_USER}@${JETSON_IP}" "
    cd ${REMOTE_DIR} && mkdir -p build && cd build &&
    cmake .. -DCMAKE_BUILD_TYPE=Release &&
    make -j\$(nproc)
"
echo "==> Deploy done"
