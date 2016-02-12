#include "util/time.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <stdlib.h>

#include "skrillex/skrillex.hpp"

using namespace std;
using namespace skrillex;
using namespace skrillex::internal;

const string characters = "abcdefghijklmnopqrstuvwxyz1234567890";

void checkStatus(Status status) {
    if (status != Status::OK()) {
        cerr << status.message() << endl;
        exit(1);
    }
}

string randomString(int length, function<char(void)> randChar) {
    string s(length, 0);
    generate_n(s.begin(), length, randChar);
    return s;
}

void benchMapNew(function<char(void)> randChar) {
    DB* raw = 0;
    checkStatus(open(raw, "bench.db", Options::TestOptions()));
    shared_ptr<DB> db(raw);
    shared_ptr<Mapper> mapper(new Mapper(db));

    for (int i = 0; i < 1000; i++) {
        Song song;

        string genreName = randomString(5, randChar);
        string artistName = randomString(10, randChar);
        string songName = randomString(10, randChar);

        auto start = now();
        checkStatus(mapper->map(song, songName, artistName, genreName));
        auto end = now();

        cout << (end - start).count() << endl;
    }
}

void benchMapExisting() {
    DB* raw = 0;
    checkStatus(open(raw, "bench.db", Options::TestOptions()));
    shared_ptr<DB> db(raw);
    shared_ptr<Mapper> mapper(new Mapper(db));

    string genreName = "12345";
    string artistName = "thisis10le";
    string songName = "also10lett";

    for (int i = 0; i < 1000; i++) {
        Song song;

        auto start = now();
        checkStatus(mapper->map(song, songName, artistName, genreName));
        auto end = now();

        cout << (end - start).count() << endl;
    }
}

void benchLookupExisting() {
    DB* raw = 0;
    checkStatus(open(raw, "bench.db", Options::TestOptions()));
    shared_ptr<DB> db(raw);
    shared_ptr<Mapper> mapper(new Mapper(db));

    string genreName = "12345";
    string artistName = "thisis10le";
    string songName = "also10lett";

    Song song;
    checkStatus(mapper->map(song, songName, artistName, genreName));

    for (int i = 0; i < 1000; i++) {
        Song song;
        auto start = now();
        checkStatus(mapper->lookup(song, songName, artistName));
        auto end = now();

        cout << (end - start).count() << endl;
    }
}

void benchLookupNonExisting() {
    DB* raw = 0;
    checkStatus(open(raw, "bench.db", Options::TestOptions()));
    shared_ptr<DB> db(raw);
    shared_ptr<Mapper> mapper(new Mapper(db));

    for (int i = 0; i < 1000; i++) {
        Song song;
        auto start = now();
        Status s = mapper->lookup(song, "1234567890", "098765431");
        if (!s.notFound()) {
            cout << "Expected not found, but was found" << endl;
            exit(1);
        }
        auto end = now();
        cout << (end - start).count() << endl;
    }
}


int main() {
    default_random_engine rng(random_device{}());
    uniform_int_distribution<> dist(0, characters.size() - 1);
    auto randChar = [&dist, &rng]() { return characters[dist(rng)]; };

    // For consistency, all tests use:
    //     * 1000 iterations
    //     * 10 character song names
    //     * 10 character artist names
    //     * 5 character genre names
    benchLookupNonExisting();
}
