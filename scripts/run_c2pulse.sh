#!/bin/bash
set -euo pipefail

HERE="$(dirname "$0")/../"

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 <source1.c> [source2.c ...]"
  exit 1
fi


echo "Processing all files: $@"

$HERE/external/llvm-project/build/bin/c2pulse "$@" \
  --extra-arg-before=-DC2PULSE \
  --extra-arg-before=-include \
  --extra-arg-before=$(realpath "$HERE/test/include/PulseMacros.h")
