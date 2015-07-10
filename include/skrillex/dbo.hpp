//
// dbo.hpp
//
// Objects wrapping internal database objects
//

#ifndef skrillex_dbo_hpp
#define skrillex_dbo_hpp

#include <string>

struct Countable {
    int count;
    int votes;
};

struct Artist : public Countable {
    int         id;
    std::string name;

    // Unix timestamp
    uint64_t    last_played;
};

struct Genre : public Countable {
    int         id;
    std::string name;

    // Unix timestamp
    uint64_t    last_played;
};

struct Song : public Countable {
    int         id;
    Artist      artist;
    Genre       genre;
    std::string name;

    // Unix timestamp
    uint64_t    last_played;
};

#endif
