#!/bin/bash
# Get absolute path of the script directory
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
echo "SCRIPT_DIR: $SCRIPT_DIR"  

# Set paths relative to the script's directory
BUILD_DIR="$SCRIPT_DIR/vendor"
BIN_DIR="$SCRIPT_DIR/bin"

# Build vendor directory
mkdir -p "$BIN_DIR"
mkdir -p "$BUILD_DIR"

# Fetch dependencies
cd "$BUILD_DIR"
# Build dependencies
cd "$BUILD_DIR/cadmium_v2"
./build.sh
cd "$BUILD_DIR/googletest"
cmake .
make -j$(nproc)
cd "$BUILD_DIR/yaml-cpp"
cmake .
make -j$(nproc)
cd "$BUILD_DIR/cryptopp"
make -j$(nproc)

# Make current DEVS project
cd "$PROJECT_ROOT"