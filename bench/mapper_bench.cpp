#include "util/time.hpp"

#include <iostream>
#include <memory>
#include <stdlib.h>

#include "skrillex/skrillex.hpp"

using namespace std;
using namespace skrillex;
using namespace skrillex::internal;

void checkStatus(Status status) {
    if (status != Status::OK()) {
        cerr << status.message() << endl;
        exit(1);
    }
}

int main() {
    DB* raw = 0;
    checkStatus(open(raw, "bench.db", Options::TestOptions()));
    shared_ptr<DB> db(raw);
    shared_ptr<Mapper> mapper(new Mapper(db));

    Song song;
    auto start = now();
    Status s = mapper->map(song, "", "", " Genre");
    auto end = now();

    checkStatus(s);
    cout << "Time: " << (end - start).count() << " ns" << endl;
}
