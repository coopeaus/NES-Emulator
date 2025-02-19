cmake_minimum_required( VERSION 3.28.3 )

# Env setup
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set( CMAKE_CXX_STANDARD 23 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

# Project setup
project( nes_emu VERSION 0.1 LANGUAGES CXX )

# Includes
set(CORE_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(FRONTEND_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/frontend/include)
set(TEST_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/tests/include)

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
target_include_directories(emu_core PUBLIC ${CORE_INCLUDES})

# Fmt library
find_package(fmt CONFIG REQUIRED)
target_link_libraries(emu_core PRIVATE fmt::fmt)

#[[
################################################
||                                            ||
||             Emulator Executable            ||
||                                            ||
################################################
]]
if(BUILD_FRONTEND)
  add_executable(emu
    frontend/main.cpp
    frontend/ui.cpp
  )
  target_include_directories(emu PRIVATE ${CORE_INCLUDES})
  target_include_directories(emu PRIVATE ${FRONTEND_INCLUDES})
  target_link_libraries(emu PRIVATE emu_core)

  # SDL
  find_package(SDL2 CONFIG REQUIRED)
  target_link_libraries(emu
        PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    )

  # Imgui
  find_package(imgui CONFIG REQUIRED)
  target_link_libraries(emu PRIVATE imgui::imgui)

  # OpenGL
  find_package(glad CONFIG REQUIRED)
  target_link_libraries(emu PRIVATE glad::glad)
  if(APPLE)
    target_link_libraries(emu PRIVATE "-framework OpenGL")
  else()
    target_link_libraries(emu PRIVATE OpenGL::GL)
  endif()

  # Copy roms to build directory, for testing purposes
  add_custom_command(TARGET emu POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_SOURCE_DIR}/tests/roms ${CMAKE_BINARY_DIR}/tests/roms
  )

  # Copy palettes directory to build directory
  add_custom_command(TARGET emu POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_SOURCE_DIR}/palettes ${CMAKE_BINARY_DIR}/palettes
  )
endif()

#[[
################################################
||                                            ||
||              Test Executables              ||
||                                            ||
################################################
]]
if(BUILD_TESTS)
  # Enable testing framework
  enable_testing()

  # Find the GTest package
  find_package(GTest CONFIG REQUIRED)

  # Include the GoogleTest module for gtest_discover_tests()
  include(GoogleTest)

  # Define include directories for your project
  set(ALL_INCLUDES ${CORE_INCLUDES} ${TEST_INCLUDES})

  # Function to add test executables
  function(add_test_executable TARGET_NAME SOURCE_FILE)
    add_executable(${TARGET_NAME} ${SOURCE_FILE})
    target_include_directories(${TARGET_NAME} PRIVATE ${ALL_INCLUDES})
    target_link_libraries(${TARGET_NAME} PRIVATE emu_core GTest::gtest_main)
    target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic -O2)
    gtest_discover_tests(${TARGET_NAME} PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endfunction()

  # Add your test executables
  add_test_executable(cpu_test tests/cpu_test.cpp)
  add_test_executable(rom_test tests/rom_test.cpp)

endif()
