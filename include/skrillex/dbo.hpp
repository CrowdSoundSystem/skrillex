//
// dbo.hpp
//
// Objects wrapping internal database objects
//

#ifndef skrillex_dbo_hpp
#define skrillex_dbo_hpp

#include <string>

class Countable {
    int count;
    int votes;
};

class Artist : public Countable {
    int         id;
    std::string name;
};

class Genre : public Countable {
    int         id;
    std::string name;
};

class Song : public Countable {
    int         id;
    Artist      artist;
    Genre       genre;
    std::string name;
};

#endif
