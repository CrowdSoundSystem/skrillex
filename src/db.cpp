#include <string>

#include "skrillex/db.hpp"
#include "store/store.hpp"
#include "store/memory_store.hpp"

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

        // TODO: Support SQLite3
        if (!options.memory_only) {
            return Status::Error("Non-memory databases not implemented");
        }

        db = new DB(path, options);
        db->db_state_ = DB::State::Open;

        // Create the underlying store
        if (options.memory_only) {
            db->store_ = make_unique<MemoryStore>();
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

    Status DB::addSong(Song& song) {
        return store_->addSong(song);
    }

    Status DB::addArtist(Artist& artist) {
        return store_->addArtist(artist);
    }

    Status DB::addGenre(Genre& genre) {
        return store_->addGenre(genre);
    }

    Status DB::vote(Song& song, int amount) {
        if (!isOpen()) {
            return Status::Error("Database closed.");
        }

        return store_->vote(song, amount);
    }
}
