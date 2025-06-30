#!/bin/bash
set -euo pipefail

HERE=$(dirname "$0")
LLVM_DIR="$(realpath $HERE/external/llvm-project/)"
LLVM_BUILD_DIR="$LLVM_DIR/build"
CLANG_BIN="$LLVM_BUILD_DIR/bin"

mkdir -p $LLVM_BUILD_DIR

pushd $LLVM_BUILD_DIR
# Note: I'm removing the following
#   -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
#   -DCMAKE_LINKER=lld -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld"
# making clang build with the system compiler, whatever that is,
# so we transparently use ccache.
cmake -G Ninja  -DCMAKE_BUILD_TYPE=Debug -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON -DLLVM_PARALLEL_LINK_JOBS=2 -DLLVM_BUILD_LLVM_DYLIB=ON -DLLVM_LINK_LLVM_DYLIB=ON -DLLVM_ENABLE_RTTI=ON -DLLVM_ENABLE_LLD=ON  -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" -DLLVM_TARGETS_TO_BUILD="X86" ../llvm
popd

ninja -C $LLVM_BUILD_DIR
