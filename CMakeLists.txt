cmake_minimum_required( VERSION 3.28.3 )

# Check if Clang is available and set it as the compiler
find_program(CLANG_C_COMPILER clang)
find_program(CLANG_CXX_COMPILER clang++)

if(CLANG_C_COMPILER AND CLANG_CXX_COMPILER)
  message(STATUS "Clang found, using Clang as the compiler")
  set(CMAKE_C_COMPILER ${CLANG_C_COMPILER})
  set(CMAKE_CXX_COMPILER ${CLANG_CXX_COMPILER})
else()
  message(STATUS "Clang not found, using default system compiler")
endif()

# Env setup
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set( CMAKE_CXX_STANDARD 23 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set(CMAKE_OSX_SYSROOT /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk)

# Project setup
project( nes_emu VERSION 0.1 LANGUAGES CXX )

# Includes
set(GENERAL_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(CORE_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/core)
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

target_include_directories(emu_core PUBLIC
  ${GENERAL_INCLUDES}
  ${CORE_INCLUDES}
)

# Fmt library
find_package(fmt CONFIG REQUIRED)
target_link_libraries(emu_core PRIVATE fmt::fmt)

# Build asset directories
# --copy source
set(ROOT "${CMAKE_SOURCE_DIR}")
set(ROM_S "${ROOT}/roms")
set(TESTS_S "${ROOT}/tests")
set(FONTS_S "${ROOT}/fonts")
set(PALETTES_S "${ROOT}/palettes")

# --copy dest
set(ASSETS "${CMAKE_BINARY_DIR}/assets")
set(TESTS_D "${CMAKE_BINARY_DIR}/tests")
set(ROMS_D "${ASSETS}/roms")
set(FONTS_D "${ASSETS}/fonts")
set(PALETTES_D "${ASSETS}/palettes")

message(STATUS "Checking asset paths:")
message(STATUS "Palettes: ${PALETTES_D}")
message(STATUS "Fonts: ${FONTS_D}")
message(STATUS "Roms: ${ROMS_D}")

function(copy_asset source dest)
  if(EXISTS "${source}")
    add_custom_command(TARGET emu_core POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory "${source}" "${dest}"
    )
  else()
    message(FATAL_ERROR "COPY ASSET FAILED: Source: ${source} Destination: ${dest}")
  endif()
endfunction()

copy_asset(${PALETTES_S} ${PALETTES_D})
copy_asset(${FONTS_S}    ${FONTS_D})
copy_asset(${ROM_S}      ${ROMS_D})
copy_asset(${TESTS_S}    ${TESTS_D})

# Force assets to copy by adding a target
add_custom_target(copied_assets ALL DEPENDS emu_core)
add_dependencies(copied_assets emu_core)


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

  target_compile_definitions(emu PRIVATE NOMINMAX)

  target_include_directories(emu PRIVATE
    ${GENERAL_INCLUDES}
    ${CORE_INCLUDES}
    ${FRONTEND_INCLUDES}
    ${UI_INCLUDES}
  )
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
  set(ALL_INCLUDES
    ${GENERAL_INCLUDES}
    ${CORE_INCLUDES}
    ${TEST_INCLUDES}
  )

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

