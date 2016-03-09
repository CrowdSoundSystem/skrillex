#include "skrillex/dbo.hpp"

namespace skrillex {
    Countable::Countable() : count(0), votes(0) {}

    Artist::Artist() : id(0), name(""), last_played(0) {}
    Artist::Artist(const Artist& o)
    : id(o.id)
    , name(o.name)
    , last_played(o.last_played)
    {
    }

    Genre::Genre() : id(0), name(""), last_played(0) {}
    Genre::Genre(const Genre& o)
    : id(o.id)
    , name(o.name)
    , last_played(o.last_played)
    {
    }

    Song::Song() : id(0), name(""), last_played(0) {}
    Song::Song(const Song& o)
    : id(o.id)
    , name(o.name)
    , last_played(o.last_played)
    , artist(o.artist)
    , genre(o.genre)
    {
    }

    bool operator==(const Artist& a, const Artist& b) {
        return a.id == b.id;
    }

    std::ostream& operator<<(std::ostream& os, const Artist& artist) {
        os << "Artist{id=" << artist.id << ", name=" << artist.name << ", count=" << artist.count << ", votes=" << artist.votes << "}";
        return os;
    }

    bool operator==(const Genre& a, const Genre& b) {
        return a.id == b.id;
    }

    std::ostream& operator<<(std::ostream& os, const Genre& genre) {
        os << "Genre{id=" << genre.id << ", name=" << genre.name << ", count=" << genre.count << ", votes=" << genre.votes << "}";
        return os;
    }

    bool operator==(const Song& a, const Song& b) {
        return
            a.artist == b.artist &&
            a.genre == b.genre &&
            a.id == b.id;
    }

    std::ostream& operator<<(std::ostream& os, const Song& song) {
        os << "Song{id=" << song.id << ", name=" << song.name << ", count=" << song.count << ", votes=" << song.votes << "}";
        return os;
    }

}

