#include <iostream>
#include <string>
#include <vector>
#include <sys/stat.h>

#include "store/sqlite3_bootstrap.hpp"
#include "sqlite3/sqlite3.h"

using namespace std;

namespace skrillex {
namespace internal {
    const vector<string> DROP_TABLES = {
        "DROP TABLE IF EXISTS PlayHistory",
        "DROP TABLE IF EXISTS SongVotes",
        "DROP TABLE IF EXISTS ArtistVotes",
        "DROP TABLE IF EXISTS GenreVotes",

        "DROP TABLE IF EXISTS SessionHistory",
        "DROP TABLE IF EXISTS UserActivity",

        "DROP TABLE IF EXISTS Normalized",

        "DROP TABLE IF EXISTS Songs",
        "DROP TABLE IF EXISTS Artists",
        "DROP TABLE IF EXISTS Genres"
    };

    const vector<string> CREATE_TABLES = {
        "CREATE TABLE IF NOT EXISTS Artists ("
        "    ArtistID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    Name     VARCHAR(255) NOT NULL"
        ")",

        "CREATE TABLE IF NOT EXISTS Genres ("
        "    GenreID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    Name    VARCHAR(255)  NOT NULL"
        ")",

        "CREATE TABLE IF NOT EXISTS Songs ("
        "    SongID   INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    ArtistID INT,"
        "    GenreID  INT,"
        "    Name     VARCHAR(255) NOT NULL,"
        "    FOREIGN KEY(ArtistID) REFERENCES Artists(ArtistID),"
        "    FOREIGN KEY(GenreID)  REFERENCES Genres(GenreID)"
        ")",

        "CREATE TABLE IF NOT EXISTS Normalized ("
        "    Normalized VARCHAR(255) NOT NULL,"
        "    SongID     INT,"
        "    ArtistID   INT,"
        "    GenreID    INT,"
        "    PRIMARY KEY(Normalized),"
        "    FOREIGN KEY(SongID)    REFERENCES Songs(SongID),"
        "    FOREIGN KEY(ArtistID)  REFERENCES Artists(ArtistID),"
        "    FOREIGN KEY(GenreID)   REFERENCES Genres(GenreID)"
        ")",

        "CREATE TABLE IF NOT EXISTS SessionHistory ("
        "    SessionID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    Date      DATETIME NOT NULL"
        ")",

        "CREATE TABLE IF NOT EXISTS UserActivity ("
        "    UserID     VARCHAR(255) NOT NULL PRIMARY KEY,"
        "    LastActive DATETIME NOT NULL"
        ")",

        "CREATE TABLE IF NOT EXISTS ArtistVotes ("
        "    ArtistID  INT NOT NULL,"
        "    SessionID INT NOT NULL,"
        "    UserID    VARCHAR(255) NOT NULL,"
        "    Vote      INT NOT NULL DEFAULT 0,"
        "    PRIMARY KEY(ArtistID, SessionID, UserID),"
        "    FOREIGN KEY(ArtistID)  REFERENCES Artists(ArtistID),"
        "    FOREIGN KEY(SessionID) REFERENCES SessionHistory(SessionID),"
        "    FOREIGN KEY(UserID)    REFERENCES UserActivity(UserID)"
        ")",

        "CREATE TABLE IF NOT EXISTS GenreVotes ("
        "    GenreID   INT NOT NULL,"
        "    SessionID INT NOT NULL,"
        "    UserID    VARCHAR(255) NOT NULL,"
        "    Vote      INT NOT NULL DEFAULT 0,"
        "    PRIMARY KEY(GenreID, SessionID, UserID),"
        "    FOREIGN KEY(GenreID)   REFERENCES Genres(GenreID),"
        "    FOREIGN KEY(SessionID) REFERENCES SessionHistory(SessionID),"
        "    FOREIGN KEY(UserID)    REFERENCES UserActivity(UserID)"
        ")",

        "CREATE TABLE IF NOT EXISTS SongVotes ("
        "    SongID    INT NOT NULL,"
        "    SessionID INT NOT NULL,"
        "    UserID    VARCHAR(255) NOT NULL,"
        "    Vote      INT NOT NULL DEFAULT 0,"
        "    PRIMARY KEY(SongID, SessionID, UserID),"
        "    FOREIGN KEY(SongID)    REFERENCES Songs(SongID),"
        "    FOREIGN KEY(SessionID) REFERENCES SessionHistory(SessionID),"
        "    FOREIGN KEY(UserID)    REFERENCES UserActivity(UserID)"
        ")",

        "CREATE TABLE IF NOT EXISTS PlayHistory ("
        "    SongID    INT NOT NULL,"
        "    SessionID INT NOT NULL,"
        "    Timestamp BIGINT NOT NULL,"
        "    PRIMARY KEY(SongID, SessionID)"
        ")"
    };

    bool exists(const std::string& name) {
        struct stat buffer;
        return (stat(name.c_str(), &buffer) == 0);
    }

    Status disable_sync(sqlite3* db) {
        sqlite3_stmt* statement = 0;
        if (sqlite3_prepare_v2(db, "pragma synchronous = off", -1, &statement, 0)) {
            return Status::Error(sqlite3_errmsg(db));
        }

        while (sqlite3_step(statement) == SQLITE_ROW) {
        }

        sqlite3_finalize(statement);
        return Status::OK();
    }

    Status bootstrap(const string& path, sqlite3*& db, bool create_if_missing, bool recreate) {
        if (db) {
            return Status::Error("Attempting to bootstrap a non-null DB");
        }

        // Check if it exists
        if (!exists(path) && !create_if_missing) {
            return Status::Error("Database does not exist.");
        }

        int r = sqlite3_open(path.c_str(), &db);
        if (r) {
            return Status::Error(sqlite3_errmsg(db));
        }

        // Disable sync as it greatly imrpoves performance
        // Note: This is only acceptable if we are okay with the possibility
        // of losing data, especially on a crash.
        Status s = disable_sync(db);
        if (s) {
            cout << "WARNING: Could not disable sync: " << s.message() << endl;
        }

        // If we're overwriting the existing database, then
        // drop all the tables, so we can recreate.
        if (recreate) {
            for (auto& query : DROP_TABLES) {
                sqlite3_stmt* statement = 0;

                r = sqlite3_prepare_v2(db, query.c_str(), -1, &statement, 0);
                while ((r = sqlite3_step(statement)) == SQLITE_ROW) {
                }

                if (r != SQLITE_DONE) {
                    return Status::Error(sqlite3_errmsg(db));
                }

                sqlite3_finalize(statement);
            }
        }

        // Create tables, if necessary
        for (auto& query : CREATE_TABLES) {
            sqlite3_stmt* statement = 0;

            r = sqlite3_prepare_v2(db, query.c_str(), -1, &statement, 0);
            while ((r = sqlite3_step(statement)) == SQLITE_ROW) {
            }

            if (r != SQLITE_DONE) {
                return Status::Error(sqlite3_errmsg(db));
            }

            sqlite3_finalize(statement);
        }

        return Status::OK();
    }
}
}

