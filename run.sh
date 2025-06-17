#!/bin/bash
set -euo pipefail

if [ $# -lt 1 ]; then
  echo "Usage: $0 <source.c> [additional args...]"
  exit 1
fi

SRC_FILE="$1"
shift  # shift out the filename, leave the rest

CLANG_RESOURCE="../external_tools/llvm-project/build/lib/clang/21"
CLANG_BUILTIN="$CLANG_RESOURCE/include"

# Validate Clang resource directory exists
if [ ! -f "$CLANG_BUILTIN/stddef.h" ]; then
  echo "Missing Clang built-in headers at: $CLANG_BUILTIN"
  echo "Try running 'make install' in your LLVM build to generate them."
  exit 1
fi

exec ./build/c2pulse "$SRC_FILE" \
  -p build \
  --extra-arg-before=-resource-dir \
  --extra-arg-before="$CLANG_RESOURCE" \
  --extra-arg-before=-isystem --extra-arg-before="/usr/lib/clang/18.1.3/include" \
  --extra-arg-before=-isystem --extra-arg-before="/usr/local/include" \
  --extra-arg-before=-isystem --extra-arg-before="/usr/include/x86_64-linux-gnu" \
  --extra-arg-before=-isystem --extra-arg-before="/usr/include" \
  --extra-arg-before=-isystem --extra-arg-before="$CLANG_BUILTIN" \
  --extra-arg-before=-x --extra-arg-before=c \
  --extra-arg-before=-std=c11 \
  --extra-arg-before=-c \
  "$@"
