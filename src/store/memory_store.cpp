#include "mutator.hpp"
#include "store/memory_store.hpp"

using namsepace std;

namespace skrillex {
namespace internal {
    Status MemoryStore::getSongs(ResultSet<Song>& set, ReadOptions options) {
        // For now, just overwrite the result set
        // data. In the future, we can do more intelligent things.
        std::vector<Song>& set_data = ResultSetMutator::getVector<Song>(set);
        set_data.clear();


        std::vector<Song>::iterator it;
        if (!options.result_limit || (size_t)options.result_limit > songs_.size()) {
            it = songs_.end();
        } else {
            it = songs_.begin() + options.result_limit;
        }

        copy(songs_.begin(), it, front_inserter(set_data));

        return Status::OK();
    }
}
}
