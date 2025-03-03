#!/bin/bash
# Set working directory to the project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$(dirname "$SCRIPT_DIR/vendor")"
cd "$PROJECT_ROOT" || exit
# Build vendor directory
mkdir -p "$BUILD_DIR"

# Fetch dependencies
cd "$BUILD_DIR"
git clone https://github.com/SimulationEverywhere/cadmium_v2.git
git clone https://github.com/weidai11/cryptopp.git
git clone https://github.com/google/googletest.git
git clone https://github.com/nlohmann/json.git
git clone https://github.com/jbeder/yaml-cpp.git
# Build dependencies
cd "$BUILD_DIR/googletest"
mkdir build
cmake ..
cd "$BUILD_DIR/yaml-cpp"
mkdir build
cmake ..
cd "$BUILD_DIR/cryptopp"
make .
# Make current DEVS project
cd "$PROJECT_ROOT"
make build_test_raft
