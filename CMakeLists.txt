
# Build Instructions:
#   To skip building SDL, add: -DBUILD_FRONTEND=OFF
#   To skip building tests, add: -DBUILD_TESTS=OFF
#
#   i.e., building only the backend and tests:
#   cmake -S . -B build -DBUILD_FRONTEND=OFF
#

cmake_minimum_required( VERSION 3.15 )

# Optionally set a default build type if none is specified.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# The usual CMake setup
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set( CMAKE_CXX_STANDARD 23 )
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

# Options to control which parts to build.
# You can set these via -D option.
option(BUILD_FRONTEND "Build with SDL" ON)
option(BUILD_TESTS    "Build test executables" ON)

# Includes
set(EMU_CORE_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
set(EMU_TEST_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/include
)

include(FetchContent)

#[[
################################################
||                                            ||
||                  Emu Core                  ||
||                                            ||
################################################
]]
# All emulation functionality is built as a static library.
add_library(emu_core STATIC
  core/cpu.cpp
  core/bus.cpp
  core/cartridge.cpp
  core/ppu.cpp
  core/mappers/mapper0.cpp
  core/mappers/mapper1.cpp
  core/utils.cpp
)
target_include_directories(emu_core PUBLIC ${EMU_CORE_INCLUDES})
if(MSVC)
  target_compile_options(emu_core PRIVATE /W4 /WX)
else()
  target_compile_options(emu_core PRIVATE -Wall -Wextra -Wpedantic -Werror -O2)
endif()


#[[
################################################
||                                            ||
||             Emulator Executable            ||
||                                            ||
################################################
]]
# The frontend is separate from the core. It only builds main.cpp (and any
# additional SDL-specific files you might add later) and links against emu_core.
if(BUILD_FRONTEND)
  add_executable(emu
          frontend/main.cpp
  )
  target_include_directories(emu PRIVATE ${EMU_CORE_INCLUDES})
  target_link_libraries(emu PRIVATE emu_core)

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
    # target_compile_options(emu PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(emu PRIVATE -Wall -Wextra -Wpedantic -Werror -O2)
  endif()

#[[
################################
  ||             SDL            ||
################################
  ]]
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

#[[
################################
||         Mac Bundle         ||
################################
]]
  # Uncomment to build emu as a standalone MacOS .app bundle (optional)
  # Useful for distribution or if you need emu to behave as a native MacOS app.
  # Has no effect on Windows or Linux builds.
  # set_target_properties(emu PROPERTIES MACOSX_BUNDLE TRUE)

endif()

#[[
################################################
||                                            ||
||              Test Executables              ||
||                                            ||
################################################
]]
if(BUILD_TESTS)
  enable_testing()
  # Download GoogleTest
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2  # As of Nov 6, 2024
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  #[[
  ################################
  ||       CPU JSON Tests       ||
  ################################
  ]]
  add_executable(cpu_test tests/cpu_test.cpp)
  target_include_directories(cpu_test PRIVATE ${EMU_CORE_INCLUDES} ${EMU_TEST_INCLUDES})
  target_link_libraries(cpu_test PRIVATE emu_core gtest_main)
  target_compile_options(cpu_test PRIVATE -Wall -Wextra -Wpedantic -Werror -O2)

  #[[
  ################################
  ||          ROM Tests         ||
  ################################
  ]]
  add_executable(rom_test tests/rom_test.cpp)
  target_include_directories(rom_test PRIVATE ${EMU_CORE_INCLUDES} ${EMU_TEST_INCLUDES})
  target_link_libraries(rom_test PRIVATE emu_core gtest_main)
  target_compile_options(rom_test PRIVATE -Wall -Wextra -Wpedantic -Werror -O2)

  include(GoogleTest)
  gtest_discover_tests(cpu_test PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  gtest_discover_tests(rom_test PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

endif()
