#!/bin/bash
set -euo pipefail

clang_version=$(clang --version | grep "clang version" | awk '{print $4}'  | cut -d'-' -f1 | cut -d'.' -f1-3)
SYSTEM_CC_INCLUDE="/usr/lib/clang/$clang_version/include"
echo "Using Clang version: $clang_version"
SYSTEM_INCLUDE="/usr/include"
SYSTEM_ARCH_INCLUDE="/usr/include/x86_64-linux-gnu"
C_STD="-std=c23"

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 <source1.c> [source2.c ...]"
  exit 1
fi


echo "Processing all files: $@"

../external/llvm-project/build/bin/c2pulse "$@" \
  -p ./external/llvm-project/build/ \
  --extra-arg-before=-DC2PULSE \
  --extra-arg-before="-resource-dir" \
  --extra-arg-before="./external/llvm-project/build/lib/clang/21" \
  --extra-arg-before="-isystem" \
  --extra-arg-before="$SYSTEM_CC_INCLUDE" \
  --extra-arg-before="-isystem" \
  --extra-arg-before="$SYSTEM_ARCH_INCLUDE" \
  --extra-arg-before="-isystem" \
  --extra-arg-before="$SYSTEM_INCLUDE" \
  --extra-arg-before="-isystem" \
  --extra-arg-before="./external/llvm-project/build/lib/clang/21/include" \
  --extra-arg-before="-x" \
  --extra-arg-before="c" \
  --extra-arg-before="$C_STD" \
  --extra-arg-before="-c" \
  --extra-arg-before=-fmodules \
  --extra-arg-before=-fimplicit-modules \
  --extra-arg-before=-fmodules-cache-path=/tmp/clang-modules
