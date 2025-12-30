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
ABIS=("arm64-v8a")  # Only arm64-v8a, no armv7 support

echo "Build Type: $BUILD_TYPE"
echo "API Level: $API_LEVEL"
echo "ABIs: ${ABIS[@]}"
echo

# Build shared library for each ABI
for ABI in "${ABIS[@]}"; do
    echo -e "${YELLOW}Building SHARED LIBRARY for $ABI...${NC}"
    
    BUILD_DIR="build-android-$ABI"
    
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
        -DUSE_ANGLE=ON
    
    # Build
    echo "Building shared library..."
    cmake --build . -j$(nproc)
    
    cd ..
    
    echo -e "${GREEN}✓ Built shared library for $ABI${NC}"
    echo
done

# Create output directories and copy to all example projects
echo
echo -e "${YELLOW}Copying libraries to example projects...${NC}"

# List of example project directories
EXAMPLE_PROJECTS=(
    "android/example-nativeactivity-so-gl"
    "android/example-nativeactivity-app-gl"
    "android/example-gameactivity-so-gl"
    "android/example-gameactivity-app-gl"
    "android/example-java-so-gl"
    "android/example-java-app-gl"
)

for ABI in "${ABIS[@]}"; do
    if [ -f "build-android-$ABI/lib/libului_app.so" ]; then
        # Copy to each example project
        for PROJECT in "${EXAMPLE_PROJECTS[@]}"; do
            ABI_DIR="$PROJECT/app/src/main/jniLibs/$ABI"
            mkdir -p "$ABI_DIR"
            cp "build-android-$ABI/lib/libului_app.so" "$ABI_DIR/"
            echo -e "${GREEN}✓ Copied libului_app.so for $ABI to $PROJECT${NC}"
        done
    else
        echo -e "${RED}✗ Shared library not found for $ABI${NC}"
    fi
done

# Copy shaders to assets for all example projects
if [ -d "shaders" ]; then
    for PROJECT in "${EXAMPLE_PROJECTS[@]}"; do
        ASSETS_DIR="$PROJECT/app/src/main/assets"
        mkdir -p "$ASSETS_DIR"
        cp -r shaders "$ASSETS_DIR/"
    done
    echo -e "${GREEN}✓ Copied shaders to all example projects${NC}"
fi

echo
echo -e "${GREEN}================================${NC}"
echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "${GREEN}================================${NC}"
echo

echo -e "${YELLOW}Shared Library (libului_app.so):${NC}"
echo "  Architectures: ${ABIS[@]}"
echo "  Copied to 6 example projects in android/"
echo
echo "To build and run example projects:"
echo "  cd android/example-nativeactivity-so-gl && ./gradlew installDebug"
echo "  cd android/example-nativeactivity-app-gl && ./gradlew installDebug"
echo "  cd android/example-gameactivity-so-gl && ./gradlew installDebug"
echo "  cd android/example-gameactivity-app-gl && ./gradlew installDebug"
echo "  cd android/example-java-so-gl && ./gradlew installDebug"
echo "  cd android/example-java-app-gl && ./gradlew installDebug"
echo
