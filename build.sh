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

if [[ -x "$CLANG_BIN/c2pulse" ]]; then
  echo "C2Pulse exists in $CLANG_BIN!"
  echo "Rebuilding existing project!"
  cd ../external_tools/llvm-project/build/
  ninja -j $(nproc)
  exit 0
fi

if [[ ! -x "../external_tools/llvm-project/clang/tools/CtoPulse" ]]; then
	ln -s "$(pwd)" ../external_tools/llvm-project/clang/tools/CtoPulse
fi

if grep -q 'add_clang_subdirectory(CtoPulse)' ../external_tools/llvm-project/clang/tools/CMakeLists.txt; then
    echo "CtoPulse already added as a build target!"
else
    echo "Adding CtoPulse as a clang project!"
    echo 'add_clang_subdirectory(CtoPulse)' >> ../external_tools/llvm-project/clang/tools/CMakeLists.txt
fi

cd ../external_tools/llvm-project/build/
ninja -j $(nproc)

echo "Build Successful! Please find the binary C2pulse in the llvm build directory. Use the run.sh script to invoke it from the current directory."
