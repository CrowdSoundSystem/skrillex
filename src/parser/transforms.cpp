#include <string>

#include <boost/algorithm/string.hpp>

#include "parser/transforms.hpp"

using namespace std;
using namespace boost::algorithm;

namespace skrillex {
namespace internal {
    string normalize(string song, string artist, string genre) {
        // General normalization:
        //     1. Remove whitespace.
        //     2. Convert to lowercase
        //     3. Remove special characters.
        //
        // How do we deal with cross-field collisions? For example,
        // there is a song called Kanye, but it's actually by his
        // mortal enemy, Taylor Swift. However, the library was
        // dirty, so no artist / genre information was provided.
        // Our current approach will normalize this song into the
        // artist Kanye, which is in fact incorrect.
        //
        // Alternatively, suppose we have a song called Kanye - Gay fish.
        // However, the entry is dirty, and it's all in the song field.
        // The normalization result may match the correct one!
        //
        // To try and deal with these cases, we can take a few
        // approaches:
        //     1. If only one field is present, ignore the other two.
        //     2. If only song is present, ignore other fields.
        //        If only artist/genre is present, replace the
        //        song field with some special flag, to prevent
        //        collision.
        //
        // Approach 1. allows for the case that all the information
        // can be stuck in a single field. Approach 2. assumes that
        // dirty data will only be in the song field, and that genres
        // and artists are definitely not songs. This property is
        // mostly important for voting, where we don't want to
        // accidentally vote on a song, when only genre or artist
        // was specified. From an unverified guess, it is likely
        // that most libraries won't allow entries with no title,
        // so this assumption *probably& helps support 2.
        trim(song);
        trim(artist);
        trim(genre);

        return "";
    }
}
}
