#ifndef skrillex_memory_store_hpp
#define skrillex_memory_store_hpp

#include <queue>
#include <vector>

#include "skrillex/options.hpp"
#include "skrillex/result_set.hpp"
#include "skrillex/status.hpp"
#include "store/store.hpp"

namespace skrillex {
namespace internal {
    class MemoryStore : public Store {
    public:
        MemoryStore(const MemoryStore& other) = delete;
        MemoryStore(MemoryStore&& other)      = delete;

        Status getSongs(ResultSet<Song>& set, ReadOptions options);
        Status getArtists(ResultSet<Artist>& set, ReadOptions options);
        Status getGenres(ResultSet<Genre>& set, ReadOptions options);

        Status getPlayHistory(ResultSet<Song>& set, ReadOptions options);

        Status queueSong(int songId);
        Status songFinished();

    private:
        std::vector<Song>   songs_;
        std::vector<Artist> artists_;
        std::vector<Genre>  genres_;

        std::queue<Song>    song_queue_;
    };
}
}
#endif
