//
// mapper.hpp
//
// The Mapper interface.
//
// The Mapper interface is used to map user input data
// to cannonical database objects.
//

#ifndef skrillex_parser_hpp
#define skrillex_parser_hpp

#include <memory>
#include <boost/optional.hpp>

#include "skrillex/db.hpp"
#include "skrillex/dbo.hpp"
#include "skrillex/status.hpp"

namespace skrillex {
    class Mapper {
    private:
        std::shared_ptr<DB> db_;

    public:
        Mapper(std::shared_ptr<DB> db);
        ~Mapper();

        // Map takes in three inputs, the song name, artist name, and genre name.
        //
        // It then attempts to map each value to their respective DBO object. If
        // a mapping cannot be found, a new DBO is created for the missing value.
        //
        // If any of the input fields are empty (with the exception of SongName),
        // they are ignored.
        //
        // Result the mapped Song object.
        //
        // Note: This function may modify the underlying store.
        Status map(Song& result, std::string song, std::string artist, std::string genre);

        // Lookup attempts to lookup a corresponding song for a given
        // <song name, artist name> combination. If a song cannot be
        // found, Status::NotFound() is returned.
        Status lookup(Song& result, std::string song, std::string artist);
    };
}

#endif

