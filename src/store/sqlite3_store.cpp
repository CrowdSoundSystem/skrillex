#include <iostream>
#include <set>

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
            "SELECT Songs.SongID, Songs.Name, COUNT(SongVotes.SongID) as Count, COALESCE(SUM(SongVotes.Vote), 0) as Votes, PlayHistory.Timestamp, Artists.ArtistID, Artists.Name, Genres.GenreID, Genres.Name, SongVotes.SessionID FROM Songs "
            "LEFT JOIN SongVotes ON Songs.SongID = SongVotes.SongID AND SongVotes.UserID IN ("
            "    SELECT UserID FROM UserActivity"
            "    WHERE UserActivity.LastActive > ?"
            ") ";

        // TODO: Proper semantics for last played.
        if (options.session_id != -1) {
            query += "AND SongVotes.SessionID = ? ";
        } else {
            query += "AND SongVotes.SessionID != ? ";
        }

        query +=
            "LEFT JOIN Artists     ON Songs.ArtistID = Artists.ArtistID "
            "LEFT JOIN Genres      ON Songs.GenreID  = Genres.GenreID "
            "LEFT JOIN PlayHistory ON Songs.SongID   = PlayHistory.SongID AND PlayHistory.SessionID = ? "
            "GROUP BY Songs.SongID ";

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

        if (sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int64(statement, 1, timestamp() - options.inactivity_threshold)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (options.session_id <= 0) {
            options.session_id = session_id_;
        }

        if (sqlite3_bind_int64(statement, 2, options.session_id)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int64(statement, 3, options.session_id)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        std::set<int> song_buffer_ids;
        if (options.filter_buffered) {
            lock_guard<mutex> lock(buffer_lock_);
            auto it = song_buffer_ids.begin();
            copy(song_buffer_ids_.begin(), song_buffer_ids_.end(), inserter(song_buffer_ids, it));
        }

        int result = 0;
        bool completed = false;
        while ((result = sqlite3_step(statement)) == SQLITE_ROW) {
            Song s;
            s.id          = sqlite3_column_int(statement, 0);

            // If the returned ID is zero, then there are actually zero results. However,
            // since we use COUNT(), a row will still be returned, so we must perform this check.
            if (s.id == 0) {
                completed = true;
                break;
            }

            if (options.filter_buffered) {
                if (song_buffer_ids.find(s.id) != song_buffer_ids.end()) {
                    continue;
                }
            }

            s.name        = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
            s.count       = sqlite3_column_int(statement, 2);
            s.votes       = sqlite3_column_int(statement, 3);
            s.last_played = sqlite3_column_int64(statement, 4);

            s.artist.id   = sqlite3_column_int(statement, 5);
            if (s.artist.id > 0) {
                s.artist.name = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 6)));
            }

            s.genre.id    = sqlite3_column_int(statement, 7);
            if (s.genre.id > 0) {
                s.genre.name  = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 8)));
            }

            set_data.push_back(s);
        }

        sqlite3_finalize(statement);

        if (result != SQLITE_OK && result != SQLITE_DONE && !completed) {
            return Status::Error(sqlite3_errmsg(db_));
        }

		return Status::OK();
	}

    Status Sqlite3Store::getArtists(ResultSet<Artist>& set, ReadOptions options) {
        vector<Artist>& set_data = ResultSetMutator::getVector<Artist>(set);
        set_data.clear();

        sqlite3_stmt* statement = 0;

        string query =
            "SELECT Artists.ArtistID, Name, COUNT(ArtistVotes.ArtistID) as Count, COALESCE(SUM(ArtistVotes.Vote), 0) as Votes FROM Artists "
            "LEFT JOIN ArtistVotes on Artists.ArtistID = ArtistVotes.ArtistID AND ArtistVotes.UserID IN ("
            "    SELECT UserID FROM UserActivity"
            "    WHERE UserActivity.LastActive > ?"
            ") ";

        if (options.session_id != -1) {
            query += "AND SessionID = ? ";
        } else {
            query += "AND SessionID != ? ";
        }

        query += "GROUP BY Artists.ArtistID ";

        switch (options.sort) {
            case SortType::Counts:
                query += "ORDER BY Count DESC ";
                break;
            case SortType::Votes:
                query += "ORDER BY Votes DESC ";
                break;
            default:
                break;
        }

        if (options.result_limit > 0) {
            query += "LIMIT " + to_string(options.result_limit);
        }

        if (sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int64(statement, 1, timestamp() - options.inactivity_threshold)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (options.session_id <= 0) {
            options.session_id = session_id_;
        }

        if (sqlite3_bind_int64(statement, 2, options.session_id)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        int result = 0;
        bool completed = false;
        while ((result = sqlite3_step(statement)) == SQLITE_ROW) {
            Artist a;
            a.id         = sqlite3_column_int(statement, 0);

            // See; getSongs
            if (a.id == 0) {
                completed = true;
                break;
            }

            a.name       = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
            a.count      = sqlite3_column_int(statement, 2);
            a.votes      = sqlite3_column_int(statement, 3);
            set_data.push_back(a);
        }

        sqlite3_finalize(statement);

        if (result != SQLITE_OK && result != SQLITE_DONE && !completed) {
            return Status::Error(sqlite3_errmsg(db_));
        }

		return Status::OK();
	}
    Status Sqlite3Store::getGenres(ResultSet<Genre>& set, ReadOptions options) {
        vector<Genre>& set_data = ResultSetMutator::getVector<Genre>(set);
        set_data.clear();

        sqlite3_stmt* statement = 0;

        string query = "SELECT Genres.GenreID, Name, COUNT(GenreVotes.GenreID) as Count, COALESCE(SUM(GenreVotes.Vote), 0) as Votes FROM Genres "
            "LEFT JOIN GenreVotes on Genres.GenreID = GenreVotes.GenreID AND GenreVotes.UserID IN ("
            "    SELECT UserID FROM UserActivity"
            "    WHERE UserActivity.LastActive > ?"
            ") ";

        if (options.session_id != -1) {
            query += "AND SessionID = ? ";
        } else {
            query += "AND SessionID != ? ";
        }

        query += "GROUP BY Genres.GenreID ";

        switch (options.sort) {
            case SortType::Counts:
                query += "ORDER BY Count DESC ";
                break;
            case SortType::Votes:
                query += "ORDER BY Votes DESC ";
                break;
            default:
                break;
        }

        if (options.result_limit > 0) {
            query += "LIMIT " + to_string(options.result_limit);
        }

        if (sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int64(statement, 1, timestamp() - options.inactivity_threshold)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (options.session_id <= 0) {
            options.session_id = session_id_;
        }

        if (sqlite3_bind_int64(statement, 2, options.session_id)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        int result = 0;
        bool completed = false;
        while ((result = sqlite3_step(statement)) == SQLITE_ROW) {
            Genre g;
            g.id    = sqlite3_column_int(statement, 0);

            // See: getSongs
            if (g.id == 0) {
                completed = true;
                break;
            }

            g.name  = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
            g.count = sqlite3_column_int(statement, 2);
            g.votes = sqlite3_column_int(statement, 3);
            set_data.push_back(g);
        }

        sqlite3_finalize(statement);

        if (result != SQLITE_OK && result != SQLITE_DONE && !completed) {
            return Status::Error(sqlite3_errmsg(db_));
        }

		return Status::OK();
	}

    Status Sqlite3Store::getSongFromId(Song& s, int songId) {
        sqlite3_stmt* statement = 0;

        string query =
            "SELECT Songs.SongID, Songs.Name, PlayHistory.Timestamp, Artists.ArtistID, Artists.Name, Genres.GenreID, Genres.Name FROM Songs "
            "LEFT JOIN Artists     ON Songs.ArtistID = Artists.ArtistID "
            "LEFT JOIN Genres      ON Songs.GenreID  = Genres.GenreID "
            "LEFT JOIN PlayHistory ON Songs.SongID   = PlayHistory.SongID AND PlayHistory.SessionID = " + to_string(session_id_) + " "
            "WHERE Songs.SongID = " + to_string(songId);

        int result = 0;
        s.id = -1;

        sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0);
        while ((result = sqlite3_step(statement)) == SQLITE_ROW) {
            s.id          = sqlite3_column_int(statement, 0);
            s.name        = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
            s.last_played = sqlite3_column_int64(statement, 2);

            s.artist.id   = sqlite3_column_int(statement, 3);
            if (s.artist.id > 0) {
                s.artist.name = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 4)));
            }

            s.genre.id    = sqlite3_column_int(statement, 5);
            if (s.genre.id > 0) {
                s.genre.name  = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 6)));
            }
        }

        sqlite3_finalize(statement);

        if (result != SQLITE_OK && result != SQLITE_DONE) {
            return Status::Error(sqlite3_errmsg(db_));
        }

		if (s.id == -1) {
            return Status::NotFound("Could not find song");
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
            "ORDER BY DESC Timestamp";

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
            if (s.artist.id > 0) {
                s.artist.name = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 6)));
            }

            s.genre.id    = sqlite3_column_int(statement, 7);
            if (s.genre.id > 0) {
                s.genre.name  = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 8)));
            }

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
        Song s;
		Status status = getSongFromId(s, songId);
		if (status != Status::OK()){
			return status;
		}

        lock_guard<mutex> lock(queue_lock_);
        song_queue_.push_back(s);

		return Status::OK();
	}
    Status Sqlite3Store::clearQueue() {
        lock_guard<mutex> lock(queue_lock_);
        song_queue_.clear();

        return Status::OK();
    }
    Status Sqlite3Store::songFinished() {
        if (song_buffer_.empty()) {
            return Status::Error("Buffer empty");
        }

        Song song;

        {
            lock_guard<mutex> lock(buffer_lock_);
            song = song_buffer_.front();
            song_buffer_.erase(song_buffer_.begin());
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
        //
        // After a few months (coming back fresh), while the above
        // statement is true in the sense of stale data, we only
        // need to update timestamp, which is trivial in SQL, so
        // no need to update. My guess is that comment was written
        sqlite3_stmt* statement = 0;

        string query = "REPLACE INTO `PlayHistory` (`SongID`, `SessionID`, `Timestamp`) VALUES (?, ?, ?)";

        if (sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int(statement, 1, song.id)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int(statement, 2, session_id_)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int64(statement, 3, song.last_played)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        int r = sqlite3_step(statement);
        sqlite3_finalize(statement);

        if (r != SQLITE_OK && r != SQLITE_DONE) {
            return Status::Error(sqlite3_errmsg(db_));
        }

		return Status::OK();
	}

    Status Sqlite3Store::getBuffer(ResultSet<Song>& set) {
        vector<Song>& set_data = ResultSetMutator::getVector(set);
        set_data.clear();

        lock_guard<mutex> lock(buffer_lock_);
        copy(song_buffer_.begin(), song_buffer_.end(), back_inserter(set_data));

		return Status::OK();
	}

    Status Sqlite3Store::bufferNext() {
        lock_guard<mutex> queue_lock(queue_lock_);

        if (song_queue_.empty()) {
            return Status::Error("Queue empty");
        }

        lock_guard<mutex> buffer_lock(buffer_lock_);

        copy(song_queue_.begin(), song_queue_.begin() + 1, back_inserter(song_buffer_));
        song_queue_.erase(song_queue_.begin());
        song_buffer_ids_.insert(song_buffer_.begin()->id);

		return Status::OK();
	}

	Status Sqlite3Store::setActivity(std::string userId, int64_t timestamp) {
        sqlite3_stmt* statement = 0;

        // SQLite3 does not support an INSERT OR UPDATE query, so we have
        // two options:
        //     1. Delete and Recreate: This doesn't work due to FK constraints
        //     2. Try update, if fail, insert: Annoying, but should be okay in most cases.
        string query = "UPDATE `UserActivity` SET LastActive = ? where UserID = ?";

        if (sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int64(statement, 1, timestamp)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_text(statement, 2, userId.c_str(), userId.size(), SQLITE_STATIC)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        int r = sqlite3_step(statement);
        sqlite3_finalize(statement);

        if (r != SQLITE_OK && r != SQLITE_DONE) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        int affected = sqlite3_changes(db_);
        if (affected) {
            return Status::OK();
        }

        query = "INSERT INTO `UserActivity` (`UserId`, `LastActive`) VALUES (?, ?)";

        if (sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_text(statement, 1, userId.c_str(), userId.size(), SQLITE_STATIC)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int64(statement, 2, timestamp)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        r = sqlite3_step(statement);
        sqlite3_finalize(statement);

        if (r != SQLITE_OK && r != SQLITE_DONE) {
            return Status::Error(sqlite3_errmsg(db_));
        }

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

        song.id = (int) sqlite3_last_insert_rowid(db_);
        song.last_played = 0;

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

        artist.id = (int) sqlite3_last_insert_rowid(db_);

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

        genre.id = (int) sqlite3_last_insert_rowid(db_);

        if (r != SQLITE_OK && r != SQLITE_DONE) {
            return Status::Error(sqlite3_errmsg(db_));
        }

		return Status::OK();
	}

    Status Sqlite3Store::insertNormalized(string normalized, int songId, int artistId, int genreId) {
        sqlite3_stmt* statement = 0;

        string query = "REPLACE INTO `Normalized` (`Normalized`, `SongID`, `ArtistID`, `GenreID`) VALUES (?, ?, ?, ?)";

        if (sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_text(statement, 1, normalized.c_str(), normalized.size(), SQLITE_STATIC)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int(statement, 2, songId)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int(statement, 3, artistId)) {
            return Status::Error(sqlite3_errmsg(db_));
        }
        if (sqlite3_bind_int(statement, 4, genreId)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        int r = sqlite3_step(statement);
        sqlite3_finalize(statement);

        if (r != SQLITE_OK && r != SQLITE_DONE) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        return Status::OK();
    }

    Status Sqlite3Store::getNormalized(Song& song, string normalized) {
        sqlite3_stmt* statement = 0;

        string query =
            "SELECT Normalized.SongID, Songs.Name, Normalized.ArtistID, Artists.Name, Normalized.GenreID, Genres.Name FROM Normalized "
            "LEFT JOIN Songs   ON Normalized.SongID == Songs.SongID "
            "LEFT JOIN Artists ON Normalized.ArtistID == Artists.ArtistID "
            "LEFT JOIN Genres  ON Normalized.GenreID == Genres.GenreID "
            "WHERE Normalized.Normalized = ?";

        if (sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_text(statement, 1, normalized.c_str(), normalized.size(), SQLITE_STATIC)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        int id = 0;
        int count = 0;
        int result = 0;
        while ((result = sqlite3_step(statement)) == SQLITE_ROW) {
            count++;
            id = sqlite3_column_int(statement, 0);
            if (id != 0) {
                song.id   = id;
                song.name = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
            }

            id = sqlite3_column_int(statement, 2);
            if (id != 0) {
                song.artist.id   = id;
                song.artist.name = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 3)));
            }

            id = sqlite3_column_int(statement, 4);
            if (id != 0) {
                song.genre.id   = id;
                song.genre.name = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 5)));
            }
        }

        sqlite3_finalize(statement);

        if (result != SQLITE_OK && result != SQLITE_DONE) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (!count) {
            return Status::NotFound("Could not find normalized entry");
        }

        return Status::OK();
    }

    Status Sqlite3Store::voteSong(std::string userId, Song& song, int amount, WriteOptions options) {
        sqlite3_stmt* statement = 0;

        if (song.id == 0) {
            return Status::Error("Cannot count a song that does not exist");
        }

        Status s = setActivity(userId, timestamp());
        if (s != Status::OK()) {
            return s;
        }

        string query = "REPLACE INTO `SongVotes` (`SongID`, `SessionID`, `UserID`, `Vote`) VALUES (?, ?, ?, ?)";

        if (sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int(statement, 1, song.id)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int(statement, 2, session_id_)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_text(statement, 3, userId.c_str(), userId.size(), SQLITE_STATIC)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int(statement, 4, amount)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        int r = sqlite3_step(statement);
        sqlite3_finalize(statement);

        if (r != SQLITE_OK && r != SQLITE_DONE) {
            return Status::Error(sqlite3_errmsg(db_));
        }

		return Status::OK();
	}
    Status Sqlite3Store::voteArtist(std::string userId, Artist& artist, int amount, WriteOptions options) {
        sqlite3_stmt* statement = 0;

        if (artist.id == 0) {
            return Status::Error("Cannot count an artist that does not exist");
        }

        Status s = setActivity(userId, timestamp());
        if (s != Status::OK()) {
            return s;
        }

        string query = "REPLACE INTO `ArtistVotes` (`ArtistID`, `SessionID`, `UserID`, `Vote`) VALUES (?, ?, ?, ?)";

        if (sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int(statement, 1, artist.id)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int(statement, 2, session_id_)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_text(statement, 3, userId.c_str(), userId.size(), SQLITE_STATIC)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int(statement, 4, amount)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        int r = sqlite3_step(statement);
        sqlite3_finalize(statement);

        if (r != SQLITE_OK && r != SQLITE_DONE) {
            return Status::Error(sqlite3_errmsg(db_));
        }

		return Status::OK();
	}
    Status Sqlite3Store::voteGenre(std::string userId, Genre& genre, int amount, WriteOptions options) {
        sqlite3_stmt* statement = 0;

        if (genre.id == 0) {
            return Status::Error("Cannot count a song that does not exist");
        }

        Status s = setActivity(userId, timestamp());
        if (s != Status::OK()) {
            return s;
        }

        string query = "REPLACE INTO `GenreVotes` (`GenreID`, `SessionID`, `UserID`, `Vote`) VALUES (?, ?, ?, ?)";

        if (sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int(statement, 1, genre.id)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int(statement, 2, session_id_)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_text(statement, 3, userId.c_str(), userId.size(), SQLITE_STATIC)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int(statement, 4, amount)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        int r = sqlite3_step(statement);
        sqlite3_finalize(statement);

        if (r != SQLITE_OK && r != SQLITE_DONE) {
            return Status::Error(sqlite3_errmsg(db_));
        }

		return Status::OK();
	}

    Status Sqlite3Store::createSession() {
        int64_t result = 0;
        return createSession(result);
	}
    Status Sqlite3Store::createSession(int64_t& result) {
        sqlite3_stmt* statement = 0;

        string query = "INSERT INTO `SessionHistory` (`Date`) VALUES (?)";

        if (sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        if (sqlite3_bind_int64(statement, 1, timestamp())) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        int r = sqlite3_step(statement);
        sqlite3_finalize(statement);

        if (r != SQLITE_OK && r != SQLITE_DONE) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        session_id_ = (int64_t) sqlite3_last_insert_rowid(db_);
        result = session_id_;

		return Status::OK();
	}

    Status Sqlite3Store::getSession(int64_t& result) {
        result = session_id_;
		return Status::OK();
	}
    Status Sqlite3Store::getSessionCount(int& result) {
        sqlite3_stmt* statement = 0;

        string query = "SELECT Count(*) FROM `SessionHistory`";

        if (sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0)) {
            return Status::Error(sqlite3_errmsg(db_));
        }

        int r = 0;
        while ((r = sqlite3_step(statement)) == SQLITE_ROW) {
            result = sqlite3_column_int(statement, 0);
        }

        sqlite3_finalize(statement);

        if (r != SQLITE_OK && r != SQLITE_DONE) {
            return Status::Error(sqlite3_errmsg(db_));
        }

		return Status::OK();
	}

}
}

