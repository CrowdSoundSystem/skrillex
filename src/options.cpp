#include "skrillex/options.hpp"

namespace skrillex {
    Options::Options()
    : create_if_missing(false)
    , recreate(false)
    , enable_caching(true)
    , session_id(0)
    {
    }

    ReadOptions::ReadOptions()
    : session_id(0)
    , result_limit(0)
    , sort(SortType::Counts)
    , filter_buffered(true)
    , inactivity_threshold(1800000)
    {
    }

    WriteOptions::WriteOptions()
    : session_id(0)
    {
    }

    Options Options::TestOptions() {
        Options options;
        options.create_if_missing = true;
        options.recreate          = true;
        return options;
    }
}

