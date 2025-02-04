cmake_minimum_required( VERSION 3.15 )

# Enable compile_commands.json (for non-IDE builds)
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

# C++ 17 standard
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

# Set clang as the default compiler if available
find_program( CLANGXX clang++ )

if( CLANGXX )
  set( CMAKE_CXX_COMPILER ${CLANGXX} )
  message( STATUS "Using clang++ compiler" )
else()
  message( STATUS "Using default compiler" )
endif()

# Project name and version. Compiler info needs to be set before this.
project( nes_emu VERSION 0.1 LANGUAGES CXX )

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
        src/cartridge.cpp
        src/ppu.cpp
        src/mappers/mapper0.cpp
        src/mappers/mapper1.cpp
)

# Copy roms to build directory, for testing purposes
add_custom_command(TARGET emu POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/tests/roms ${CMAKE_BINARY_DIR}/tests/roms
)

# Copy palettes directory to build directory
add_custom_target(copy_palettes ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/palettes ${CMAKE_BINARY_DIR}/palettes
)

# Apply compiler options to emu target
if(MSVC)
  target_compile_options(emu PRIVATE /W4 /WX)
else()
  # Leave commented out for debugging purposes, for easy switching
  # target_compile_options(emu PRIVATE -Wall -Wextra -Wpedantic -Werror)
  target_compile_options(emu PRIVATE -Wall -Wextra -Wpedantic -Werror -O2)
endif()

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

# --- SDL2 ---
if(APPLE)
  message(STATUS "Configuring SDL2 for macOS")
  set(SDL2_FRAMEWORK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/sdl2/mac")
  target_include_directories(emu PRIVATE "${SDL2_FRAMEWORK_DIR}/SDL2.framework/Headers")
  target_link_libraries(emu PRIVATE
          "-F${SDL2_FRAMEWORK_DIR}"
          "-framework SDL2"
  )

  # Copy SDL2 framework to the build directory
  add_custom_command(TARGET emu POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${SDL2_FRAMEWORK_DIR}/SDL2.framework"
          "$<TARGET_FILE_DIR:emu>/SDL2.framework"
  )

  # Set RPATH to look in the build directory
  set_target_properties(emu PROPERTIES
          BUILD_WITH_INSTALL_RPATH TRUE
          INSTALL_RPATH "@loader_path"
  )
elseif(WIN32)
  message(STATUS "Configuring SDL2 for Windows")

  # Fetch SDL2
  FetchContent_Declare(
          SDL2
          GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
          GIT_TAG        release-2.30.9  # Latest version as of Nov 22, 2024
  )

  # SDL2 Build Options
  set(SDL_STATIC OFF CACHE BOOL "" FORCE)       # Build SDL2 as a shared library
  set(SDL_SHARED ON CACHE BOOL "" FORCE)
  set(SDL_TEST OFF CACHE BOOL "" FORCE)         # Do not build SDL2 tests

  # Ensure SDL2 builds with the same runtime library as the project
  if(MSVC)
    set(SDL_FORCE_STATIC_VCRT OFF CACHE BOOL "" FORCE)
  endif()

  # Prevent SDL2 from treating warnings as errors
  set(SDL_WERROR OFF CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(SDL2)

  # Link SDL2 to emu target
  target_link_libraries(emu PRIVATE SDL2::SDL2)

  # Get the SDL2 include directories and add them to your target
  get_target_property(SDL2_INCLUDE_DIRS SDL2::SDL2 INTERFACE_INCLUDE_DIRECTORIES)
  message("SDL2 include directories: ${SDL2_INCLUDE_DIRS}")
  target_include_directories(emu PRIVATE ${SDL2_INCLUDE_DIRS}/..)

  # Copy SDL2.dll
  add_custom_command(TARGET emu POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
          $<TARGET_FILE:SDL2::SDL2>
          $<TARGET_FILE_DIR:emu>/$<TARGET_FILE_NAME:SDL2::SDL2>
  )
elseif(UNIX AND NOT APPLE)
  message(STATUS "Configuring SDL2 for Linux")

  # Assume SDL2 is installed on the system. We cannot baby Linux users.
  find_package(SDL2 REQUIRED)
  target_include_directories(emu PRIVATE ${SDL2_INCLUDE_DIRS})
  target_link_libraries(emu PRIVATE ${SDL2_LIBRARIES})

  message(STATUS "SDL2 found: ${SDL2_LIBRARIES}")
endif()

# ----------------------- TEST EXECUTABLES -----------------------
add_executable(sample_test
        tests/sample.cpp
        # Add related cpp files here
)
add_executable(cpu_test
        tests/cpu_test.cpp
        src/cpu.cpp
        src/bus.cpp
        src/cartridge.cpp
        src/ppu.cpp
        src/mappers/mapper0.cpp
        src/mappers/mapper1.cpp
        src/utils.cpp
)

# target_compile_options(emu PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_compile_options(cpu_test PRIVATE -Wall -Wextra -Wpedantic -Werror -O2 )

add_executable(rom_test
  tests/rom_test.cpp
  src/cpu.cpp
  src/bus.cpp
  src/cartridge.cpp
        src/ppu.cpp
  src/mappers/mapper0.cpp
  src/mappers/mapper1.cpp
  src/utils.cpp
)
# add other test executables here

# ----------------------- TEST LIBRARIES -----------------------
target_link_libraries(sample_test PRIVATE gtest_main)
target_link_libraries(cpu_test PRIVATE gtest_main)
target_link_libraries(rom_test PRIVATE gtest_main)
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
gtest_discover_tests(rom_test
        PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
# Add other tests here

# ----------------------- MAC BUNDLE -----------------------
# Uncomment to build emu as a standalone MacOS .app bundle (optional)
# Useful for distribution or if you need emu to behave as a native MacOS app.
# Has no effect on Windows or Linux builds.
# set_target_properties(emu PROPERTIES MACOSX_BUNDLE TRUE)

