#include <iostream>
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

TEST(Sqlite3DatabaseTests, Init) {
    DB* raw = 0;

    {
        Status s = open(raw, "test.db", Options::TestOptions());
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

TEST(Sqlite3DatabaseTests, PopulateEmpty) {
    DB* raw = 0;
    Status s = open(raw, "test.db", Options::TestOptions());
    ASSERT_EQ(Status::OK(), s);

    shared_ptr<DB> db(raw);
    s = populate_empty(raw, 100, 10, 11);
    EXPECT_EQ(Status::OK(), s);

    ResultSet<Artist> artists;
    s = db->getArtists(artists);
    EXPECT_EQ(Status::OK(), s);
    EXPECT_EQ(10, artists.size());
    for (auto& a : artists) {
        EXPECT_EQ("a", a.name.substr(0, 1));
        EXPECT_EQ(0, a.votes);
        EXPECT_EQ(0, a.count);
    }

    ResultSet<Genre> genres;
    s = db->getGenres(genres);
    EXPECT_EQ(Status::OK(), s);
    EXPECT_EQ(11, genres.size());
    for (auto& g : genres) {
        EXPECT_EQ("g", g.name.substr(0, 1));
        EXPECT_EQ(0, g.votes);
        EXPECT_EQ(0, g.count);
    }
}
