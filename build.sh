#!/usr/bin/env bash
set -Eeuo pipefail

# -----------------------------
# Helpers
# -----------------------------
here() (
  cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd
)

cores() {
  if command -v nproc >/dev/null 2>&1; then
    nproc
  elif [[ "$(uname -s)" == "Darwin" ]]; then
    sysctl -n hw.ncpu
  else
    printf '%s\n' 8
  fi
}

abspath() {
  # Portable absolute path (no GNU readlink -f dependency)
  python3 - "$1" <<'PY'
import os, sys
print(os.path.abspath(sys.argv[1]))
PY
}

# -----------------------------
# Setup
# -----------------------------
HERE="$(here)"
BUILD_DIR="${BUILD_DIR:-$HERE/build}"
JOBS="${JOBS:-$(cores)}"
CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-RelWithDebInfo}"

# Sanity checks
command -v cmake >/dev/null 2>&1 || { echo "cmake not found. Please install CMake."; exit 1; }
command -v ninja >/dev/null 2>&1 || { echo "ninja not found. Please install Ninja."; exit 1; }

# Ensure build dir exists
mkdir -p "$BUILD_DIR"

# If previously configured with a different generator, wipe it
if [[ -f "$BUILD_DIR/CMakeCache.txt" ]]; then
  GEN="$(sed -n 's/^CMAKE_GENERATOR:INTERNAL=//p' "$BUILD_DIR/CMakeCache.txt" || true)"
  if [[ -n "${GEN:-}" && "$GEN" != "Ninja" ]]; then
    echo "Reconfiguring with Ninja (found previous generator: $GEN). Cleaning build dir..."
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
  fi
fi

# Configure if missing or incomplete
if [[ ! -f "$BUILD_DIR/build.ninja" ]]; then
  echo "Running CMake configure..."
  cmake -S "$HERE" -B "$BUILD_DIR" -G Ninja -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE"
fi

# Build c2pulse target via CMake (generator-agnostic)
echo "Building c2pulse..."
cmake --build "$BUILD_DIR" --target c2pulse -j"$JOBS"

# -----------------------------
# Build F*, Pulse, and Pulse.Lib.C
# -----------------------------
FSTAR_DIR="$HERE/external/FStar"
PULSE_DIR="$HERE/external/pulse"
PULSE_LIBC_DIR="$HERE/include/pulse"

if [[ ! -d "$FSTAR_DIR" ]]; then
  echo "ERROR: $FSTAR_DIR not found. Is the submodule checked out?"
  exit 1
fi
if [[ ! -d "$PULSE_DIR" ]]; then
  echo "ERROR: $PULSE_DIR not found. Is the submodule checked out?"
  exit 1
fi
if [[ ! -d "$PULSE_LIBC_DIR" ]]; then
  echo "ERROR: $PULSE_LIBC_DIR not found."
  exit 1
fi

echo "Building F*..."
make -C "$FSTAR_DIR" -j"$JOBS" ADMIT=1

FSTAR_EXE="$(abspath "$FSTAR_DIR/bin/fstar.exe")"
if [[ ! -x "$FSTAR_EXE" && ! -f "$FSTAR_EXE" ]]; then
  echo "WARNING: fstar.exe not found at $FSTAR_EXE. Pulse build may fail. Continuing..."
fi

echo "Building Pulse..."
make -C "$PULSE_DIR" -j"$JOBS" ADMIT=1 FSTAR_EXE="$FSTAR_EXE"

echo "Building Pulse.Lib.C libraries..."
make -C "$PULSE_LIBC_DIR" -j"$JOBS"

BIN_HINT=""
if [[ -x "$BUILD_DIR/bin/c2pulse" ]]; then
  BIN_HINT="$BUILD_DIR/bin/c2pulse"
elif [[ -x "$BUILD_DIR/c2pulse" ]]; then
  BIN_HINT="$BUILD_DIR/c2pulse"
fi

echo
echo "✅ Build Successful!"
if [[ -n "$BIN_HINT" ]]; then
  echo "   c2pulse binary: $BIN_HINT"
else
  echo "   c2pulse should be in the build tree (e.g., $BUILD_DIR/bin)."
fi
echo "   Use ./run.sh to invoke it from the repo root."
