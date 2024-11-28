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

# Create the build directory
echo "Creating build directory: $BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Navigate to the build directory
cd "$BUILD_DIR"

# Run CMake configuration to generate compile_commands.json
echo "Configuring with CMake..."
cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXPORT_COMPILE_COMMANDS=YES .. || {
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