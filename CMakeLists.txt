cmake_minimum_required( VERSION 3.15 )

# Project name and version
project( nes_emu VERSION 0.1 LANGUAGES CXX )

# Enable compile_commands.json (for non-IDE builds)
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

# C++ 17 standard
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

if(MSVC)
  # If in visual studio
  add_compile_options(/W4 /WX) # /W4 for high warning level, /WX to treat warnings as errors
else()
  # GCC/Clang warning flags
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Include directories
include_directories( include )
include_directories( tests/include ) # for tests only
# Add additional includes here
# include_directories(/path/to/other/includes)

# Fetch content to download dependencies at configure time
include(FetchContent)

# Fetch the Google Test Library
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2 # Latest version as of Nov 6, 2024
)

# Ensures GoogleTest uses the shared C runtime (DLL) on Windows, instead of the static one
set( gtest_force_shared_crt ON CACHE BOOL "" FORCE )
FetchContent_MakeAvailable(googletest)

# ----------------------- MAIN EXECUTABLES -----------------------
add_executable(emu
  src/main.cpp
  src/cpu.cpp
  src/bus.cpp
)

# ----------------------- LIBRARIES -----------------------

# Usage of PRIVATE, PUBLIC, and INTERFACE:
# - PRIVATE: Only the target can use the linked library.
# - PUBLIC: The target and its dependents can use the linked library.
# - INTERFACE: Can be used by the target's dependents, but not the target itself.

# Example
# target_link_libraries(emu
#   PRIVATE graphics_library           # Only emu
#   PUBLIC core_library                # emu and anything using emu
#   INTERFACE api_support_library      # Anything using emu, but not emu itself
# )

# ----------------------- TEST EXECUTABLES -----------------------
add_executable(sample_test
  tests/sample.cpp
  # Add related cpp files here
)
add_executable(cpu_test
  tests/cpu_test.cpp
  src/cpu.cpp
  src/bus.cpp
)
# add other test executables here

# ----------------------- TEST LIBRARIES -----------------------
target_link_libraries(sample_test PRIVATE gtest_main)
target_link_libraries(cpu_test PRIVATE gtest_main)
# Link all other test executables to gtest_main

# --- enable testing, do not touch, set only once ---
enable_testing()

# ----------------------- TEST CONFIGURATION -----------------------
# We need to add the below to make tests discoverable. The PROPERTIES WORKING_DIRECTORY
# sets the working directory of the test to the root folder instead of the build folder,
# which resolves any issues with relative paths in the tests.

# --- set only once ---
include(GoogleTest)
# ---
gtest_discover_tests(sample_test
    PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
gtest_discover_tests(cpu_test
    PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
# Add other tests here

# ----------------------- MAC BUNDLE -----------------------
# Uncomment to build emu as a standalone MacOS .app bundle (optional)
# Useful for distribution or if you need emu to behave as a native MacOS app.
# Has no effect on Windows or Linux builds.
# set_target_properties(emu PROPERTIES MACOSX_BUNDLE TRUE)

