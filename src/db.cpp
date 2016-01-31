#include <string>

#include "skrillex/db.hpp"
#include "store/store.hpp"
#include "store/sqlite3_store.hpp"

using namespace std;
using namespace skrillex::internal;

namespace skrillex {
    DB::DB(string path, Options options)
    : db_path_(move(path))
    , db_options_(options)
    {
    }

    DB::~DB() {
        db_state_ = State::Closed;
    }

    Status open(DB*& db, string path, Options options) {
        if (db) {
            return Status::Error("Database is already open.");
        }
        if (options.session_id) {
            return Status::Error("SQLite3 does not support restoring sessions.");
        }

        db = new DB(path, options);
        db->db_state_ = DB::State::Open;

        db->store_.reset(new Sqlite3Store());

        Status s;
        if ((s = db->store_->open(path, options))) {
            return s;
        }

        if ((s = db->store_->createSession())) {
            return s;
        }

        return Status::OK();
    }

    bool DB::isOpen() const {
        return db_state_ == State::Open;
    }

    void DB::close() {
    }

    Status DB::getSongs(ResultSet<Song>& rs)     { return getSongs(rs, ReadOptions()); }
    Status DB::getArtists(ResultSet<Artist>& rs) { return getArtists(rs, ReadOptions()); }
    Status DB::getGenres(ResultSet<Genre>& rs)   { return getGenres(rs, ReadOptions()); }

    Status DB::getSongs(ResultSet<Song>& rs, ReadOptions options) {
        if (!isOpen()) {
            return Status::Error("Database closed.");
        }

        return store_->getSongs(rs, options);
    }
    Status DB::getArtists(ResultSet<Artist>& rs, ReadOptions options) {
        if (!isOpen()) {
            return Status::Error("Database closed.");
        }

        return store_->getArtists(rs, options);
    }
    Status DB::getGenres(ResultSet<Genre>& rs, ReadOptions options) {
        if (!isOpen()) {
            return Status::Error("Database closed.");
        }

        return store_->getGenres(rs, options);
    }

    Status DB::getQueue(ResultSet<Song>& set) {
        if (!isOpen()) {
            return Status::Error("Database closed.");
        }

        return store_->getQueue(set);
    }

    Status DB::queueSong(int song_id) {
        if (!isOpen()) {
            return Status::Error("Database closed.");
        }

        return store_->queueSong(song_id);
    }

    Status DB::songFinished() {
        if (!isOpen()) {
            return Status::Error("Database closed.");
        }

        return store_->songFinished();
    }

    Status DB::setActivity(std::string userId, int64_t timestamp) {
        if (!isOpen()) {
            return Status::Error("Database closed.");
        }

        return store_->setActivity(userId, timestamp);
    }

    Status DB::addSong(Song& song) {
        if (!isOpen()) {
            return Status::Error("Database closed.");
        }

        return store_->addSong(song);
    }

    Status DB::addArtist(Artist& artist) {
        if (!isOpen()) {
            return Status::Error("Database closed.");
        }

        return store_->addArtist(artist);
    }

    Status DB::addGenre(Genre& genre) {
        if (!isOpen()) {
            return Status::Error("Database closed.");
        }

        return store_->addGenre(genre);
    }

    Status DB::voteSong(std::string userId, Song& song, int amount) {
        if (!isOpen()) {
            return Status::Error("Database closed.");
        }

        return store_->voteSong(userId, song, amount, WriteOptions());
    }

    Status DB::voteArtist(std::string userId, Artist& artist, int amount) {
        if (!isOpen()) {
            return Status::Error("Database closed.");
        }

        return store_->voteArtist(userId, artist, amount, WriteOptions());
    }

    Status DB::voteGenre(std::string userId, Genre& genre, int amount) {
        if (!isOpen()) {
            return Status::Error("Database closed.");
        }

        return store_->voteGenre(userId, genre, amount, WriteOptions());
    }
}
