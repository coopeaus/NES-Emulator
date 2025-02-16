#!/usr/bin/env bash

# Function to extract version number from the first line of output.
extract_version() {
  head -n1 | sed -E 's/.*version[[:space:]]+([0-9]+\.[0-9]+\.[0-9]+).*/\1/'
}

# Detect platform using uname.
PLATFORM=$(uname -s)
echo "Platform detected: $PLATFORM"
echo ""

FAILED=0

EXTRAS=0
if [ "$1" = "--extras" ]; then
  EXTRAS=1
fi

# Function to check if a command exists, and optionally verify its version.
# Usage: check_dependency <command> <required_version_pattern> <install_message>
# If <required_version_pattern> is empty, the version check is skipped.
check_dependency() {
  local cmd="$1"
  local req_ver="$2"
  local install_msg="$3"

  local path
  path=$(command -v "$cmd")
  if [ -z "$path" ]; then
    echo "❌ Cannot detect $cmd."
    echo "$install_msg"
    FAILED=1
  else
    if [ -n "$req_ver" ]; then
      local ver
      ver=$("$cmd" --version 2>/dev/null | extract_version)
      if [[ "$ver" != $req_ver ]]; then
        echo "❌ Detected $cmd version $ver, need version matching '$req_ver'."
        FAILED=1
      else
        echo "✅ $cmd $ver: $path"
      fi
    else
      echo "✅ $cmd: $path"
    fi
  fi
}

# Check dependencies.
check_dependency "clang" "19.*" "brew install llvm@19"
check_dependency "clang++" "19.*" "brew install llvm@19"
check_dependency "cmake" "" "brew install cmake"
check_dependency "ninja" "" "brew install ninja"
check_dependency "pkg-config" "" "brew install pkg-config"
check_dependency "vcpkg" "" "Please install vcpkg: https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-bash"

# Extra dependencies
if [ "$EXTRAS" -eq 1 ]; then
  echo ""
  echo "Checking extra dependencies..."
  echo ""
  check_dependency "clang-format" "19.*" "brew install llvm"
  check_dependency "clang-tidy" "19.*" "brew install llvm"
fi

# If any dependency check failed, output platform-specific installation instructions.
if [ "$FAILED" -ne 0 ]; then
  if [ "$PLATFORM" = "Darwin" ]; then
    if [ -z "$(command -v brew)" ]; then
      echo "❌ Cannot detect brew."
      echo "Please install Homebrew: https://brew.sh"
      exit 1
    fi
    BREW_BIN="$(brew --prefix)/bin"
    echo ""
    echo "Install the missing dependencies and add the brew path to your PATH."
    echo "Brew path: $BREW_BIN"
    echo "For example, add the following to your shell configuration file:"
    echo "echo 'export PATH=$BREW_BIN:\$PATH' >> ~/.bashrc && source ~/.bashrc"
  else
    echo ""
    echo "Please install the missing dependencies and add them to your PATH."
    echo "For example, on Debian/Ubuntu:"
    echo "sudo apt-get update && sudo apt-get install -y clang cmake ninja-build pkg-config"
  fi
  exit 1
fi

echo ""
echo "✅ All set!"
