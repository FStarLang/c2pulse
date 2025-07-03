#!/bin/bash
set -euo pipefail

HERE=$(dirname "$0")
LLVM_DIR="$(realpath $HERE/../external/llvm-project/)"
LLVM_BUILD_DIR="$LLVM_DIR/build"
LIT_BIN="$LLVM_BUILD_DIR/bin/llvm-lit"

# Check if the user provided a path
if [ $# -lt 1 ]; then
  echo "Usage: $0 /path/to/test"
  exit 1
fi

TEST_PATH="$1"
shift

# Run llvm-lit with verbose output
"$LIT_BIN" -v "$TEST_PATH" "$@"
