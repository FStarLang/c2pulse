#!/bin/bash
set -euo pipefail

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 <source1.c | directory>"
  exit 1
fi

HERE=$(dirname "$0")
SNAPSHOT_DIR="$(realpath "$HERE/../test/snapshots")"

echo "----------------------: " $SNAPSHOT_DIR

# Ensure snapshot directory exists
mkdir -p "$SNAPSHOT_DIR"

FAILED_FILES=()

echo "Snapshot output directory: $SNAPSHOT_DIR"

# Function to process a single .c file
process_c_file() {
  local cfile
  cfile="$(realpath "$1")"
  local fst_file="${cfile%.c}.fst"
  local fst_dirname
  fst_dirname="$(dirname "$fst_file")"
  local fst_basename
  fst_basename="$(basename "$fst_file")"
  local capitalized_name="${fst_basename^}"
  local capitalized_path="$fst_dirname/$capitalized_name"

  echo "▶ Processing: $cfile"
  
  if bash "$HERE/../run.sh" "$cfile" &> /dev/null; then
    echo "  ✔ Success"
    
    if [ -f "$capitalized_path" ]; then
      echo "  Copying: $capitalized_name → $SNAPSHOT_DIR/"
      cp "$capitalized_path" "$SNAPSHOT_DIR/" || echo "  ❌ Failed to copy"
    else
      echo "  ❌ F* output not found: $capitalized_path"
    fi
  else
    echo "  ❌ Failed to process."
    FAILED_FILES+=("$cfile")
  fi
}


echo "▶ Generating Pulse code from C code and running F* on them"
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
echo "Pulse files copied to: $SNAPSHOT_DIR"