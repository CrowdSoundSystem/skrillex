cmake_minimum_required(VERSION 2.8.0)
project(skrillex_bench)

find_package(Threads REQUIRED)

# Test the internals too
include_directories(../src)

# I'm sure there's a better way to due this, but deadlines
# are creeping, and there shouldn't be more benches than
# there are fingers on my hand.
add_executable(mapper_bench "mapper_bench.cpp")
set_property(TARGET mapper_bench PROPERTY CXX_STANDARD 11)
target_link_libraries(mapper_bench skrillex ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS mapper_bench DESTINATION bin)
