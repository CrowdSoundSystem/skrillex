#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "skrillex/db.hpp"
#include "skrillex/dbo.hpp"
#include "skrillex/testing/populator.hpp"

#include "store/store.hpp"
#include "mutator.hpp"

using namespace std;
using namespace skrillex;
using namespace skrillex::internal;
using namespace skrillex::testing;

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

TEST(InMemoryDatabaseTests, InsertAndGet) {
    DB* raw = 0;
    Status s = open(raw, "", Options::InMemoryOptions());
    ASSERT_EQ(Status::OK(), s);

    shared_ptr<DB> db(raw);
    populate_empty(raw, 10, 10, 100);

    vector<Genre> genres;
    vector<Artist> artists;
    vector<Song> songs;
}
