#include <string>

#include "skrillex/db.hpp"

using namespace std;

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

    Status DB::songPlayed(Song& song)   { return songPlayed(song, WriteOptions()); }
    Status DB::songFinished(Song& song) { return songFinished(song, WriteOptions()); }

    Status DB::getSongs(ResultSet<Song>& rs, ReadOptions options) {
        return Status::OK();
    }
    Status DB::getArtists(ResultSet<Artist>& rs, ReadOptions options) {
        return Status::OK();
    }
    Status DB::getGenres(ResultSet<Genre>& rs, ReadOptions options) {
        return Status::OK();
    }

    Status DB::songPlayed(Song& song, WriteOptions options) {
        return Status::OK();
    }

    Status DB::songFinished(Song& song, WriteOptions options) {
    }
}
