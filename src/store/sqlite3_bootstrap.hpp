#ifndef skrillex_sqlite3bootstrap_hpp
#define skrillex_sqlite3bootstrap_hpp

#include <string>

#include "skrillex/status.hpp"
#include "sqlite3/sqlite3.h"

namespace skrillex {
namespace internal {
    Status bootstrap(
            const std::string& path,
            sqlite3*& db,
            bool create_if_missing,
            bool recreate);
}
}

#endif

