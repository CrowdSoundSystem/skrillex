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

#ifndef skrillex_result_set_hpp
#define skrillex_result_set_hpp

#include <vector>
#include "skrillex/dbo.hpp"

namespace skrillex {
    template<typename T>
    class ResultSet {
        std::vector<T> data_;
        int            data_version_;

    public:
        ResultSet();

        class iterator {
        public:
            T&        operator*();

            bool      operator==(const iterator& o) const;
            bool      operator!=(const iterator& o) const;

            iterator& operator=(const iterator& o);
            iterator& operator++();
        };

        iterator begin() const;
        iterator end() const;

        int getDataVersion() const;
    };
}

#endif

