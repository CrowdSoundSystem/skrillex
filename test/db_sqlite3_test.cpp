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
#include <thread>
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
    EXPECT_EQ(Status::OK(), populate_full(raw, NUM_SONGS, NUM_ARTISTS, NUM_GENRES, 3));

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
    EXPECT_EQ(Status::OK(), db->getSongs(songs, voteSort));
    EXPECT_EQ(NUM_SONGS, songs.size());
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


TEST(Sqlite3DatabaseTests, Activity) {
    DB* raw = 0;
    Status s = open(raw, "test.db", Options::TestOptions());
    ASSERT_EQ(Status::OK(), s);

    shared_ptr<DB> db(raw);

    ResultSet<Genre> genres;
    EXPECT_EQ(Status::OK(), db->getGenres(genres));
    EXPECT_EQ(0, genres.size());

    ResultSet<Artist> artists;
    EXPECT_EQ(Status::OK(), db->getArtists(artists));
    EXPECT_EQ(0, artists.size());

    ResultSet<Song> songs;
    EXPECT_EQ(Status::OK(), db->getSongs(songs));
    EXPECT_EQ(0, songs.size());

    // Insert a song, then have a user vote on it.
    Genre g;
    g.name = "g0";

    Artist a;
    a.name = "a0";

    EXPECT_EQ(Status::OK(), db->addGenre(g));
    EXPECT_EQ(Status::OK(), db->addArtist(a));

    Song song;
    song.artist = a;
    song.genre = g;
    song.name = "s0";

    EXPECT_EQ(Status::OK(), db->addSong(song));

    for (int i = 0; i < 3; i++) {
        int vote = (i == 2) ? 0 : 1;
        EXPECT_EQ(Status::OK(), db->voteGenre("u" + to_string(i), g, vote));
        EXPECT_EQ(Status::OK(), db->voteArtist("u" + to_string(i), a, vote));
        EXPECT_EQ(Status::OK(), db->voteSong("u" + to_string(i), song, vote));
    }

    EXPECT_EQ(Status::OK(), db->getGenres(genres));
    EXPECT_EQ(1, genres.size());
    for (auto& g : genres) {
        EXPECT_EQ(3, g.count);
        EXPECT_EQ(2, g.votes);
    }

    EXPECT_EQ(Status::OK(), db->getArtists(artists));
    EXPECT_EQ(1, artists.size());
    for (auto& a : artists) {
        EXPECT_EQ(3, a.count);
        EXPECT_EQ(2, a.votes);
    }

    EXPECT_EQ(Status::OK(), db->getSongs(songs));
    EXPECT_EQ(1, songs.size());
    for (auto& s : songs) {
        EXPECT_EQ(3, s.count);
        EXPECT_EQ(2, s.votes);
    }

    this_thread::sleep_for(chrono::milliseconds(100));

    ReadOptions options;
    options.inactivity_threshold = 20;

    EXPECT_EQ(Status::OK(), db->getGenres(genres, options));
    EXPECT_EQ(1, genres.size());
    for (auto& g : genres) {
        EXPECT_EQ(0, g.count);
        EXPECT_EQ(0, g.votes);
    }

    EXPECT_EQ(Status::OK(), db->getArtists(artists, options));
    EXPECT_EQ(1, artists.size());
    for (auto& a : artists) {
        EXPECT_EQ(0, a.count);
        EXPECT_EQ(0, a.votes);
    }

    EXPECT_EQ(Status::OK(), db->getSongs(songs, options));
    EXPECT_EQ(1, songs.size());
    for (auto& s : songs) {
        EXPECT_EQ(0, s.count);
        EXPECT_EQ(0, s.votes);
    }
}

TEST(Sqlite3DatabaseTests, QueueBuffer) {
    DB* raw = 0;
    Status s = open(raw, "test.db", Options::TestOptions());
    ASSERT_EQ(Status::OK(), s);

    shared_ptr<DB> db(raw);
    Store* store = StoreMutator::getStore(raw);

    PopulatorData data = get_populator_data(10, 3, 3);
    ASSERT_EQ(Status::OK(), populate_empty(raw, 10, 3, 3));

    // Queue all of the songs
    for (int i = 0; i < data.songs.size(); i++) {
        EXPECT_EQ(Status::OK(), db->queueSong(data.songs[i].id));

        ResultSet<Song> queue;
        EXPECT_EQ(Status::OK(), db->getQueue(queue));
        EXPECT_EQ(i + 1, queue.size());
    }

    // Verify the final queue
    int id = 0;
    ResultSet<Song> queue;
    EXPECT_EQ(Status::OK(), db->getQueue(queue));
    for (auto it = queue.begin(); it != queue.end(); it++,id++) {
        EXPECT_EQ(data.songs[id], *it);
    }

    // Test buffer
    ResultSet<Song> buffer;
    EXPECT_EQ(Status::OK(), db->getBuffer(buffer));
    EXPECT_EQ(Status::OK(), db->getQueue(queue));
    for (int i = 0; i < 10; i++) {
        int originalBufferSize = buffer.size();
        int originalQueueSize = queue.size();

        // Move from queue into buffer
        EXPECT_EQ(Status::OK(), db->bufferNext());

        // Make sure song moved over
        EXPECT_EQ(Status::OK(), db->getBuffer(buffer));
        EXPECT_EQ(Status::OK(), db->getQueue(queue));
        EXPECT_EQ(originalQueueSize - 1, queue.size());
        EXPECT_EQ(originalBufferSize + 1, buffer.size());
    }

    // Test bufferNext() with empty queue
    s = db->bufferNext();
    EXPECT_TRUE(s.error());
    EXPECT_EQ("Queue empty", s.message());

    // Test songFinished
    for (int i = 0; i < 10; i++) {
        Song head = *buffer.begin();
        EXPECT_EQ(0, head.last_played);

        int originalBufferSize = buffer.size();

        EXPECT_EQ(Status::OK(), db->songFinished());
        EXPECT_EQ(Status::OK(), db->getBuffer(buffer));
        EXPECT_EQ(originalBufferSize - 1, buffer.size());

        // Make sure last played was updated
        Song lastPlayed;
        EXPECT_EQ(Status::OK(), store->getSongFromId(lastPlayed, head.id));
        EXPECT_LT(0, lastPlayed.last_played);
    }

    // Test that unplayable songs do not get added to queue.
    EXPECT_EQ(Status::OK(), db->markUnplayable(data.songs[0].id));
    EXPECT_EQ(Status::OK(), db->queueSong(data.songs[0].id));
    EXPECT_EQ(Status::OK(), db->getQueue(queue));
    EXPECT_EQ(0, queue.size());
}

