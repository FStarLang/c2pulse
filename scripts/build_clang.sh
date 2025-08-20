#!/bin/bash
set -euo pipefail

HERE=$(realpath $(dirname "$0")/..)
LLVM_DIR="$(realpath $HERE/external/llvm-project/)"
LLVM_BUILD_DIR="$HERE/build"

mkdir -p $LLVM_BUILD_DIR

# Default to Debug build, but allow external override. The CI will use
# MinSizeRel to save on disk space (~60GB -> ~10GB).
C2PULSE_BUILD_TYPE=${C2PULSE_BUILD_TYPE:-RelWithDebInfo}

pushd $LLVM_BUILD_DIR
# Note: I'm removing the following
#   -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
#   -DCMAKE_LINKER=lld -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld"
# making clang build with the system compiler, whatever that is,
# so we transparently use ccache.
cmake -G Ninja \
    -DLLVM_EXTERNAL_CLANG_TOOLS_EXTRA_SOURCE_DIR="$HERE" \
    -DCMAKE_BUILD_TYPE=$C2PULSE_BUILD_TYPE \
    -DLLVM_CCACHE_BUILD=ON \
    -DLLVM_ENABLE_ASSERTIONS=ON \
    -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
    -DLLVM_PARALLEL_LINK_JOBS=2 \
    -DLLVM_BUILD_LLVM_DYLIB=ON \
    -DLLVM_LINK_LLVM_DYLIB=ON \
    -DLLVM_ENABLE_RTTI=ON \
    -DLLVM_ENABLE_LLD=ON \
    -DLLVM_ENABLE_LIBXML2=OFF \
    -DLLVM_ENABLE_LIBEDIT=OFF \
    -DLLVM_ENABLE_LIBPFM=OFF \
    -DLLVM_ENABLE_OCAMLDOC=OFF \
    -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" \
    -DLLVM_TARGETS_TO_BUILD="X86" \
    -DLLVM_ENABLE_RUNTIMES="" \
    -DLLVM_ENABLE_BINDINGS=OFF \
    -DLLVM_INCLUDE_BENCHMARKS=OFF \
    -DLLVM_INCLUDE_EXAMPLES=OFF \
    -DLLVM_INCLUDE_TESTS=OFF \
    -DLLVM_INCLUDE_UTILS=OFF \
    $LLVM_DIR/llvm
popd

ninja -C $LLVM_BUILD_DIR c2pulse
