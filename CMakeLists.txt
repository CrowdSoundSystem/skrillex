cmake_minimum_required(VERSION 2.8.0)
project(skrillex)

# Allow C++11
add_definitions(-std=c++11)

# The Install Location (i.e. make install)
set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR})

# Build Location (i.e. make)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

include_directories(include)

add_subdirectory(src)
add_subdirectory(test)
