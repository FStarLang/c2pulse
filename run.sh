#!/bin/bash
set -euo pipefail

if [ $# -lt 1 ]; then
  echo "Usage: $0 <source.c> [additional args...]"
  exit 1
fi

SRC_FILE="$1"
shift  # shift out the filename, leave the rest

exec ../external_tools/llvm-project/build/bin/c2pulse "$SRC_FILE" \
  -p build \
  "$@"
