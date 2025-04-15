cmake_minimum_required(VERSION 3.28.3)
project(EmulatorCLI)

# Find dependencies
find_package(fmt CONFIG REQUIRED)
find_package(Python COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

# Build the Python module
pybind11_add_module(emu MODULE emu.cpp)
target_link_libraries(emu PRIVATE emu_core)
target_link_libraries(emu PRIVATE fmt::fmt)

# Place the module in the same directory as this CMakeLists.txt
set_target_properties(emu PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

