#!/bin/bash
# test.sh
# Optional test script to run CTest in the build directory
# Give file permission to execute: chmod +x test.sh
# run script: ./test.sh

BUILD_DIR="build"

# Check if the build directory exists
if [ ! -d "$BUILD_DIR" ]; then
	echo "Error: Build directory '$BUILD_DIR' does not exist. Please run ./build.sh first."
	exit 1
fi

# Navigate to the build directory and run ctest
cd "$BUILD_DIR" || {
	echo "Error: Could not enter build directory '$BUILD_DIR'."
	exit 1
}

ctest
