#ifndef skrillex_time_hpp
#define skrillex_time_hpp

#include <chrono>

namespace skrillex {
namespace internal {
    int64_t timestamp();
    std::chrono::time_point<std::chrono::high_resolution_clock> now();
}
}

#endif

