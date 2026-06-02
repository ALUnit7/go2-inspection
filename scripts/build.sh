#!/bin/bash
set -e
cd "$(dirname "$0")/.."
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
echo "Build OK: build/go2_inspection"
