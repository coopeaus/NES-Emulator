#!/bin/bash
set -euo pipefail

# scripts/test.sh
# Optional test script to run CTest in the build directory.
# To use, set proper permissions: chmod +x scripts/test.sh
# Run script: ./scripts/test.sh

# See repo docs for more details

BUILD_DIR="build"

# Navigate to the project root directory
cd "$(dirname "$0")/.." || exit 1

# Navigate to the build directory and run ctest
cd "$BUILD_DIR" || {
  echo "Error: Could not enter build directory '$BUILD_DIR'."
  exit 1
}

# Dynamically determine the number of cores
if [ -n "${CI:-}" ]; then
  CORES=2
elif command -v nproc >/dev/null 2>&1; then
  CORES=$(nproc)
elif [ "$(uname)" = "Darwin" ]; then
  CORES=$(sysctl -n hw.ncpu)
else
  CORES=2
fi

# Run specific test if provided, otherwise run all tests in parallel
if [ -n "${1:-}" ]; then
  echo "Running specific test: $1"
  ctest -R "$1" -V # Matches test names against provided pattern
else
  echo "Running all tests in parallel on $CORES core(s)"
  ctest -j "$CORES"
fi

# Example usage:
# ./scripts/test.sh "CpuTest.IMM"   # Run a specific test
# ./scripts/test.sh                      # Run all tests in parallel

# List of all tests: ctest -N
