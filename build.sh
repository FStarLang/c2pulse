#!/bin/bash
set -euo pipefail

# Set LLVM directory to the one in external tools
LLVM_DIR="$(realpath ../external_tools/llvm-project/build)"
CLANG_BIN="$LLVM_DIR/bin"
BUILD_DIR="./build"
BUILD_TYPE="Debug"

#Ensure that LLVM is build first.
if [[ ! -x "$CLANG_BIN/clang++" ]]; then
  echo "clang++ not found in $CLANG_BIN! Please build llvm first from external_tools."
  exit 1
fi

echo "Sourcing LLVM from: $LLVM_DIR"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Cmake Command
cmake -G Ninja .. \
  -DCMAKE_C_COMPILER="$CLANG_BIN/clang" \
  -DCMAKE_CXX_COMPILER="$CLANG_BIN/clang++" \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DCMAKE_PREFIX_PATH="$LLVM_DIR" \
  -DCMAKE_CXX_FLAGS="-frtti"

# use all cores for ninja
ninja -j"$(nproc)"

echo "Build Successful! Please find the binary C2pulse in the build directory."