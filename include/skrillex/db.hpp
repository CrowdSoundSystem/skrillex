//
// db.hpp
//
// The DB object is the primary interface to the actual database.
//
// By default, a  DB instance has an associated session, which is
// created upon creation. All queries and updates to the database
// will be done so under the DB objects session.
//
// In order to access data from *another* session, one can simply
// set the session id in the Options when calling a query.
//
// Alternatively, a database can 'restore' to an existing session
// upon creation time. It should be noted that once a DB object
// is initialized, the session *cannot* be changed.
//

#ifndef skrillex_db_hpp
#define skrillex_db_hpp

#include <memory>

#include "skrillex/options.hpp"
#include "skrillex/result_set.hpp"
#include "skrillex/status.hpp"

namespace skrillex {
    class DB {
    public:
        Status getSongs(ResultSet<Song>& set);
        Status getSongs(ResultSet<Song>& set, ReadOptions options);

        Status getArtists(ResultSet<Artist>& set);
        Status getArtists(ResultSet<Artist>& set, ReadOptions options);

        Status getGenres(ResultSet<Genre>& set);
        Status getGenres(ResultSet<Genre>& set, ReadOptions options);

        Status songPlayed(Song& song);
        Status songPlayed(Song& song, WriteOptions options);

        Status songFinished(Song& song);
        Status songFinished(Song& song, WriteOptions options);
    };

}

#endif

