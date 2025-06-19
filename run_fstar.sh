#!/bin/bash
set -euo pipefail

if [ $# -lt 1 ]; then
  echo "Usage: $0 <source_file.fst> [additional F* args]"
  exit 1
fi

SRC_FILE="$1"
shift

exec ../external_tools/FStarLang/bin/fstar.exe \
  --include ../external_tools/pulse/out/lib/pulse --z3version 4.13.3 "$SRC_FILE" "$@"

