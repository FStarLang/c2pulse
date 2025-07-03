#!/bin/bash
set -euo pipefail

HERE=$(dirname $0)
FSTAR_BIN="$(realpath $HERE/../external/FStar/bin/fstar.exe)"
PULSE_DIR="$(realpath $HERE/../external/pulse/out/lib/pulse)"
AUX_LIBS="$(realpath $HERE/../include/pulse)"

if [ $# -lt 1 ]; then
  echo "Usage: $0 <source_file.fst> [additional F* args]"
  exit 1
fi

SRC_FILE="$1"
shift

exec "$FSTAR_BIN" --include "$PULSE_DIR" --include "$AUX_LIBS" --z3version 4.13.3 "$SRC_FILE" "$@"
