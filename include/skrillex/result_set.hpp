//
// result_set.hpp
//
// A result set contains the data from a given query.
//
// The primary way to iterate through data of a result
// set is by using iterators. This is done mainly to
// remove any dependence on the underlying container.
//
// Result sets can be reused. That is, the following
// operation will yield with the most recent results:
//
//     ResultSet<Song> songs;
//     getSongs(songs, legacyOptions);  // Read legacy
//     getSongs(songs, defaultOptions); // Read session
//
// The result of this operation will yield only the
// results from the second call. This makes it more
// convienient to capture results, but more importantly,
// allows for the avoidance of queries altogether!
//
// ResultSet is **not** thread safe. When ResultSet is
// passed into a DB object, it assumes exlusive access.
//

#ifndef skrillex_result_set_hpp
#define skrillex_result_set_hpp

#include <vector>
#include "skrillex/dbo.hpp"

namespace skrillex {
    namespace internal {
        class ResultSetMutator;
    }

    template<typename T>
    class ResultSet {
    public:
        ResultSet()
        : data_version_(0)
        {
        }

        typedef typename std::vector<T>::const_iterator const_iterator;

        bool empty() const { return data_.empty(); }
        int  size()  const { return data_.size(); }

        const_iterator begin() const { return data_.begin(); }
        const_iterator end()   const { return data_.end(); }

    private:
        std::vector<T> data_;
        int            data_version_;

        friend class internal::ResultSetMutator;
    };
}
#endif

