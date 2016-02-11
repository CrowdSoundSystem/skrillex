#ifndef skrillex_parser_transforms_hpp
#define skrillex_parser_transforms_hpp

#include <string>

namespace skrillex {
namespace internal {
    enum FieldType {
        None,
        ArtistField,
        GenreField
    };

    // Normalizes a given input based on its field type.
    std::string normalize(FieldType type, std::string value);

    // Combine normalized fields into a single field.
    std::string combine(std::string song, std::string artist);
}
}

#endif

