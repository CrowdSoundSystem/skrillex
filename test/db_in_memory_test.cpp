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

TEST(InMemoryDatabaseTests, PopulateEmpty) {
    DB* raw = 0;
    Status s = open(raw, "", Options::InMemoryOptions());
    ASSERT_EQ(Status::OK(), s);

    shared_ptr<DB> db(raw);
    populate_empty(raw, 100, 10, 11);

    Store* store = StoreMutator::getStore(raw);
    PopulatorData data = get_populator_data(100, 10, 11);

    int session_count = 0;
    store->getSessionCount(session_count);
    EXPECT_EQ(1, session_count);

    ResultSet<Song> songs;
    s = db->getSongs(songs);
    EXPECT_EQ(Status::OK(), s);
    EXPECT_EQ(100, songs.size());
    for (auto& s : songs) {
        EXPECT_EQ(data.songs[s.id - 1], s);
        EXPECT_EQ(data.artists[(s.id - 1) % 10], s.artist);
        EXPECT_EQ(data.genres[(s.id - 1) % 11], s.genre);
    }

    ResultSet<Artist> artists;
    s = db->getArtists(artists);
    EXPECT_EQ(Status::OK(), s);
    EXPECT_EQ(10, artists.size());
    for (auto& a : artists) {
        EXPECT_EQ(data.artists[a.id - 1], a);
    }

    ResultSet<Genre> genres;
    s = db->getGenres(genres);
    EXPECT_EQ(Status::OK(), s);
    EXPECT_EQ(11, genres.size());
    for (auto& g : genres) {
        EXPECT_EQ(data.genres[g.id - 1], g);
    }
}
