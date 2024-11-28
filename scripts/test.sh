#!/bin/bash
# scripts/test.sh
# Optional test script to run CTest in the build directory
# Give file permission to execute: chmod +x scripts/test.sh
# Run script: ./scripts/test.sh

# Determine if we're running inside a Docker container
if [ -f /.dockerenv ]; then
    # We're inside the container, use a container-specific build directory
    BUILD_DIR="/workspace/build_container"
    echo "Testing inside Docker container. Using build directory: $BUILD_DIR"
else
    # We're running locally, use the local build directory
    BUILD_DIR="build"
    echo "Testing locally. Using build directory: $BUILD_DIR"
fi

cd "$(dirname "$0")/.." || exit 1 # run from the project root directory

# Check if the build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    if [ -f /.dockerenv ]; then
        echo "Error: Build directory '$BUILD_DIR' does not exist in the container."
        echo "Ensure the container has been properly set up and the build process has run successfully. See README for commands."
    else
        echo "Error: Build directory '$BUILD_DIR' does not exist locally."
        echo "Please run './scripts/build.sh' first."
    fi
    exit 1
fi

# Continue with testing if the build directory exists
echo "Build directory '$BUILD_DIR' found. Running tests..."
# Add CTest or other testing commands here

# Navigate to the build directory and run ctest
cd "$BUILD_DIR" || {
	echo "Error: Could not enter build directory '$BUILD_DIR'."
	exit 1
}

# Run specific test if provided, otherwise run all tests
if [ -n "$1" ]; then
	echo "Running specific test: $1"
	ctest -R "$1" -V # Matches test names against provided pattern
else
	echo "Running all tests"
	ctest -V
fi

# Isolating a specific test locally
# ./scripts/test.sh "CPUTestFixture.IMM" # Immediate addressing test
# ./scripts/test.sh "CPUTestFixture.xEA" # Test for opcode 0xEA (JMP)