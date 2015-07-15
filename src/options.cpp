#include "skrillex/options.hpp"

namespace skrillex {
    Options::Options()
    : create_if_missing(false)
    , recreate(false)
    , enable_caching(true)
    , memory_only(false)
    , session_id(0)
    {
    }

    ReadOptions::ReadOptions()
    : session_id(0)
    , result_limit(0)
    , sort(SortType::Counts)
    {
    }

    WriteOptions::WriteOptions()
    : session_id(0)
    {
    }

    Options Options::InMemoryOptions() {
        Options options;
        options.memory_only = true;
        return options;
    }

    Options Options::TestOptions() {
        Options options;
        options.create_if_missing = true;
        options.recreate          = true;
        return options;
    }
}

