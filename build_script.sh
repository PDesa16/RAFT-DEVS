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
git clone https://github.com/SimulationEverywhere/cadmium_v2.git
git clone https://github.com/weidai11/cryptopp.git
git clone https://github.com/google/googletest.git
git clone https://github.com/nlohmann/json.git
git clone https://github.com/jbeder/yaml-cpp.git

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
make build_test_raft