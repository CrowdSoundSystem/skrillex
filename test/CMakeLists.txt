cmake_minimum_required(VERSION 3.0)
project(skrillex_tests)

FILE(GLOB_RECURSE all_test_sources "*.cpp")

# Unit Test
add_executable(skrillex_tests ${all_test_sources})

# Link test executable against gtest and gtest_main
target_link_libraries(skrillex_tests gtest gtest_main)

install(TARGETS skrillex_tests DESTINATION bin)
