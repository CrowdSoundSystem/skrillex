//
// dbo.hpp
//
// Objects wrapping internal database objects
//

#ifndef skrillex_dbo_hpp
#define skrillex_dbo_hpp

#include <string>

namespace skrillex {

struct SessionData {
    int session_id;
};

struct Countable : public SessionData {
    int count;
    int votes;
};

struct Artist : public Countable {
    int         id;
    std::string name;

    // Unix timestamp
    uint64_t    last_played;

    friend bool operator==(const Artist& a, const Artist& b);
};

struct Genre : public Countable {
    int         id;
    std::string name;

    // Unix timestamp
    uint64_t    last_played;
    friend bool operator==(const Genre& a, const Genre& b);
};

struct Song : public Countable {
    int         id;
    Artist      artist;
    Genre       genre;
    std::string name;

    // Unix timestamp
    uint64_t    last_played;

    friend bool operator==(const Song& a, const Song& b);

};

}

#endif
