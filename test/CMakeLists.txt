cmake_minimum_required(VERSION 3.0)
project(skrillex_tests)

# Test the internels too
include_directories(../src)

# GTest Setup
add_subdirectory (gtest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

FILE(GLOB_RECURSE all_test_sources "*.cpp")
FOREACH(item ${all_test_sources})
  IF(${item} MATCHES "gtest/")
    LIST(REMOVE_ITEM all_test_sources ${item})
  ENDIF(${item} MATCHES "gtest/")
ENDFOREACH(item)
MESSAGE(STATUS "${all_test_sources}")

# Unit Test
add_executable(skrillex_tests ${all_test_sources})

# Link test executable against gtest and gtest_main
target_link_libraries(skrillex_tests skrillex gtest gtest_main)

install(TARGETS skrillex_tests DESTINATION bin)
