#!/bin/bash
set -euo pipefail

# Set LLVM directory to the one in external tools
HERE=$(dirname "$0")
LLVM_DIR="$(realpath $HERE/external/llvm-project/)"
LLVM_BUILD_DIR="$LLVM_DIR/build"
CLANG_BIN="$LLVM_BUILD_DIR/bin"

#Ensure that LLVM is build first.
if [[ ! -x "$CLANG_BIN/clang++" ]]; then
  echo "clang++ not found in $CLANG_BIN!"
  echo "Building LLVM first"
  $HERE/scripts/build_clang.sh
fi

echo "Building F*"
make -C $HERE/external/FStar -j$(nproc) ADMIT=1

echo "Building Pulse"
make -C $HERE/external/pulse -j$(nproc) ADMIT=1 FSTAR_EXE=$(realpath $HERE/external/FStar/bin/fstar.exe)

echo "Building Pulse.Lib.C libraries"
make -C $HERE/include/pulse -j$(nproc)

if [[ ! -x "$LLVM_DIR/clang/tools/c2pulse" ]]; then
	ln -s "$(pwd)" $LLVM_DIR/clang/tools/c2pulse
fi

if grep -q 'add_clang_subdirectory(c2pulse)' $LLVM_DIR/clang/tools/CMakeLists.txt; then
    echo "CtoPulse already added as a build target!"
else
    echo "Adding CtoPulse as a clang project!"
    echo 'add_clang_subdirectory(c2pulse)' >> $LLVM_DIR/clang/tools/CMakeLists.txt
fi

ninja -C $LLVM_BUILD_DIR -j $(nproc)

echo "Build Successful! Please find the binary C2pulse in the llvm build directory. Use the run.sh script to invoke it from the current directory."
