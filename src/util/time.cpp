#include "util/time.hpp"

using namespace std;

namespace skrillex {
namespace internal {
    int64_t timestamp() {
        return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    }

    chrono::time_point<std::chrono::high_resolution_clock> now() {
        chrono::high_resolution_clock::now();
    }
}
}
