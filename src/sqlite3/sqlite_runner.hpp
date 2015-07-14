#ifndef sqlite_runner_hpp
#define sqlite_runner_hpp

#include <chrono>
#include <iostream>
#include <string>

#include "../query.hpp"
#include "../stats.hpp"
#include "sqlite3.h"

void disable_sync(sqlite3* db) {
    sqlite3_stmt* statement;
    if (sqlite3_prepare_v2(db, "pragma synchronous = off", -1, &statement, 0)) {
        std::cout << "Could not disable sync" << std::endl;
        return;
    }

    while (sqlite3_step(statement) == SQLITE_ROW) {
    }
}

Stats sqlite_run(const SQLQuery& query, int iterations) {
    Stats stats;

    using namespace std;
    typedef std::chrono::high_resolution_clock Clock;

    sqlite3* db;

    if (sqlite3_open("bench.db", &db)) {
        cout << "Could not open database: " << sqlite3_errmsg(db) << endl;
        return stats;
    }

    // Turn off Synch for better speeds.
    disable_sync(db);

    for (int i = 0; i < iterations; i++) {
        int row_count = 0;
        sqlite3_stmt* statement = 0;

        auto start = Clock::now();

        int result = 0;
        sqlite3_prepare_v2(db, query.getSqliteQuery().c_str(), -1, &statement, 0);

        if (query.getType() == QueryType::Update) {
            result = sqlite3_step(statement);
        } else {
            while ((result = sqlite3_step(statement)) == SQLITE_ROW) {
                row_count++;
            }
        }

        if (result != SQLITE_OK && result != SQLITE_DONE) {
            cout << "Result: " << result << endl;
            cout << "Error Code: " << sqlite3_errmsg(db) << endl;
        }

        auto end   = Clock::now();

        stats.add_time(chrono::duration<double, milli>(end - start).count());
        stats.set_rows_processed(row_count);

        sqlite3_finalize(statement);
    }

    return stats;
}

#endif
