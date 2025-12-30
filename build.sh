#!/bin/bash

# Simple build script for ULUI project

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}ULUI Build Script${NC}"
echo "===================="

# Configuration
BUILD_TYPE=${1:-Release}
BUILD_DIR="build"

echo -e "${YELLOW}Build Type: ${BUILD_TYPE}${NC}"

# Create build directory
if [ -d "$BUILD_DIR" ]; then
    echo "Build directory exists, cleaning..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
echo -e "${YELLOW}Configuring CMake...${NC}"
cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE

# Build
echo -e "${YELLOW}Building project...${NC}"
cmake --build . --config $BUILD_TYPE

# Success message
echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "Executable location: ${GREEN}$BUILD_DIR/bin/${NC}"
echo ""
echo "To run the application:"
echo "  cd $BUILD_DIR/bin"
echo "  ./ului_app"
