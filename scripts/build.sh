#!/bin/bash

# scripts/build.sh
# An optional script to create a build with CMake.
# Usage:
#   ./scripts/build.sh [full|core|test]
#   - "full" (or no argument): build the full emulator (frontend + tests)
#   - "core": build only the emulation core (backend)
#   - "test": build only the test executables (which link against the core)
#
# To run, first provide permissions: chmod +x scripts/build.sh
# Then run the script: ./scripts/build.sh [option]

# Determine build configuration based on command-line argument
if [ -z "$1" ] || [ "$1" == "full" ]; then
  echo "Configuring build for: FULL (SDL, core files, and tests)"
  CMAKE_OPTIONS="-DBUILD_FRONTEND=ON -DBUILD_TESTS=ON"
elif [ "$1" == "core" ]; then
  echo "Configuring build for: CORE only (No SDL, no tests)"
  CMAKE_OPTIONS="-DBUILD_FRONTEND=OFF -DBUILD_TESTS=OFF"
elif [ "$1" == "tests" ]; then
  echo "Configuring build for: TESTS only (core + tests, no frontend)"
  CMAKE_OPTIONS="-DBUILD_FRONTEND=OFF -DBUILD_TESTS=ON"
else
  echo "Usage: $0 [full|core|tests]"
  exit 1
fi

# Determine if we're running inside a Docker container
if [ -f /.dockerenv ]; then
  # We're inside the container, use a container-specific build directory
  BUILD_DIR="/workspace/build_container"
  echo "Building inside Docker container. Using build directory: $BUILD_DIR"
else
  # We're running locally, use the local build directory
  BUILD_DIR="build"
  echo "Building locally. Using build directory: $BUILD_DIR"
fi

# Navigate to the project root directory
cd "$(dirname "$0")/.." || exit 1

echo "Starting build process..."

# Create the build directory
echo "Creating build directory: $BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Navigate to the build directory
cd "$BUILD_DIR"

# Run CMake configuration to generate compile_commands.json
echo "Configuring with CMake..."
cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXPORT_COMPILE_COMMANDS=YES $CMAKE_OPTIONS .. || {
  echo "CMake configuration failed."
  exit 1
}

# Run make to build the project
echo "Building the project..."
make || {
  echo "Build failed."
  exit 1
}

# Notify user of created executables
echo "Checking for created executable files..."
executables=$(find . -maxdepth 1 -type f -perm +111)

if [ -n "$executables" ]; then
  echo "Build complete. Created executables:"
  for exec in $executables; do
    echo " - $BUILD_DIR/$(basename $exec)"
  done
else
  echo "No executable files were created."
fi

