#!/bin/bash

# scripts/build.sh
# An optional script to create a build with CMake
# To run, provide permissions first: chmod +x scripts/build.sh
# Then, run the script: ./scripts/build.sh

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

# Remove the build directory if it exists (to avoid cache conflicts)
if [ -d "$BUILD_DIR" ]; then
    echo "Removing existing build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

# Create the build directory
echo "Creating build directory: $BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Navigate to the build directory
cd "$BUILD_DIR"

# Run CMake configuration to generate compile_commands.json
echo "Configuring with CMake..."
cmake -DCMAKE_CX