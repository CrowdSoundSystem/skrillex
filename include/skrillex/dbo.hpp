//
// dbo.hpp
//
// Objects wrapping internal database objects
//

#ifndef skrillex_dbo_hpp
#define skrillex_dbo_hpp

#include <string>

struct SessionData {
    int session_id;
};

struct Countable {
    int count;
    int votes;
};

struct Artist : public SessionData, Countable {
    int         id;
    std::string name;

    // Unix timestamp
    uint64_t    last_played;
};

struct Genre : public SessionData, Countable {
    int         id;
    std::string name;

    // Unix timestamp
    uint64_t    last_played;
};

struct Song : public SessionData, Countable {
    int         id;
    Artist      artist;
    Genre       genre;
    std::string name;

    // Unix timestamp
    uint64_t    last_played;
};

#endif
