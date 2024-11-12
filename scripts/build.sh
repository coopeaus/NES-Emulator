#!/bin/bash

# scripts/build.sh
# An optional script to create a build with CMake
# To run, provide permissions first: chmod +x scripts/build.sh
# Then, run the script: ./scripts/build.sh

# Set the build directory (adjust if your build directory is in a different location)
BUILD_DIR="build"
cd "$(dirname "$0")/.." || exit 1 # run from the project root directory

echo "Starting build process..."

# Create the build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
	echo "Creating build directory: $BUILD_DIR"
	mkdir -p "$BUILD_DIR"
fi

# Navigate to the build directory
cd "$BUILD_DIR"

# Run CMake configuration
echo "Configuring with CMake..."
cmake .. || {
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
