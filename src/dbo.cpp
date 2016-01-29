#include "skrillex/dbo.hpp"

namespace skrillex {
    bool operator==(const Artist& a, const Artist& b) {
        return a.id == b.id;
    }

    bool operator==(const Genre& a, const Genre& b) {
        return a.id == b.id;
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

