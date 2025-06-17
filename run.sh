#!/bin/bash
set -e

if [ $# -lt 1 ]; then
  echo "Usage: $0 <source.c> [additional args...]"
  exit 1
fi

SRC_FILE="$1"
shift  # Shift out the filename, leaving any extra args behind

./build/c2pulse "$SRC_FILE" \
  -p build \
  --extra-arg-before="-resource-dir" \
  --extra-arg-before="../external_tools/llvm-project/build/lib/clang/21" \
  --extra-arg-before="-x" \
  --extra-arg-before="c" \
  --extra-arg-before="-std=c11" \
  --extra-arg-before="-c" \
  "$@"

