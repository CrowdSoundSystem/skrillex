#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "skrillex/db.hpp"
#include "skrillex/dbo.hpp"

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

TEST(InMemoryDatabaseTests, Insert) {
    DB* raw = 0;
    Status s = open(raw, "", Options::InMemoryOptions());
    ASSERT_EQ(Status::OK(), s);

    shared_ptr<DB> db(raw);

    vector<Genre> genres;
    vector<Artist> artists;
    vector<Song> songs;

    for (int i = 0; i < 10; i++) {
        Genre g;
        g.id   = i;
        g.name = "g" + to_string(i);
        g.votes = i;
        g.count = 10 - i;

        Artist a;
        a.id = i;
        a.name = "a" + to_string(i);
        a.votes = 10 - i;
        g.count = i;

        cout << "Adding genre" << endl;
        EXPECT_EQ(Status::OK(), db->addGenre(g));
        cout << "Adding genre complete" << endl;

        EXPECT_EQ(i + 1, g.id);
        EXPECT_EQ("g" + to_string(i), g.name);
        EXPECT_EQ(i, g.votes);
        EXPECT_EQ(10 - i, g.count);

        EXPECT_EQ(Status::OK(), db->addArtist(a));
        EXPECT_EQ(i + 1, a.id);
        EXPECT_EQ("a" + to_string(i), a.name);
        EXPECT_EQ(10 - i, a.votes);
        EXPECT_EQ(i, a.count);

        genres.push_back(g);
        artists.push_back(a);
    }

    for (int i = 1; i <= 100; i++) {
        Song s;
        s.id     = i;
        s.genre  = genres[i % 10];
        s.artist = artists[i % 10];
        s.name   = "s" + to_string(i);

        EXPECT_EQ(Status::OK(), db->addSong(s));
        EXPECT_EQ(i + 1, s.id);
        EXPECT_EQ(i % 10, s.genre.id);
        EXPECT_EQ(i % 10, s.artist.id);

        songs.push_back(s);
    }
}
