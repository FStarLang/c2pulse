#!/bin/bash
set -euo pipefail

HERE="$(dirname "$0")/../"

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 <source1.c> [source2.c ...]"
  exit 1
fi


echo "Processing all files: $@"

exec $HERE/build/bin/c2pulse "$@"