TEST(Sqlite3DatabaseTests, SetQueue) {
    DB* raw = 0;
    Status s = open(raw, "test.db", Options::TestOptions());
    ASSERT_EQ(Status::OK(), s);

    shared_ptr<DB> db(raw);
    Store* store = StoreMutator::getStore(raw);

    PopulatorData data = get_populator_data(10, 3, 3);
    ASSERT_EQ(Status::OK(), populate_empty(raw, 10, 3, 3));

    // Queue all of the songs
    for (int i = 0; i < data.songs.size(); i++) {
        EXPECT_EQ(Status::OK(), db->queueSong(data.songs[i].id));

        ResultSet<Song> queue;
        EXPECT_EQ(Status::OK(), db->getQueue(queue));
        EXPECT_EQ(i + 1, queue.size());
    }

    // Verify the final queue
    int id = 0;
    ResultSet<Song> queue;
    EXPECT_EQ(Status::OK(), db->getQueue(queue));
    for (auto it = queue.begin(); it != queue.end(); it++,id++) {
        EXPECT_EQ(data.songs[id], *it);
    }

    // Ensure that calling setQueue() replaces the current queue entirely.
    vector<int> songIds;
    for (int i = data.songs.size() - 1; i >= 0; i--) {
        songIds.push_back(data.songs[i].id);
    }

    EXPECT_EQ(Status::OK(), db->setQueue(songIds));
    EXPECT_EQ(Status::OK(), db->getQueue(queue));

    id = data.songs.size() - 1;
    for (auto it = queue.begin(); it != queue.end(); it++,id--) {
        EXPECT_EQ(data.songs[id], *it);
    }
}

TEST(Sqlite3DatabaseTests, Normalized) {
    DB* raw = 0;
    Status s = open(raw, "test.db", Options::TestOptions());
    ASSERT_EQ(Status::OK(), s);

    Store* store = StoreMutator::getStore(raw);

    // We merely want to test whether or not IO for normalized
    // lookups work. We don't care about how normalization works.
    // As a result, we'll just make up our own normalization for
    // test purposes.
    Genre g;
    g.name = "Genre";

    Artist a;
    a.name = "Artist";

    EXPECT_EQ(Status::OK(), store->addGenre(g));
    EXPECT_EQ(Status::OK(), store->addArtist(a));

    Song song;
    song.name = "Song";
    song.artist.id = a.id;
    song.genre.id = g.id;

    EXPECT_EQ(Status::OK(), store->addSong(song));

    EXPECT_EQ(Status::OK(), store->insertNormalized("genre", 0, 0, g.id));
    EXPECT_EQ(Status::OK(), store->insertNormalized("artist", 0, a.id, 0));
    EXPECT_EQ(Status::OK(), store->insertNormalized("song", song.id, 0, 0));
    EXPECT_EQ(Status::OK(), store->insertNormalized("complete", song.id, song.artist.id, song.genre.id));

    Song result;
    EXPECT_EQ(Status::OK(), store->getNormalized(result, "genre"));
    EXPECT_EQ(0, result.id);
    EXPECT_EQ(0, result.artist.id);
    EXPECT_EQ(g.id, result.genre.id);

    result = Song();
    EXPECT_EQ(Status::OK(), store->getNormalized(result, "artist"));
    EXPECT_EQ(0, result.id);
    EXPECT_EQ(a.id, result.artist.id);
    EXPECT_EQ(0, result.genre.id);

    result = Song();
    EXPECT_EQ(Status::OK(), store->getNormalized(result, "song"));
    EXPECT_EQ(song.id, result.id);
    EXPECT_EQ(0, result.artist.id);
    EXPECT_EQ(0, result.genre.id);

    result = Song();
    EXPECT_EQ(Status::OK(), store->getNormalized(result, "complete"));
    EXPECT_EQ(song.id, result.id);
    EXPECT_EQ(song.name, result.name);
    EXPECT_EQ(a.id, result.artist.id);
    EXPECT_EQ(a.name, result.artist.name);
    EXPECT_EQ(g.id, result.genre.id);
    EXPECT_EQ(g.name, result.genre.name);
}

