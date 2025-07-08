#!/bin/bash
set -euo pipefail

#Default paths to run in case user does not provide any paths.
DEFAULT_PATHS=(
  "./test/general/"
  "./test/issue-related/"
)

SHOULD_FAIL=(
 "./test/issue-related/issue27_test.c"
 "./test/issue-related/issue22_test.c"
 "./test/issue-related/issue32_test.c"
)

is_expected_to_fail() {
  for file in "${SHOULD_FAIL[@]}"; do
    if [[ "$file" == "$1" ]]; then
      return 0
    fi
  done
  return 1
}

# Check if any arguments are provided, if not use default paths.
if [ "$#" -lt 1 ]; then
  echo "The user did not provide any paths, running tests on known directories!"
  set -- "${DEFAULT_PATHS[@]}"
fi

HERE=$(dirname "$0")

FAILED_FILES=()

# Function to process a single .c file
process_c_file() {
  local cfile="$1"
  echo "Processing: $cfile"
 
  if bash $HERE/scripts/run_test.sh "$cfile" &> /dev/null ; then
    echo "  ✔ Success: $cfile"
  else
    if is_expected_to_fail $cfile; then
	    echo " ✔ Success expected failure: $cfile"
    else
	    echo "  ❌ Failed: $cfile"
	    FAILED_FILES+=("$cfile")
    fi
  fi
}

normalize_path() {
  echo "${1%/}"
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
    clean_path=$(normalize_path "$path")
    for cfile in "$clean_path"/*.c; do
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
