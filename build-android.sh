#!/bin/bash

# Build script for Android native library

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}ULUI Android Build Script${NC}"
echo "================================"
echo

# Check for Android NDK
if [ -z "$ANDROID_NDK" ]; then
    echo -e "${RED}Error: ANDROID_NDK environment variable is not set${NC}"
    echo "Please set it to your Android NDK installation path:"
    echo "  export ANDROID_NDK=/path/to/android-ndk"
    exit 1
fi

if [ ! -d "$ANDROID_NDK" ]; then
    echo -e "${RED}Error: Android NDK not found at: $ANDROID_NDK${NC}"
    exit 1
fi

echo -e "${GREEN}Using Android NDK: $ANDROID_NDK${NC}"
echo

# Configuration
BUILD_TYPE=${1:-Release}
API_LEVEL=${2:-21}
ABIS=("arm64-v8a" "armeabi-v7a")

echo "Build Type: $BUILD_TYPE"
echo "API Level: $API_LEVEL"
echo "ABIs: ${ABIS[@]}"
echo

# Build for each ABI
for ABI in "${ABIS[@]}"; do
    echo -e "${YELLOW}Building for $ABI...${NC}"
    
    BUILD_DIR="build-android-$ABI"
    
    # Clean if exists
    if [ -d "$BUILD_DIR" ]; then
        echo "Cleaning existing build directory..."
        rm -rf "$BUILD_DIR"
    fi
    
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Configure
    echo "Configuring CMake..."
    cmake .. \
        -DCMAKE_SYSTEM_NAME=Android \
        -DCMAKE_ANDROID_ARCH_ABI=$ABI \
        -DCMAKE_ANDROID_NDK=$ANDROID_NDK \
        -DCMAKE_ANDROID_STL_TYPE=c++_shared \
        -DCMAKE_ANDROID_API=$API_LEVEL \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DUSE_ANGLE=ON
    
    # Build
    echo "Building..."
    cmake --build . -j$(nproc)
    
    cd ..
    
    echo -e "${GREEN}✓ Built for $ABI${NC}"
    echo
done

# Create output directory structure
OUTPUT_DIR="android/app/src/main/jniLibs"
mkdir -p "$OUTPUT_DIR"

# Copy libraries
echo -e "${YELLOW}Copying libraries to Android project...${NC}"
for ABI in "${ABIS[@]}"; do
    ABI_DIR="$OUTPUT_DIR/$ABI"
    mkdir -p "$ABI_DIR"
    
    if [ -f "build-android-$ABI/lib/libului_app.so" ]; then
        cp "build-android-$ABI/lib/libului_app.so" "$ABI_DIR/"
        echo -e "${GREEN}✓ Copied libului_app.so for $ABI${NC}"
    else
        echo -e "${RED}✗ Library not found for $ABI${NC}"
    fi
done

# Copy shaders to assets
ASSETS_DIR="android/app/src/main/assets"
mkdir -p "$ASSETS_DIR"
if [ -d "shaders" ]; then
    cp -r shaders "$ASSETS_DIR/"
    echo -e "${GREEN}✓ Copied shaders to assets${NC}"
fi

echo
echo -e "${GREEN}================================${NC}"
echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "${GREEN}================================${NC}"
echo
echo "Native libraries are in: $OUTPUT_DIR"
echo "Shaders are in: $ASSETS_DIR/shaders"
echo
echo "To build the Android APK:"
echo "  cd android"
echo "  ./gradlew assembleDebug"
echo
echo "To install on device:"
echo "  cd android"
echo "  ./gradlew installDebug"
