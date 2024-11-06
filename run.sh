#!/bin/bash
# run.sh
# An optional script to run the executable after building it.
# Provide permissions to the script by running `chmod +x run.sh`.
# Run with `./run.sh <executable_name>`.

BUILD_DIR="build"

# Check for the executable name argument
if [ -z "$1" ]; then
	echo "Usage: ./run.sh <executable_name>"
	exit 1
fi

EXECUTABLE="$BUILD_DIR/$1"

# Check if the build directory exists
if [ ! -d "$BUILD_DIR" ]; then
	echo "Build directory '$BUILD_DIR' does not exist. Try running ./build.sh first."
	exit 1
fi

# Check if the executable exists and is executable
if [ ! -f "$EXECUTABLE" ]; then
	echo "Executable '$1' not found in the build directory."
	exit 1
fi

if [ ! -x "$EXECUTABLE" ]; then
	echo "'$1' is not executable. Ensure it was built correctly."
	exit 1
fi

# Run the executable
"$EXECUTABLE"
