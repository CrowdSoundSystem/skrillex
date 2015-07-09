#include <gtest/gtest.h>
#include <memory>

#include "skrillex/db.hpp"

using namespace std;
using namespace skrillex;

TEST(InMemoryDatabaseTests, Init) {
    DB* raw = 0;

    {
        Status s = open(raw, "", Options::InMemoryOptions());
        shared_ptr<DB> db(raw);

        ASSERT_TRUE(s.ok()) << s.string();
        ASSERT_TRUE(db.get());
        ASSERT_TRUE(db->isOpen());
    }

    // Since we wrapped the raw pointer in
    // a shared pointer in a scope, it should
    // have cleaned up, calling the destructor
    // and closing the database. This works
    // on linux, but may fail on a debugger that
    // scrambles memory, in which case this should
    // be handled better.
    ASSERT_FALSE(raw->isOpen());
}

TEST(InMemoryDatabaseTests, Songs) {
    DB* raw = 0;
    Status s = open(raw, "", Options::InMemoryOptions());
    shared_ptr<DB> db(raw);

    // Populate songs
}
