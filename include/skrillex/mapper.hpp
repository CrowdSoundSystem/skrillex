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

        Status map(Song& result, std::string song, std::string artist, std::string genre);
        Status lookup(Song& result, std::string song, std::string artist);
    };
}

#endif

