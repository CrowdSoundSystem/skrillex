//
// mutator.hpp
//
// Provides various mutators used for 'backdooring'
// internal structures of DB objects. Uses include
// modification of ResultSets, and mock testing.
//

#ifndef skrillex_mutator_hpp
#define skrillex_mutator_hpp

#include <vector>

#include "skrillex/db.hpp"
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

    class StoreMutator {
    public:
        static Store* getStore(DB* db) {
            return db->store_.get();
        }
    };
}
}

#endif

