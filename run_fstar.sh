#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

if [ $# -lt 1 ]; then
  echo "Usage: $0 <source_file.fst> [additional F* args]"
  exit 1
fi

SRC_FILE="$1"
shift

FSTAR_BIN="$SCRIPT_DIR/../external_tools/FStarLang/bin/fstar.exe"

exec "$FSTAR_BIN" --include "$SCRIPT_DIR/../external_tools/pulse/out/lib/pulse" --z3version 4.13.3 "$SRC_FILE" "$@"
