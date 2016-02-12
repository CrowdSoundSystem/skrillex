#include "util/time.hpp"

#include <iostream>
#include <memory>
#include <stdlib.h>

#include "skrillex/skrillex.hpp"
#include "skrillex/testing/populator.hpp"

using namespace std;
using namespace skrillex;
using namespace skrillex::internal;
using namespace skrillex::testing;

void checkStatus(Status status) {
    if (status != Status::OK()) {
        cerr << status.message() << endl;
        exit(1);
    }
}

void benchGetSongs() {
    DB* raw = 0;
    checkStatus(open(raw, "bench.db", Options::Options()));
    shared_ptr<DB> db(raw);

    // Populate with a bunch of songs
    ReadOptions options;
    options.result_limit = 1000;

    ResultSet<Song> songs;
    for (int i = 0; i < 1000; i++) {
        auto start = now();
        Status s = db->getSongs(songs, options);
        if (s != Status::OK()) {
            cout << s.message() << endl;
            exit(1);
        }
        auto end = now();

        cout << (end - start).count() << endl;
    }
}

void benchAddSong() {
    DB* raw = 0;
    checkStatus(open(raw, "bench_mut.db", Options::TestOptions()));
    shared_ptr<DB> db(raw);

    Genre g;
    g.name = "hello";

    Artist a;
    a.name = "is it me";

    Song song;
    song.name = "you're looking for!";
    song.artist = a;
    song.genre = g;

    for (int i = 0; i < 1000; i++) {
        auto start = now();
        Status s = db->addGenre(song.genre);
        if (s != Status::OK()) {
            cout << s.message() << endl;
            exit(1);
        }
        s = db->addArtist(song.artist);
        if (s != Status::OK()) {
            cout << s.message() << endl;
            exit(1);
        }
        s = db->addSong(song);
        if (s != Status::OK()) {
            cout << s.message() << endl;
            exit(1);
        }
        auto end = now();
        cout << (end - start).count() << endl;
    }
}

void benchVoteSong() {
    DB* raw = 0;
    checkStatus(open(raw, "bench_mut.db", Options::TestOptions()));
    shared_ptr<DB> db(raw);

    Genre g;
    g.name = "hello";

    Status s = db->addGenre(g);
    if (s != Status::OK()) {
        cout << s.message() << endl;
        exit(0);
    }

    Artist a;
    s = db->addArtist(a);
    if (s != Status::OK()) {
        cout << s.message() << endl;
        exit(0);
    }
    a.name = "is it me";

    Song song;
    song.name = "you're looking for!";
    song.artist = a;
    song.genre = g;

    s = db->addSong(song);
    if (s != Status::OK()) {
        cout << s.message() << endl;
        exit(0);
    }

    string user = "this is a user";
    for (int i = 0; i < 1000; i++) {
        auto start = now();

        s = db->voteGenre(user, song.genre, 1);
        if (s != Status::OK()) {
            cout << s.message() << endl;
            exit(0);
        }

        s = db->voteArtist(user, song.artist, 1);
        if (s != Status::OK()) {
            cout << s.message() << endl;
            exit(0);
        }

        s = db->voteSong(user, song, 1);
        if (s != Status::OK()) {
            cout << s.message() << endl;
            exit(0);
        }

        auto end = now();

        cout << (end - start).count() << endl;
    }
}

int main() {
    // Generally I just call one of the above functions, and then
    // pipe the results into my histogram tool to get some sweet
    // stats. Example: bin/db_bench | histogram = profit
}
