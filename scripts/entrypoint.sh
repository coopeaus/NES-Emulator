#!/bin/bash

# scripts/entrypoint.sh
# Script used to define the operations to be carried out in the Docker container.
# This allows one container to be used for both linting and building the project.

# Define container build directory
BUILD_DIR="/workspace/build_container"

# Ensure exit on error
set -e

# Check if there is a failure
report_failure() {
  if [ "$1" -ne 0 ]; then
    echo "$2 failed: $3 reported issues."
    exit 1
  fi
}
case "$1" in
  # Handle linting
  lint)
    echo "Running lint/format tasks..."

    # Check if compile_commands.json exists, if not, run cmake to generate it
    if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
      echo "compile_commands.json not found. Running cmake to generate it..."
      mkdir -p "$BUILD_DIR"
      cd "$BUILD_DIR"
      cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXPORT_COMPILE_COMMANDS=YES .. || {
        echo "CMake configuration failed."
        exit 1
      }
      cd ..
    fi

    # Initialize failure flags for clang-format and clang-tidy
    format_failures=0
    lint_failures=0

    # Run clang-format on both .cpp and .h files, and check for issues
    if [ -z "$CI" ]; then
      # In local mode, automatically fix formatting
      echo "Running clang-format locally and fixing formatting..."
      for file in $(find src/ include/ -name '*.cpp' -o -name '*.h'); do
        clang-format -i "$file"
      done

    else
      # In CI, check formatting without fixing
      echo "Running clang-format in CI mode (check only)..."
      format_failures=0
      for file in $(find src/ include/ -name '*.cpp' -o -name '*.h'); do
        # Check if clang-format changes the file
        if ! clang-format "$file" | diff -u "$file" - > /dev/null; then
          echo "Formatting issues found in $file"
          format_failures=1
        fi
        if [ $? -ne 0 ]; then
          echo "Formatting issues found in $file"
          format_failures=1
        fi
      done

      # Report failure if formatting issues were found
      if [ "$format_failures" -ne 0 ]; then
        echo "clang-format detected issues. Please fix the formatting. You can run formatting automatically using the instructions in the README"
        exit 1
      else
        echo "clang-format passed with no issues."
      fi
    fi

    # Report failure if clang-format found issues
    report_failure "$format_failures" "clang-format" "formatting"

    # Run clang-tidy on both .cpp and .h files, and check for issues
    echo "Running clang-tidy..."
    for file in $(find src/ include/ -name '*.cpp' -o -name '*.h'); do
      clang-tidy -p "$BUILD_DIR" -header-filter='.*' "$file" || lint_failures=1
    done

    # Report failure if clang-tidy found issues
    report_failure "$lint_failures" "clang-tidy" "linting"
    ;;

  # Handle building
  build)
    echo "Running build tasks..."
    ./scripts/build.sh
    ;;

  # Handle testing
  test)
    echo "Running tests..."
    # Check if a specific test name was passed
    if [ -n "$2" ]; then
      echo "Running specific test: $2"
      ./scripts/test.sh "$2"
    else
      echo "Running all tests..."
      ./scripts/test.sh
    fi
    ;;
  # Oops! Invalid command
  *)
    echo "Houston, we have a problem. Make sure to use 'lint', 'build' or 'test [test_name]'."
    exit 1
    ;;
  

    
esac
