#include <ostream>
#include "skrillex/status.hpp"

using namespace std;

namespace skrillex {
    bool operator==(const Status& a, const Status& b) {
        return a.code_ == b.code_ && a.message_ == b.message_;
    }

    bool operator!=(const Status& a, const Status& b) {
        return !(a == b);
    }

    ostream& operator<<(ostream& os, const Status& status) {
        return os << status.message();
    }
}

