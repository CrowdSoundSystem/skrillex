#include <iostream>

#include "skrillex/result_set.hpp"
#include "sqlite3/sqlite3.h"
#include "store/sqlite3_store.hpp"
#include "util/time.hpp"
#include "mutator.hpp"

using namespace std;

namespace skrillex {
namespace internal {
    Sqlite3Store::Sqlite3Store()
    : db_(0)
    {
    }

    Sqlite3Store::~Sqlite3Store() {
        if (db_) {
            int ret = sqlite3_close(db_);
            if (ret != SQLITE_OK) {
                cout << "Could not close Sqlite3 database - ";
                cout << sqlite3_errmsg(db_) << endl;
                cout << "Code: " << ret << endl;
            }
        }
    }

    Status Sqlite3Store::open(std::string path, Options options) {
        return bootstrap(path, db_, options.create_if_missing, options.recreate);
    }

    Status Sqlite3Store::getSongs(ResultSet<Song>& set, ReadOptions options) {
        vector<Song>& set_data = ResultSetMutator::getVector<Song>(set);
        set_data.clear();

        sqlite3_stmt* statement = 0;

        // Mirror, mirror, on the wall
        // Who is the ugliest query, of them all
        string query =
            "SELECT Songs.SongID, Songs.Name, Count, Votes, Artists.ArtistID, Artists.Name, Genres.GenreID, Genres.Name FROM Songs "
            "LEFT JOIN SongVotes ON Songs.SongID   = SongVotes.SongID "
            "LEFT JOIN Artists   ON Songs.ArtistID = Artists.ArtistID "
            "LEFT JOIN Genres    ON Songs.GenreID  = Genres.GenreID";

        // What does our query, need to do?
        // Well, we need to
        //     1) Get all Songs, with votes and counts
        //     2) For each song:
        //         a) Get the artist, with votes and count
        //         b) Get the genre, with votes and count
        switch (options.sort) {
            case SortType::Counts:
                query += " ORDER BY Count DESC";
                break;
            case SortType::Votes:
                query += " ORDER BY Votes DESC";
                break;
            default:
                break;
        }

        if (options.result_limit > 0) {
            query += " LIMIT " + to_string(options.result_limit);
        }

        int result = 0;
        sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0);
        while ((result = sqlite3_step(statement)) == SQLITE_ROW) {
            Song s;
            s.id          = sqlite3_column_int(statement, 0);
            s.name        = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
            s.count       = sqlite3_column_int(statement, 2);
            s.votes       = sqlite3_column_int(statement, 3);
            s.last_played = 0;

            s.artist.id   = sqlite3_column_int(statement, 4);
            s.artist.name = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 5)));

            s.genre.id    = sqlite3_column_int(statement, 6);
            s.genre.name  = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 7)));

            set_data.push_back(s);
        }

        sqlite3_finalize(statement);

        if (result != SQLITE_OK && result != SQLITE_DONE) {
            return Status::Error(sqlite3_errmsg(db_));
        }

		return Status::OK();
	}

    Status Sqlite3Store::getArtists(ResultSet<Artist>& set, ReadOptions options) {
        vector<Artist>& set_data = ResultSetMutator::getVector<Artist>(set);
        set_data.clear();

        sqlite3_stmt* statement = 0;

        string query = "SELECT Artists.ArtistID, Name, Votes, Count FROM Artists "
            "LEFT JOIN ArtistVotes on Artists.ArtistID = ArtistVotes.ArtistID";

        switch (options.sort) {
            case SortType::Counts:
                query += " ORDER BY Count DESC";
                break;
            case SortType::Votes:
                query += " ORDER BY Votes DESC";
                break;
            default:
                break;
        }

        if (options.result_limit > 0) {
            query += " LIMIT " + to_string(options.result_limit);
        }

        int result = 0;
        sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0);
        while ((result = sqlite3_step(statement)) == SQLITE_ROW) {
            Artist a;
            a.id    = sqlite3_column_int(statement, 0);
            a.name  = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
            a.votes = sqlite3_column_int(statement, 2);
            a.count = sqlite3_column_int(statement, 3);
            set_data.push_back(a);
        }

        sqlite3_finalize(statement);

        if (result != SQLITE_OK && result != SQLITE_DONE) {
            return Status::Error(sqlite3_errmsg(db_));
        }

		return Status::OK();
	}
    Status Sqlite3Store::getGenres(ResultSet<Genre>& set, ReadOptions options) {
        vector<Genre>& set_data = ResultSetMutator::getVector<Genre>(set);
        set_data.clear();

        sqlite3_stmt* statement = 0;

        string query = "SELECT Genres.GenreID, Name, Votes, Count FROM Genres "
            "LEFT JOIN GenreVotes on Genres.GenreID = GenreVotes.GenreID";

        switch (options.sort) {
            case SortType::Counts:
                query += " ORDER BY Count";
                break;
            case SortType::Votes:
                query += " ORDER BY Votes";
                break;
            default:
                break;
        }

        if (options.result_limit > 0) {
            query += " LIMIT " + to_string(options.result_limit);
        }

        int result = 0;
        sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0);
        while ((result = sqlite3_step(statement)) == SQLITE_ROW) {
            Genre g;
            g.id    = sqlite3_column_int(statement, 0);
            g.name  = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
            g.votes = sqlite3_column_int(statement, 2);
            g.count = sqlite3_column_int(statement, 3);
            set_data.push_back(g);
        }

        sqlite3_finalize(statement);

        if (result != SQLITE_OK && result != SQLITE_DONE) {
            return Status::Error(sqlite3_errmsg(db_));
        }

		return Status::OK();
	}

    Status Sqlite3Store::getPlayHistory(ResultSet<Song>& set, ReadOptions options) {
        vector<Song>& set_data =  ResultSetMutator::getVector<Song>(set);
        set_data.clear();

        sqlite3_stmt* statement = 0;

        string query =
            "SELECT Songs.SongID, Songs.Name, Date, Artists.ArtistID, Artists.Name, Genres.GenreID, Genres.NAME "
            "LEFT JOIN Artists ON Songs.ArtistID = Artists.ArtistID "
            "LEFT JOIN Genres  ON Songs.GenreID  = Genres.GenreID "
            "JOIN PlayHistory  ON Songs.SongID   = PlayHistory.SongID "
            "ORDER BY DESC Date";

        if (options.result_limit > 0) {
            query += " LIMIT " + to_string(options.result_limit);
        }

        int result = 0;
        sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0);
        while ((result = sqlite3_step(statement)) == SQLITE_ROW) {
            Song s;

            s.id          = sqlite3_column_int(statement, 0);
            s.name        = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
            s.count       = sqlite3_column_int(statement, 2);
            s.votes       = sqlite3_column_int(statement, 3);
            s.last_played = sqlite3_column_int64(statement, 4);

            s.artist.id   = sqlite3_column_int(statement, 5);
            s.artist.name = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 6)));

            s.genre.id    = sqlite3_column_int(statement, 7);
            s.genre.name  = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 8)));

            set_data.push_back(s);
        }

        sqlite3_finalize(statement);

        if (result != SQLITE_OK && result != SQLITE_DONE) {
            return Status::Error(sqlite3_errmsg(db_));
        }

		return Status::OK();
	}

    Status Sqlite3Store::getQueue(ResultSet<Song>& set) {
        // For now, there's not a huge motivation to put the
        // queue into the db, since it's reset when the program
        // end anyway.
        vector<Song>& set_data = ResultSetMutator::getVector(set);
        set_data.clear();

        lock_guard<mutex> lock(queue_lock_);
        copy(song_queue_.begin(), song_queue_.end(), back_inserter(set_data));

		return Status::OK();
	}
    Status Sqlite3Store::queueSong(int songId) {
        // Get song from the database, and insert into queue.
        sqlite3_stmt* statement = 0;

        string query =
            "SELECT Songs.SongID, Songs.Name, Count, Votes, Artists.ArtistID, Artists.Name, Genres.GenreID, Genres.Name FROM Songs "
            "LEFT JOIN SongVotes ON Songs.SongID   = SongVotes.SongID "
            "LEFT JOIN Artists   ON Songs.ArtistID = Artists.ArtistID "
            "LEFT JOIN Genres    ON Songs.GenreID  = Genres.GenreID "
            "WHERE Songs.SongID = " + to_string(songId);

        int result = 0;
        Song s;
        s.id = -1;

        sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0);
        while ((result = sqlite3_step(statement)) == SQLITE_ROW) {
            s.id          = sqlite3_column_int(statement, 0);
            s.name        = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
            s.count       = sqlite3_column_int(statement, 2);
            s.votes       = sqlite3_column_int(statement, 3);
            s.last_played = 0;

            s.artist.id   = sqlite3_column_int(statement, 4);
            s.artist.name = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 5)));

            s.genre.id    = sqlite3_column_int(statement, 6);
            s.genre.name  = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 7)));
        }

        sqlite3_finalize(statement);

        if (result != SQLITE_OK && result != SQLITE_DONE) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (s.id == -1) {
            return Status::NotFound("Could not queue song");
        }

        lock_guard<mutex> lock(queue_lock_);
        song_queue_.push_back(s);

		return Status::OK();
	}
    Status Sqlite3Store::songFinished() {
        if (song_queue_.empty()) {
            return Status::Error("Queue empty");
        }

        Song song;

        {
            lock_guard<mutex> lock(queue_lock_);
            Song song = song_queue_.front();
            song_queue_.erase(song_queue_.begin());
        }

        song.last_played = timestamp();

        // Interesting Question: Can we just save updated song?
        //
        // No! The song_queue_ acts purely as a cache. When a
        // song in inserted to song_queue_, it's data is pulled
        // from the database, and inserted. The data is never
        // touched again! What this means is that at the time
        // of popping off the queue, the data may be in an
        // invalid state, so we must update the item before
        // saving!

        // TODO: Read from database
        // TODO: Store in database
		return Status::OK();
	}

    Status Sqlite3Store::addSong(Song& song) {
        sqlite3_stmt* statement = 0;

        string query = "INSERT INTO `Songs` (`ArtistID`, `GenreID`, `Name`) VALUES (?, ?, ?)";

        if (sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int(statement, 1, song.artist.id)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int(statement, 2, song.genre.id)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_text(statement, 3, song.name.c_str(), song.name.size(), SQLITE_STATIC)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        int r = sqlite3_step(statement);
        sqlite3_finalize(statement);

        if (r != SQLITE_OK && r != SQLITE_DONE) {
            return Status::Error(sqlite3_errmsg(db_));
        }

		return Status::OK();
	}
    Status Sqlite3Store::addArtist(Artist& artist) {
        sqlite3_stmt* statement = 0;

        string query = "INSERT INTO `Artists` (`Name`) VALUES (?)";

        if (sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_text(statement, 1, artist.name.c_str(), artist.name.size(), SQLITE_STATIC)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        int r = sqlite3_step(statement);
        sqlite3_finalize(statement);

        if (r != SQLITE_OK && r != SQLITE_DONE) {
            return Status::Error(sqlite3_errmsg(db_));
        }

		return Status::OK();
	}
    Status Sqlite3Store::addGenre(Genre& genre) {
		sqlite3_stmt* statement = 0;

        string query = "INSERT INTO `Genres` (`Name`) VALUES (?)";

        if (sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_text(statement, 1, genre.name.c_str(), genre.name.size(), SQLITE_STATIC)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        int r = sqlite3_step(statement);
        sqlite3_finalize(statement);

        if (r != SQLITE_OK && r != SQLITE_DONE) {
            return Status::Error(sqlite3_errmsg(db_));
        }

		return Status::OK();
	}

    Status Sqlite3Store::countSong(Song& song, int amount, WriteOptions options) {
		return Status::OK();
	}
    Status Sqlite3Store::countArtist(Artist& artist, int amount, WriteOptions options) {
		return Status::OK();
	}
    Status Sqlite3Store::countGenre(Genre& genre, int amount, WriteOptions options) {
		return Status::OK();
	}

    Status Sqlite3Store::voteSong(Song& song, int amount, WriteOptions options) {
		return Status::OK();
	}
    Status Sqlite3Store::voteArtist(Artist& artist, int amount, WriteOptions options) {
		return Status::OK();
	}
    Status Sqlite3Store::voteGenre(Genre& genre, int amount, WriteOptions options) {
		return Status::OK();
	}

    Status Sqlite3Store::createSession() {
		return Status::OK();
	}
    Status Sqlite3Store::createSession(int& result) {
		return Status::OK();
	}

    Status Sqlite3Store::getSession(int& result) {
        result = session_id_;
		return Status::OK();
	}
    Status Sqlite3Store::getSessionCount(int& result) {
		return Status::OK();
	}

}
}

