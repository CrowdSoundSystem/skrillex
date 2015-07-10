#include "skrillex/status.hpp"

namespace skrillex {
    bool operator==(const Status& a, const Status& b) {
        return a.code_ == b.code_ && a.message_ == b.message_;
    }
}

