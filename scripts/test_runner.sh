#!/bin/bash
set -euo pipefail

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 <source1.c | directory>"
  exit 1
fi

HERE=$(dirname "$0")

FAILED_FILES=()

# Function to process a single .c file
process_c_file() {
  local cfile="$1"
  echo "Processing: $cfile"
 
  if bash $HERE/run_test.sh "$cfile" &> /dev/null ; then
    echo "  ✔ Success: $cfile"
    python3 create_lit_tests.py "$cfile" 
  else
    echo "  ❌ Failed: $cfile"
    FAILED_FILES+=("$cfile")
  fi
}

echo "▶ Generating Pulse code from C code and running F* on them"
echo "$@"
for path in "$@"; do
  if [ -f "$path" ] && [[ "$path" == *.c ]]; then
    echo "Processing C file: $path"
    process_c_file "$path"
  elif [ -d "$path" ]; then
    echo "Processing .c files in directory: $path"
    shopt -s nullglob
    for cfile in "$path"/*.c; do
      process_c_file "$cfile"
    done
    shopt -u nullglob
  else
    echo "⚠️  Skipping unsupported path: $path"
  fi
done

echo
if [ "${#FAILED_FILES[@]}" -ne 0 ]; then
  echo "‼️  Some files failed to process:"
  for file in "${FAILED_FILES[@]}"; do
    echo "  ❌ $file"
  done
  exit 1
else
  echo "✔ All files processed successfully."
fi