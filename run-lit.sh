#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# Usage: ./run-lit.sh /path/to/test

# Check if the user provided a path
if [ $# -lt 1 ]; then
  echo "Usage: $0 /path/to/test"
  exit 1
fi

TEST_PATH="$1"
shift

LIT_BIN="$SCRIPT_DIR/../external_tools/llvm-project/build/bin/llvm-lit"

# Run llvm-lit with verbose output
"$LIT_BIN" -v "$TEST_PATH" "$@"
