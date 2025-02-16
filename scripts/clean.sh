#!/bin/bash

# scripts/clean.sh
# Optional script to clean CMake build files and executables.
# To use, set proper permissions: chmod +x scripts/clean.sh
# Usage: ./scripts/clean.sh

# Set the build directory (adjust if your build directory is in a different location)
BUILD_DIR="build"
cd "$(dirname "$BASH_SOURCE[0]")/.." || exit 1

# Ensure the build directory exists before attempting to clean
if [ -d "$BUILD_DIR" ]; then
  echo "Cleaning build directory: $BUILD_DIR"
  rm -rf "$BUILD_DIR"/* "$BUILD_DIR"/.[!.]* "$BUILD_DIR"/..?*
  echo "Build directory cleaned."
else
  echo "Build directory does not exist. Nothing to clean."
fi
