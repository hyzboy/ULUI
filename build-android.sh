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
BUILD_MODE=${2:-both}  # Options: executable, shared, both
API_LEVEL=${3:-21}
ABIS=("arm64-v8a")  # Only arm64-v8a, no armv7 support

echo "Build Type: $BUILD_TYPE"
echo "Build Mode: $BUILD_MODE"
echo "API Level: $API_LEVEL"
echo "ABIs: ${ABIS[@]}"
echo

# Determine which builds to do
BUILD_EXECUTABLE=false
BUILD_SHARED=false

case $BUILD_MODE in
    executable)
        BUILD_EXECUTABLE=true
        ;;
    shared)
        BUILD_SHARED=true
        ;;
    both)
        BUILD_EXECUTABLE=true
        BUILD_SHARED=true
        ;;
    *)
        echo -e "${RED}Invalid build mode: $BUILD_MODE${NC}"
        echo "Valid options: executable, shared, both"
        exit 1
        ;;
esac

# Build for each ABI and mode
for ABI in "${ABIS[@]}"; do
    # Build executable if requested
    if [ "$BUILD_EXECUTABLE" = true ]; then
        echo -e "${YELLOW}Building EXECUTABLE for $ABI...${NC}"
        
        BUILD_DIR="build-android-exe-$ABI"
        
        # Clean if exists
        if [ -d "$BUILD_DIR" ]; then
            echo "Cleaning existing build directory..."
            rm -rf "$BUILD_DIR"
        fi
        
        mkdir -p "$BUILD_DIR"
        cd "$BUILD_DIR"
        
        # Configure
        echo "Configuring CMake for executable..."
        cmake .. \
            -DCMAKE_SYSTEM_NAME=Android \
            -DCMAKE_ANDROID_ARCH_ABI=$ABI \
            -DCMAKE_ANDROID_NDK=$ANDROID_NDK \
            -DCMAKE_ANDROID_STL_TYPE=c++_shared \
            -DCMAKE_ANDROID_API=$API_LEVEL \
            -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
            -DUSE_ANGLE=ON \
            -DANDROID_BUILD_SHARED=OFF
        
        # Build
        echo "Building executable..."
        cmake --build . -j$(nproc)
        
        cd ..
        
        echo -e "${GREEN}✓ Built executable for $ABI${NC}"
        echo
    fi
    
    # Build shared library if requested
    if [ "$BUILD_SHARED" = true ]; then
        echo -e "${YELLOW}Building SHARED LIBRARY for $ABI...${NC}"
        
        BUILD_DIR="build-android-so-$ABI"
        
        # Clean if exists
        if [ -d "$BUILD_DIR" ]; then
            echo "Cleaning existing build directory..."
            rm -rf "$BUILD_DIR"
        fi
        
        mkdir -p "$BUILD_DIR"
        cd "$BUILD_DIR"
        
        # Configure
        echo "Configuring CMake for shared library..."
        cmake .. \
            -DCMAKE_SYSTEM_NAME=Android \
            -DCMAKE_ANDROID_ARCH_ABI=$ABI \
            -DCMAKE_ANDROID_NDK=$ANDROID_NDK \
            -DCMAKE_ANDROID_STL_TYPE=c++_shared \
            -DCMAKE_ANDROID_API=$API_LEVEL \
            -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
            -DUSE_ANGLE=ON \
            -DANDROID_BUILD_SHARED=ON
        
        # Build
        echo "Building shared library..."
        cmake --build . -j$(nproc)
        
        cd ..
        
        echo -e "${GREEN}✓ Built shared library for $ABI${NC}"
        echo
    fi
done

# Create output directories
echo
echo -e "${YELLOW}Organizing output files...${NC}"

# Create separate directories for each build type
EXECUTABLE_DIR="android-executable"
SHARED_DIR="android-shared"

if [ "$BUILD_EXECUTABLE" = true ]; then
    mkdir -p "$EXECUTABLE_DIR"
    for ABI in "${ABIS[@]}"; do
        if [ -f "build-android-exe-$ABI/bin/ului_app" ]; then
            mkdir -p "$EXECUTABLE_DIR/$ABI"
            cp "build-android-exe-$ABI/bin/ului_app" "$EXECUTABLE_DIR/$ABI/"
            echo -e "${GREEN}✓ Copied executable for $ABI to $EXECUTABLE_DIR/$ABI/${NC}"
        fi
    done
    
    # Copy shaders for executable
    if [ -d "shaders" ]; then
        cp -r shaders "$EXECUTABLE_DIR/"
        echo -e "${GREEN}✓ Copied shaders to $EXECUTABLE_DIR/${NC}"
    fi
fi

if [ "$BUILD_SHARED" = true ]; then
    # Copy to Android project structure for shared library
    OUTPUT_DIR="android/app/src/main/jniLibs"
    mkdir -p "$OUTPUT_DIR"
    
    for ABI in "${ABIS[@]}"; do
        ABI_DIR="$OUTPUT_DIR/$ABI"
        mkdir -p "$ABI_DIR"
        
        if [ -f "build-android-so-$ABI/lib/libului_app.so" ]; then
            cp "build-android-so-$ABI/lib/libului_app.so" "$ABI_DIR/"
            echo -e "${GREEN}✓ Copied libului_app.so for $ABI to $ABI_DIR/${NC}"
        else
            echo -e "${RED}✗ Shared library not found for $ABI${NC}"
        fi
    done
    
    # Copy shaders to assets for shared library
    ASSETS_DIR="android/app/src/main/assets"
    mkdir -p "$ASSETS_DIR"
    if [ -d "shaders" ]; then
        cp -r shaders "$ASSETS_DIR/"
        echo -e "${GREEN}✓ Copied shaders to $ASSETS_DIR/${NC}"
    fi
fi

echo
echo -e "${GREEN}================================${NC}"
echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "${GREEN}================================${NC}"
echo

if [ "$BUILD_EXECUTABLE" = true ]; then
    echo -e "${YELLOW}Android Executable:${NC}"
    echo "  Location: $EXECUTABLE_DIR/"
    echo "  Architectures: ${ABIS[@]}"
    echo "  To deploy: adb push $EXECUTABLE_DIR/arm64-v8a/ului_app /data/local/tmp/"
    echo "  To run: adb shell /data/local/tmp/ului_app"
    echo
fi

if [ "$BUILD_SHARED" = true ]; then
    echo -e "${YELLOW}Android Shared Library (NativeActivity):${NC}"
    echo "  Location: android/app/src/main/jniLibs/"
    echo "  To build APK: cd android && ./gradlew assembleDebug"
    echo "  To install: cd android && ./gradlew installDebug"
    echo
fi
