#!/bin/bash

# clean.sh
# Optional script to clean CMake build files and executables.
# To use, set proper permissions: chmod +x clean.sh
# Usage: ./clean.sh

# Set the build directory (adjust if your build directory is in a different location)
BUILD_DIR="build"

echo "Cleaning CMake build files..."

# Check if the build directory exists
if [ -d "$BUILD_DIR" ]; then
	# Remove CMake cache and generated files
	rm -rf "$BUILD_DIR/CMakeCache.txt" "$BUILD_DIR/CMakeFiles" "$BUILD_DIR/Makefile" "$BUILD_DIR/cmake_install.cmake"
	echo "Removed cache and generated files from $BUILD_DIR."

	# Remove all executable files in the build directory
	find "$BUILD_DIR" -type f -perm +111 -exec rm -f {} + 2>/dev/null
	echo "Removed all executables from $BUILD_DIR."

	echo "Clean complete."
else
	echo "Build directory $BUILD_DIR does not exist. Nothing to clean."
fi
