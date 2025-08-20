#!/bin/bash
set -euo pipefail

# Set LLVM directory to the one in external tools
HERE=$(dirname "$0")
LLVM_BUILD_DIR="$HERE/build"

#Ensure that LLVM is build first.
if [[ ! -x "$LLVM_BUILD_DIR" ]]; then
  echo "Building LLVM first"
  $HERE/scripts/build_clang.sh
fi

echo "Building F*"
make -C $HERE/external/FStar -j$(nproc) ADMIT=1

echo "Building Pulse"
make -C $HERE/external/pulse -j$(nproc) ADMIT=1 FSTAR_EXE=$(realpath $HERE/external/FStar/bin/fstar.exe)

echo "Building Pulse.Lib.C libraries"
make -C $HERE/include/pulse -j$(nproc)

ninja -C $LLVM_BUILD_DIR c2pulse

echo "Build Successful! Please find the binary C2pulse in the llvm build directory. Use the run.sh script to invoke it from the current directory."
