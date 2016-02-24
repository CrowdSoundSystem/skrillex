#ifndef skrillex_store_hpp
#define skrillex_store_hpp

#include <string>

#include "skrillex/options.hpp"
#include "skrillex/result_set.hpp"
#include "skrillex/status.hpp"

namespace skrillex {
namespace internal {
    class Store {
    public:
        virtual ~Store() { }

        virtual Status open(std::string db, Options options) = 0;

        virtual Status getSongs(ResultSet<Song>& set, ReadOptions options) = 0;
        virtual Status getArtists(ResultSet<Artist>& set, ReadOptions options) = 0;
        virtual Status getGenres(ResultSet<Genre>& set, ReadOptions options) = 0;

        virtual Status getPlayHistory(ResultSet<Song>& set, ReadOptions options) = 0;

        virtual Status getQueue(ResultSet<Song>& set) = 0;
        virtual Status queueSong(int song_id) = 0;
        virtual Status clearQueue() = 0;
        virtual Status songFinished() = 0;

        virtual Status getBuffer(ResultSet<Song>& set) = 0;
        virtual Status bufferSong(int song_id) = 0;

        virtual Status setActivity(std::string userId, int64_t timestamp) = 0;

        virtual Status addSong(Song& song) = 0;
        virtual Status addArtist(Artist& artist) = 0;
        virtual Status addGenre(Genre& genre) = 0;

        virtual Status insertNormalized(std::string normalized, int songID, int artistID, int genreID) = 0;
        virtual Status getNormalized(Song& song, std::string normalizedName) = 0;

        virtual Status voteSong(std::string userId, Song& s, int amount, WriteOptions options) = 0;
        virtual Status voteArtist(std::string userId, Artist& s, int amount, WriteOptions options) = 0;
        virtual Status voteGenre(std::string userId, Genre& s, int amount, WriteOptions options) = 0;

        virtual Status createSession() = 0;
        virtual Status createSession(int64_t& result) = 0;

        virtual Status getSession(int64_t& result) = 0;
        virtual Status getSessionCount(int& result) = 0;
    };
}
}

#endif

