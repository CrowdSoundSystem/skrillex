//
// parser.hpp
//
// The Parser interface.
//
// The Parser interface is not stateless, and depends
// on a DB to make more accurate estimations.
// In this sense, Parser is a slight misnomer, but
// is named as such to be consistent with design docs.
//

#ifndef skrillex_parser_hpp
#define skrillex_parser_hpp

#include <memory>
#include <boost/optional.hpp>

#include "skrillex/db.hpp"
#include "skrillex/dbo.hpp"
#include "skrillex/status.hpp"

namespace skrillex {
    struct ParseResult {
        boost::optional<Song> song_;
        boost::optional<Artist> artist_;
        boost::optional<Genre> genre_;
    };

    class Parser{
    private:
        std::shared_ptr<DB> db_;

    public:
        Parser(std::shared_ptr<DB> db);
        ~Parser();

        // Overloads parse with autoAdd = true.
        Status parse(ParseResult& result, std::string name, std::string artist, std::string genre);

        // Parses the input data, attempting to lookup existing corresponding entries.
        // If autoAdd = true, parse will automatically.
        Status parse(ParseResult& result, std::string name, std::string artist, std::string genre, bool autoAdd);
    };
}

#endif

