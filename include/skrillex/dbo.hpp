//
// dbo.hpp
//
// Objects wrapping internal database objects
//

#ifndef skrillex_dbo_hpp
#define skrillex_dbo_hpp

#include <iostream>
#include <string>

namespace skrillex {

struct Countable {
    Countable();

    int count;
    int votes;
};

struct Artist : public Countable {
    Artist();
    Artist(const Artist&);

    int         id;
    std::string name;

    // Unix timestamp
    uint64_t    last_played;

    friend bool operator==(const Artist& a, const Artist& b);
    friend std::ostream& operator<<(std::ostream& os, const Artist& artist);
};

struct Genre : public Countable {
    Genre();
    Genre(const Genre&);

    int         id;
    std::string name;

    // Unix timestamp
    uint64_t    last_played;
    friend bool operator==(const Genre& a, const Genre& b);
    friend std::ostream& operator<<(std::ostream& os, const Genre& genre);
};

struct Song : public Countable {
    Song();
    Song(const Song&);

    int         id;
    Artist      artist;
    Genre       genre;
    std::string name;

    // Unix timestamp
    uint64_t    last_played;

    friend bool operator==(const Song& a, const Song& b);
    friend std::ostream& operator<<(std::ostream& os, const Song& song);
};

std::ostream& operator<<(std::ostream& os, const Artist& artist);
std::ostream& operator<<(std::ostream& os, const Genre& genre);
std::ostream& operator<<(std::ostream& os, const Song& song);

}

#endif
