//
// db.hpp
//
// The DB object is the primary interface to the actual database.
//
// By default, a  DB instance has an associated session, which is
// created upon creation. All queries and updates to the database
// will be done so under the DB objects session.
//
// In order to access data from *another* session, one can simply
// set the session id in the Options when calling a query.
//
// Alternatively, a database can 'restore' to an existing session
// upon creation time. It should be noted that once a DB object
// is initialized, the session *cannot* be changed.
//

#ifndef skrillex_db_hpp
#define skrillex_db_hpp

#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "skrillex/options.hpp"
#include "skrillex/result_set.hpp"
#include "skrillex/status.hpp"

namespace skrillex {
    namespace internal {
        class Store;
        class StoreMutator;
    }

    class Mapper;

    class DB;
    Status open(DB*& db, std::string path, Options options);

    class DB {
    public:
        ~DB();

        bool isOpen() const;
        void close();

        Status getSongs(ResultSet<Song>& set);
        Status getSongs(ResultSet<Song>& set, ReadOptions options);

        Status getArtists(ResultSet<Artist>& set);
        Status getArtists(ResultSet<Artist>& set, ReadOptions options);

        Status getGenres(ResultSet<Genre>& set);
        Status getGenres(ResultSet<Genre>& set, ReadOptions options);

        Status getPlayHistory(ResultSet<Song>& set);
        Status getPlayHistory(ResultSet<Song>& set, ReadOptions options);

        Status setQueue(std::vector<int> songIds);
        Status getQueue(ResultSet<Song>& set);
        Status queueSong(int song_id);
        Status clearQueue();

        Status getBuffer(ResultSet<Song>& buffer);
        Status bufferNext();
        Status removeFromBuffer(int songId);
        Status songFinished();

        Status setActivity(std::string userId, int64_t timestamp);

        Status addSong(Song& s);
        Status addArtist(Artist& artist);
        Status addGenre(Genre& genre);

        Status markUnplayable(int songId);

        Status voteSong(std::string userId, Song& song, int amount);
        Status voteArtist(std::string userId, Artist& artist, int amount);
        Status voteGenre(std::string userId, Genre& genre, int amount);

        Status getSessionUserCount(int& userCount);
        Status getSessionUserCount(int& userCount, ReadOptions options);
    private:
        DB(std::string path, Options options);
        DB(const DB& other)  = delete;
        DB(const DB&& other) = delete;

        friend Status open(DB*& db, std::string path, Options options);
        friend class internal::StoreMutator;
        friend class Mapper;
    private:
        enum class State {
            Closed,
            Open
        };

        std::string db_path_;
        Options     db_options_;
        State       db_state_;
        int64_t     session_id_;

        std::unique_ptr<internal::Store> store_;
    };
}

#endif

