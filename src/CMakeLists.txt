cmake_minimum_required(VERSION 2.8.0)
project(skrillex)

include_directories(.)

file(GLOB_RECURSE skrillex_srcs
    "*.cpp"
    "*.c"
)

add_library(skrillex STATIC ${skrillex_srcs})
set_target_properties(skrillex PROPERTIES PREFIX "")
install(TARGETS skrillex DESTINATION bin)
