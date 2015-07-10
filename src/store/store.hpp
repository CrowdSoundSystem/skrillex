#ifndef skrillex_store_hpp
#define skrillex_store_hpp

#include "skrillex/options.hpp"
#include "skrillex/result_set.hpp"
#include "skrillex/status.hpp"

namespace skrillex {
namespace internal {
    class Store {
    public:
        virtual ~Store() = 0;

        virtual Status getSongs(ResultSet<Song>& set, ReadOptions options) = 0;
        virtual Status getArtists(ResultSet<Artist>& set, ReadOptions options) = 0;
        virtual Status getGenres(ResultSet<Genre>& set, ReadOptions options) = 0;

        virtual Status getPlayHistory(ResultSet<Song>& set, ReadOptions options) = 0;

        virtual Status queueSong(int song_id);
        virtual Status songFinished();

    };
}
}

#endif

