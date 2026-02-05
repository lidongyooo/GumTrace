#!/bin/bash

ANDROID_NDK_HOME=/Users/lidongyooo/Library/Android/sdk/ndk/25.0.8775105

# Check if ANDROID_NDK_HOME is set
if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "Please set it to your Android NDK installation path."
    exit 1
fi

echo "Building for Android (arm64-v8a)..."

rm -rf build_ios
mkdir -p build_android
cd build_android

cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-25 \
    -DCMAKE_BUILD_TYPE=Release

cmake --build .

echo "Android build complete. Output is in build_android/"
