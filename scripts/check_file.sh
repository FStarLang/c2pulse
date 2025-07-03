#!/bin/bash
set -euo pipefail

# Directory where the script resides
HERE="$(cd "$(dirname "$0")" && pwd)"

# List of files to check
# Usage: ./check_file.sh file1.fst file2.fsti ...
if [ "$#" -lt 1 ]; then
  echo "Usage: $0 <file1.fst|fsti> [file2.fst|fsti] ..."
  exit 1
fi

SRC_FILES=("$@")

echo "Checking if the generated file matches the expected output..."

for file in "${SRC_FILES[@]}"; do
  # Extract base name and capitalize first letter for snapshot lookup
  name="$(basename "$file")"
  capitalized="${name^}"
  snapshot="$(realpath "$HERE/../test/snapshots/$capitalized")"

  echo "  Checking: [ $file ] against snapshot [ $snapshot ]"

  if diff_output=$(diff -u "$file" "$snapshot"); then
    echo "    ✔ OK: $file matches snapshot."
    python3 create_lit_tests.py "$file" 
  else
    echo "    ❌ Mismatch found in $file!"
    echo "$diff_output"
    exit 1
  fi
done

echo "✔ All files match their snapshots."
