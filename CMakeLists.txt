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
set(UI_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/frontend/ui)
set(TEST_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/tests/include)

#[[
################################################
||                                            ||
||                  Emu Core                  ||
||                                            ||
################################################
]]
# All emulation functionality is built as a static library.
file(GLOB CORE_SOURCES "core/*.cpp" "core/mappers/*.cpp")
add_library(emu_core STATIC ${CORE_SOURCES})

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
  message(STATUS "BUILD_FRONTEND is enabled. Configuring frontend...")

  file(GLOB FRONTEND_SOURCES "frontend/*.cpp" "frontend/ui/*.cpp")
  add_executable(emu ${FRONTEND_SOURCES})

  target_include_directories(emu PRIVATE ${CORE_INCLUDES})
  target_include_directories(emu PRIVATE ${FRONTEND_INCLUDES})
  target_include_directories(emu PRIVATE ${UI_INCLUDES})
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
  elseif(WIN32)
    target_link_libraries(emu PRIVATE ${OpenGL_GL_LIBRARY})
  else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(emu PRIVATE OpenGL::GL)
  endif()

  # Debugging asset paths
  message(STATUS "Checking asset paths:")
  message(STATUS "Palettes: ${CMAKE_SOURCE_DIR}/palettes -> ${CMAKE_BINARY_DIR}/palettes")
  message(STATUS "Fonts: ${CMAKE_SOURCE_DIR}/fonts -> ${CMAKE_BINARY_DIR}/fonts")
  message(STATUS "Tests/Roms: ${CMAKE_SOURCE_DIR}/tests/roms -> ${CMAKE_BINARY_DIR}/tests/roms")

  # Copy assets only if they exist
  if(EXISTS "${CMAKE_SOURCE_DIR}/palettes")
    add_custom_command(TARGET emu POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/palettes ${CMAKE_BINARY_DIR}/palettes
    )
  else()
    message(WARNING "WARNING: Palettes directory NOT found!")
  endif()

  if(EXISTS "${CMAKE_SOURCE_DIR}/fonts")
    add_custom_command(TARGET emu POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/fonts ${CMAKE_BINARY_DIR}/fonts
    )
  else()
    message(WARNING "WARNING: Fonts directory NOT found!")
  endif()

  if(EXISTS "${CMAKE_SOURCE_DIR}/tests/roms")
    add_custom_command(TARGET emu POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/tests/roms ${CMAKE_BINARY_DIR}/tests/roms
    )
  else()
    message(WARNING "WARNING: Tests/Roms directory NOT found!")
  endif()

  # Force asset copy by adding a target
  add_custom_target(copy_assets ALL DEPENDS emu)
  add_dependencies(copy_assets emu)

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
  add_test_executable(ppu_test tests/ppu_test.cpp)
  add_test_executable(cart_test tests/cart_test.cpp)

endif()
