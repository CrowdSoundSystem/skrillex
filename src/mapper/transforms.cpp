#include <algorithm>
#include <cctype>
#include <functional>
#include <string>

#include "mapper/transforms.hpp"

using namespace std;

namespace skrillex {
namespace internal {
    string normalize(FieldType type, string value) {
        // Remove everything that is not alphanumeric.
        value.erase(remove_if(value.begin(), value.end(), [](char x) { return !isalnum(x); }), value.end());

        // Convert to lowercase.
        transform(value.begin(), value.end(), value.begin(), ::tolower);

        switch (type) {
            case GenreField:  return "G" + value;
            case ArtistField: return "A" + value;
            case SongField:
            case None:
                return value;
        }
    }

    string combine(string song, string artist) {
        return normalize(FieldType::None, artist) + normalize(FieldType::SongField, song);
    }
}
}
