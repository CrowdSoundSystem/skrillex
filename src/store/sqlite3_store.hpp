#ifndef skrillex_sqlite3store_hpp
#define skrillex_sqlite3store_hpp

#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "skrillex/dbo.hpp"
#include "skrillex/options.hpp"
#include "skrillex/result_set.hpp"
#include "skrillex/status.hpp"

#include "store/store.hpp"
#include "store/sqlite3_bootstrap.hpp"
#include "sqlite3/sqlite3.h"

namespace skrillex {
namespace internal {
    class Sqlite3Store : public Store {
    public:
        Sqlite3Store();
        Sqlite3Store(const Sqlite3Store& other) = delete;
        Sqlite3Store(Sqlite3Store&& other)      = delete;
        ~Sqlite3Store();

        Status open(std::string path, Options options);

        Status getSongs(ResultSet<Song>& set, ReadOptions options);
        Status getArtists(ResultSet<Artist>& set, ReadOptions options);
        Status getGenres(ResultSet<Genre>& set, ReadOptions options);

        Status getSongFromId(Song& s, int songId);

        Status getPlayHistory(ResultSet<Song>& set, ReadOptions options);

        Status getQueue(ResultSet<Song>& set);
        Status queueSong(int songId);
        Status clearQueue();

        Status getBuffer(ResultSet<Song>& set);
        Status bufferNext();
        Status removeFromBuffer(int songId);
        Status songFinished();

        Status setActivity(std::string userId, int64_t timestamp);

        Status addSong(Song& song);
        Status addArtist(Artist& artist);
        Status addGenre(Genre& genre);

        Status insertNormalized(std::string normalized, int songId, int artistId, int genreId);
        Status getNormalized(Song& song, std::string normalizedName);

        Status markUnplayable(int songId);

        Status voteSong(std::string userId, Song& song, int amount, WriteOptions options);
        Status voteArtist(std::string userId, Artist& artist, int amount, WriteOptions options);
        Status voteGenre(std::string userId, Genre& genre, int amount, WriteOptions options);

        Status createSession();
        Status createSession(int64_t& result);

        Status getSession(int64_t& result);
        Status getSessionCount(int& result);

        Status getSessionUserCount(int& userCount, ReadOptions options);
    private:
        Status insertUser(std::string userId);

    private:
        sqlite3* db_;

        std::mutex queue_lock_;
        std::vector<Song> song_queue_;
        std::set<int> unplayable_song_ids_;

        std::mutex buffer_lock_;
        std::set<int> song_buffer_ids_;
        std::vector<Song> song_buffer_;


	int64_t session_id_;
    };
}
}

#endif

