//
// db_sqlite3_test.cpp
//
// Tests the functionality of the SQLite3 store.
//
// TODO: A lot of this can be simplified and better tested,
// notably because Song, Artist, and Genre implement Countable.
// I can't believe I didn't even think about that. I'm 90% sure
// that's why I made them Countable. Ugh.
//

#include <iostream>
#include <gtest/gtest.h>

#include "skrillex/db.hpp"
#include "skrillex/dbo.hpp"
#include "skrillex/testing/populator.hpp"

#include "store/store.hpp"
#include "mutator.hpp"

#define NUM_SONGS 10
#define NUM_ARTISTS 5
#define NUM_GENRES 6

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
	return;
    ReadOptions limiter;

    DB* raw = 0;
    Status s = open(raw, "test.db", Options::TestOptions());
    ASSERT_EQ(Status::OK(), s);

    shared_ptr<DB> db(raw);
    s = populate_empty(raw, 100, 10, 11);
    EXPECT_EQ(Status::OK(), s);

    Store* store = StoreMutator::getStore(raw);
    PopulatorData data = get_populator_data(100, 10, 11);

    int session_count = 0;
    store->getSessionCount(session_count);
    EXPECT_EQ(1, session_count);

    // Test full get.
    ResultSet<Song> songs;
    s = db->getSongs(songs);
    EXPECT_EQ(Status::OK(), s);
    EXPECT_EQ(100, songs.size());
    for (auto& s : songs) {
        EXPECT_EQ(data.songs[s.id - 1], s);
        EXPECT_EQ(data.artists[((s.id - 1) % 10)], s.artist);
        EXPECT_EQ(data.genres[((s.id - 1) % 11)], s.genre);

        EXPECT_EQ(0, s.count);
        EXPECT_EQ(0, s.votes);
    }

    // Test limit
    limiter.result_limit = 30;
    s = db->getSongs(songs, limiter);
    EXPECT_EQ(Status::OK(), s);
    EXPECT_EQ(30, songs.size());

    // Test full get
    ResultSet<Artist> artists;
    s = db->getArtists(artists);
    EXPECT_EQ(Status::OK(), s);
    EXPECT_EQ(10, artists.size());
    for (auto& a : artists) {
        EXPECT_EQ(data.artists[a.id - 1], a);
    }

    // Test limit
    limiter.result_limit = 5;
    s = db->getArtists(artists, limiter);
    EXPECT_EQ(Status::OK(), s);
    EXPECT_EQ(5, artists.size());

    ResultSet<Genre> genres;
    s = db->getGenres(genres);
    EXPECT_EQ(Status::OK(), s);
    EXPECT_EQ(11, genres.size());
    for (auto& g : genres) {
        EXPECT_EQ(data.genres[g.id - 1], g);
    }

    // Test limit
    limiter.result_limit = 8;
    s = db->getGenres (genres, limiter);
    EXPECT_EQ(Status::OK(), s);
    EXPECT_EQ(8, genres.size());
}

TEST(Sqlite3DatabaseTests, PopulateFull) {
 ReadOptions readOptions;
    ReadOptions voteSort;
    voteSort.sort = SortType::Votes;

    DB* raw = 0;
    Status s = open(raw, "test.db", Options::TestOptions());
    ASSERT_EQ(Status::OK(), s);

    shared_ptr<DB> db(raw);
    s = populate_full(raw, NUM_SONGS, NUM_ARTISTS, NUM_GENRES, 3);
    EXPECT_EQ(Status::OK(), s);

    Store* store = StoreMutator::getStore(raw);
    PopulatorData data = get_populator_data(NUM_SONGS, NUM_ARTISTS, NUM_GENRES);

    int64_t session_id = 0;
    int session_count  = 0;
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
    readOptions.session_id = 2;
    voteSort.session_id  = 2;

    ResultSet<Song> oldSongs;

    // The counts should be different!
    s = db->getSongs(oldSongs, readOptions);
    EXPECT_EQ(Status::OK(), s);
    EXPECT_GT(max_count, oldSongs.begin()->count);

    s = db->getSongs(oldSongs, voteSort);
    EXPECT_EQ(Status::OK(), s);
    EXPECT_GT(max_votes, oldSongs.begin()->votes);

    // Test artist and genre too -_-
    ResultSet<Artist> artists;
    max_count = 0;
    max_votes = 0;

    s = db->getArtists(artists, voteSort);
    EXPECT_EQ(Status::OK(), s);
    last_count = 0;
    last_vote  = 10000;
    for (auto& a : artists) {
        EXPECT_GT(last_vote, a.votes);
        EXPECT_LT(last_count, a.count);

        if (a.votes > max_votes) {
            max_votes = a.votes;
        }
        if (a.count > max_count) {
            max_count = a.count;
        }

        last_vote  = a.votes;
        last_count = a.count;
    }

    s = db->getArtists(artists, readOptions);
    EXPECT_EQ(Status::OK(), s);
    last_count = 10000;
    last_vote  = 0;
    for (auto& a : artists) {
        EXPECT_LT(last_vote, a.votes);
        EXPECT_GT(last_count, a.count);

        last_vote  = a.votes;
        last_count = a.count;
    }

    // Let's go forward in time!
    readOptions.session_id = 0;
    voteSort.session_id = 0;

    ResultSet<Artist> newArtists;
    s = db->getArtists(newArtists, readOptions);
    EXPECT_EQ(Status::OK(), s);
    EXPECT_LT(max_count, newArtists.begin()->count);

    s = db->getArtists(newArtists, voteSort);
    EXPECT_EQ(Status::OK(), s);
    EXPECT_LT(max_votes, newArtists.begin()->votes);

    ResultSet<Genre> genres;
    max_count = 0;
    max_votes = 0;

    s = db->getGenres(genres, voteSort);
    EXPECT_EQ(Status::OK(), s);
    last_count = 0;
    last_vote  = 10000;
    for (auto& g : genres) {
        EXPECT_GT(last_vote, g.votes);
        EXPECT_LT(last_count, g.count);

        if (g.votes > max_votes) {
            max_votes = g.votes;
        }
        if (g.count > max_count) {
            max_count = g.count;
        }

        last_vote  = g.votes;
        last_count = g.count;
    }

    s = db->getGenres(genres, readOptions);
    EXPECT_EQ(Status::OK(), s);
    last_count = 10000;
    last_vote  = 0;
    for (auto& g : genres) {
        EXPECT_LT(last_vote, g.votes);
        EXPECT_GT(last_count, g.count);

        last_vote  = g.votes;
        last_count = g.count;
    }
    //
    // Let's go backward in time!
    readOptions.session_id = 2;
    voteSort.session_id = 2;

    ResultSet<Genre> oldGenres;
    s = db->getGenres(oldGenres, readOptions);
    EXPECT_EQ(Status::OK(), s);
    EXPECT_GT(max_count, oldGenres.begin()->count);

    s = db->getGenres(oldGenres, voteSort);
    EXPECT_EQ(Status::OK(), s);
    EXPECT_GT(max_votes, oldGenres.begin()->votes);
}
