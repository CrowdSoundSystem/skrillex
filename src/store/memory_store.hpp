#ifndef skrillex_memory_store_hpp
#define skrillex_memory_store_hpp

#include <map>
#include <set>
#include <vector>

#include "skrillex/dbo.hpp"
#include "skrillex/options.hpp"
#include "skrillex/result_set.hpp"
#include "skrillex/status.hpp"
#include "store/store.hpp"

namespace skrillex {
namespace internal {
    class MemoryStore : public Store {
    public:
        MemoryStore();
        MemoryStore(int session_id);
        MemoryStore(const MemoryStore& other) = delete;
        MemoryStore(MemoryStore&& other)      = delete;
        ~MemoryStore();

        Status getSongs(ResultSet<Song>& set, ReadOptions options);
        Status getArtists(ResultSet<Artist>& set, ReadOptions options);
        Status getGenres(ResultSet<Genre>& set, ReadOptions options);

        Status getPlayHistory(ResultSet<Song>& set, ReadOptions options);

        Status getQueue(ResultSet<Song>& set);
        Status queueSong(int songId);
        Status songFinished();

        Status addSong(Song& song);
        Status addArtist(Artist& artist);
        Status addGenre(Genre& genre);

        Status countSong(Song& song, WriteOptions options);
        Status countArtist(Artist& artist, WriteOptions options);
        Status countGenre(Genre& genre, WriteOptions options);

        Status voteSong(Song& song, int amount, WriteOptions options);
        Status voteArtist(Artist& artist, int amount, WriteOptions options);
        Status voteGenre(Genre& genre, int amount, WriteOptions options);

        Status createSession();
        Status createSession(int& result);
        Status changeSession(int session);
        Status getSession(int& result);
        Status getSessionCount(int& result);

    private:
        std::map<int, std::vector<Song>>  songs_;
        std::map<int, std::vector<Artist>> artists_;
        std::map<int, std::vector<Genre>>  genres_;

        std::vector<Song>    song_queue_;
        std::set<int>       sessions_;

        int song_id_counter_;
        int artist_id_counter_;
        int genre_id_counter_;

        int session_id_;
    };
}
}
#endif

