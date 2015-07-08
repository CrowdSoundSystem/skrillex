cmake_minimum_required(VERSION 3.0.0)
project(skrillex)

# The Install Location (i.e. make install)
set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR})

# Build Location (i.e. make)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

include_directories(include)

add_subdirectory(src)
add_subdirectory(test)
