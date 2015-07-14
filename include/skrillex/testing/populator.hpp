#ifndef skrillex_testing_populator_hpp
#define skrillex_testing_populator_hpp

#include <vector>

#include "skrillex/db.hpp"
#include "skrillex/status.hpp"

namespace skrillex {
namespace testing {
    struct PopulatorData {
        std::vector<Song>   songs;
        std::vector<Artist> artists;
        std::vector<Genre>  genres;
    };

    /*
     * Returns the data that should be populated
     * into an empty database after calling populat_empty()
     * or populate_full().
     *
     * Does not include anything about or vote information
     */
    PopulatorData get_populator_data(int num_songs, int num_artists, int num_genres);

    /*
     * Populates a database with basic data,
     * without any session information.
     *
     * The results are deterministic.
     */
    Status populate_empty(DB* db, int num_songs, int num_artists, int num_genres);

    /*
     * Populates a database with basic data,
     * as well as multiple sessions with various
     * vote and counts.
     *
     * The results are deterministic.
     */
    Status populate_full(DB* db, int num_songs, int num_artists, int num_genres, int num_sessions);
}
}

#endif

