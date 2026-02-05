#!/bin/bash

echo "Building for iOS..."

rm -rf build_ios

mkdir -p build_ios
cd build_ios

cmake .. \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0

cmake --build . --config Release

echo "iOS build complete. Output is in build_ios/"
