//
// mutator.hpp
//
// Provides private access to a ResultSet. This allows the internal
// database to modify ResultSets at will, while preventing the 'users'
// from ever modifying a ResultSet.
//

#ifndef skrillex_mutator_hpp
#define skrillex_mutator_hpp

#include <vector>

#include "skrillex/result_set.hpp"

namespace skrillex {
namespace internal {
    class ResultSetMutator {
    public:
        template<typename T>
        static std::vector<T>& getVector(ResultSet<T>& rs) {
            return rs.data_;
        }

        template<typename T>
        static int& getVersion(ResultSet<T>& rs) {
            return rs.data_version_;
        }
    };
}
}

#endif

