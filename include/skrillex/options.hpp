//
// options.hpp
//
// Options are a way of customizing certain operations to a DB
// object, such as initializing, querying, and updating.
//

#ifndef skrillex_options_hpp
#define skrillex_options_hpp

namespace skrillex {

enum SortType {
    None,
    Counts,
    Votes
};

struct Options {
    // Create the underlying database if missing.
    //
    // Default: false
    bool create_if_missing;

    // If a database already exists, recreate the
    // database, resulting in a brand new state.
    //
    // Default: false
    bool recreate;

    // Enables caching of intermediate results.
    //
    // Default: true
    bool enable_caching;

    // Sets the database session. This session will
    // be the ones used by default for ReadOptions
    // and WriteOptions.
    //
    // Special Cases:
    //     0: Create a new session
    //
    // Default: 0
    int session_id;

    Options();

    static Options TestOptions();
};

struct ReadOptions {
    // Reads will only use data from the specified session.
    //
    // Special Cases:
    //     0: Current Session
    //    -1: Everything except the current session
    //
    // Default: 0
    int session_id;

    // The maximum amount of results returned.
    // If zero, no limit is applied
    //
    // Default: 0
    int result_limit;

    // The sorting to be applied when reading.
    //
    // Default: Counts
    SortType sort;

    // The amount of time a user can be 'inactive' before
    // there data is no longer returned. If zero, then a
    // user can be inactive an infinite amount of time.
    //
    // Default: 300 (5 minutes)
    int inactivity_threshold;

    ReadOptions();
};

struct WriteOptions {
    // Writes will only persist to the specified session.
    //
    // Special Cases:
    //     0: Current Session
    //
    // Default: 0
    int session_id;

    WriteOptions();
};

}
#endif

