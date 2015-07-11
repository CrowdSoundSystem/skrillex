#ifndef skrillex_store_hpp
#define skrillex_store_hpp

#include "skrillex/options.hpp"
#include "skrillex/result_set.hpp"
#include "skrillex/status.hpp"

namespace skrillex {
namespace internal {
    class Store {
    public:
        virtual ~Store() { }

        virtual Status getSongs(ResultSet<Song>& set, ReadOptions options) = 0;
        virtual Status getArtists(ResultSet<Artist>& set, ReadOptions options) = 0;
        virtual Status getGenres(ResultSet<Genre>& set, ReadOptions options) = 0;

        virtual Status getPlayHistory(ResultSet<Song>& set, ReadOptions options) = 0;

        virtual Status getQueue(ResultSet<Song>& set) = 0;
        virtual Status queueSong(int song_id) = 0;
        virtual Status songFinished() = 0;

        virtual Status addSong(Song& song) = 0;
        virtual Status addArtist(Artist& artist) = 0;
        virtual Status addGenre(Genre& genre) = 0;

        virtual Status voteSong(Song& s, int amount, WriteOptions options) = 0;
        virtual Status voteArtist(Artist& s, int amount, WriteOptions options) = 0;
        virtual Status voteGenre(Genre& s, int amount, WriteOptions options) = 0;

        virtual Status createSession() = 0;
        virtual Status createSession(int& result) = 0;
        virtual Status getSession(int& result) = 0;
    };
}
}

#endif

