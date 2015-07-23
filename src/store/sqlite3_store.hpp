#ifndef skrillex_sqlite3store_hpp
#define skrillex_sqlite3store_hpp

#include <memory>
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

        Status getPlayHistory(ResultSet<Song>& set, ReadOptions options);

        Status getQueue(ResultSet<Song>& set);
        Status queueSong(int songId);
        Status songFinished();

        Status addSong(Song& song);
        Status addArtist(Artist& artist);
        Status addGenre(Genre& genre);

        Status countSong(Song& song, int amount, WriteOptions options);
        Status countArtist(Artist& artist, int amount, WriteOptions options);
        Status countGenre(Genre& genre, int amount, WriteOptions options);

        Status voteSong(Song& song, int amount, WriteOptions options);
        Status voteArtist(Artist& artist, int amount, WriteOptions options);
        Status voteGenre(Genre& genre, int amount, WriteOptions options);

        Status createSession();
        Status createSession(int& result);

        Status getSession(int& result);
        Status getSessionCount(int& result);
    private:
        sqlite3* db_;
        std::vector<Song> song_queue_;
    };
}
}

#endif

