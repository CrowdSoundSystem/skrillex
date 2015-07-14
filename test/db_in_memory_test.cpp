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
        EXPECT_EQ(data.artists[((s.id - 1) % 10)], s.artist);
        EXPECT_EQ(data.genres[((s.id - 1) % 11)], s.genre);
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

TEST(InMemoryDatabaseTests, PopulateFull) {
    ReadOptions voteSort;
    voteSort.sort = SortType::Votes;

    DB* raw = 0;
    Status s = open(raw, "", Options::InMemoryOptions());
    ASSERT_EQ(Status::OK(), s);

    shared_ptr<DB> db(raw);
    populate_full(raw, 10, 10, 11, 3);

    Store* store = StoreMutator::getStore(raw);
    PopulatorData data = get_populator_data(10, 10, 11);

    int session_id    = 0;
    int session_count = 0;
    store->getSession(session_id);
    store->getSessionCount(session_count);
    EXPECT_EQ(3, session_id);
    EXPECT_EQ(3, session_count);

    ResultSet<Song> songs;
    int max_count = 0;
    int max_votes  = 0;

    // Test vote ordering (non-default)
    s = db->getSongs(songs, voteSort);
    EXPECT_EQ(Status::OK(), s);
    int last_count = 0;
    int last_vote  = 10000;
    for (auto& s : songs) {
        EXPECT_GT(last_vote, s.votes);
        EXPECT_LT(last_count, s.count);

        if (s.votes > max_votes) {
            max_votes = s.votes;
        }
        if (s.count > max_count) {
            max_count = s.count;
        }

        last_vote  = s.votes;
        last_count = s.count;
    }

    // Test count ordering (default)
    s = db->getSongs(songs);
    EXPECT_EQ(Status::OK(), s);
    last_count = 10000;
    last_vote  = 0;
    for (auto& s : songs) {
        EXPECT_GT(last_count, s.count);
        EXPECT_LT(last_vote, s.votes);

        last_count = s.count;
        last_vote  = s.votes;
    }

    // Let's go back in time!
    s = store->changeSession(2);
    EXPECT_EQ(Status::OK(), s);

    ResultSet<Song> oldSongs;

    // The counts should be different!
    s = db->getSongs(oldSongs);
    EXPECT_EQ(Status::OK(), s);
    EXPECT_GT(max_count, oldSongs.begin()->count);

    s = db->getSongs(oldSongs, voteSort);
    EXPECT_EQ(Status::OK(), s);
    EXPECT_GT(max_votes, oldSongs.begin()->votes);

    // Test artist and genre too -_-
    ResultSet<Artist> artists;
    s = db->getArtists(artists, voteSort);
    EXPECT_EQ(Status::OK(), s);
    last_count = 0;
    last_vote  = 10000;
    for (auto& a : artists) {
        EXPECT_GT(last_vote, a.votes);
        EXPECT_LT(last_count, a.count);

        last_vote  = a.votes;
        last_count = a.count;
    }

    s = db->getArtists(artists);
    EXPECT_EQ(Status::OK(), s);
    last_count = 10000;
    last_vote  = 0;
    for (auto& a : artists) {
        EXPECT_LT(last_vote, a.votes);
        EXPECT_GT(last_count, a.count);

        last_vote  = a.votes;
        last_count = a.count;
    }

    ResultSet<Genre> genres;
    s = db->getGenres(genres, voteSort);
    EXPECT_EQ(Status::OK(), s);
    last_count = 0;
    last_vote  = 10000;
    for (auto& g : genres) {
        EXPECT_GT(last_vote, g.votes);
        EXPECT_LT(last_count, g.count);

        last_vote  = g.votes;
        last_count = g.count;
    }

    s = db->getGenres(genres);
    EXPECT_EQ(Status::OK(), s);
    last_count = 10000;
    last_vote  = 0;
    for (auto& g : genres) {
        EXPECT_LT(last_vote, g.votes);
        EXPECT_GT(last_count, g.count);

        last_vote  = g.votes;
        last_count = g.count;
    }
}
