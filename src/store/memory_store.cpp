#include <algorithm>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "skrillex/dbo.hpp"
#include "store/memory_store.hpp"
#include "mutator.hpp"

using namespace std;

namespace skrillex {
namespace internal {
    bool playSort(const Song& a, const Song& b) {
        return a.last_played > b.last_played;
    }

    bool countSort(const Countable& a, const Countable& b) {
        return a.count > b.count;
    }

    bool voteSort(const Countable& a, const Countable& b) {
        return a.votes > b.votes;
    }

    uint64_t timestamp() {
        return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    }

    MemoryStore::MemoryStore()
    : song_id_counter_(0)
    , artist_id_counter_(0)
    , genre_id_counter_(0)
    , session_id_(0)
    {
    }

    MemoryStore::~MemoryStore() {
    }

    Status MemoryStore::getSongs(ResultSet<Song>& set, ReadOptions options) {
        lock_guard<mutex> lock(mutex);
        // For now, just overwrite the result set
        // data. In the future, we can do more intelligent things.
        vector<Song>& set_data = ResultSetMutator::getVector<Song>(set);
        set_data.clear();

        // If no explicit option was set, just use the current one.
        if (!options.session_id) {
            options.session_id = session_id_;
        }

        if (options.session_id == -1) {
            return Status::NotImplemented("Memory Store does not implement NOT EQUAL session queries.");
        }

        auto songs = songs_.find(options.session_id);
        if (songs == songs_.end()) {
            return Status::NotFound("Session not found when querying songs.");
        }

        // Copy the entire list, so we can get a proper sorting.
        // After that, we'll trim the list based on any limits.
        copy(songs->second.begin(), songs->second.end(), back_inserter(set_data));

        switch (options.sort) {
            case SortType::Counts:
                sort(set_data.begin(), set_data.end(), countSort);
                break;
            case SortType::Votes:
                sort(set_data.begin(), set_data.end(), voteSort);
                break;
            default:
                break;
        }

        if (options.result_limit) {
            size_t erase_size = set_data.size() - (size_t)options.result_limit;
            if (erase_size > 0) {
                set_data.erase(set_data.end() - erase_size, set_data.end());
            }
        }

        return Status::OK();
}

    Status MemoryStore::getArtists(ResultSet<Artist>& set, ReadOptions options) {
        lock_guard<mutex> lock(mutex);

        // For now, just overwrite the result set
        // data. In the future, we can do more intelligent things.
        vector<Artist>& set_data = ResultSetMutator::getVector<Artist>(set);
        set_data.clear();

        // If no explicit option was set, just use the current one.
        if (!options.session_id) {
            options.session_id = session_id_;
        }

        if (options.session_id == -1) {
            return Status::NotImplemented("Memory Store does not implement NOT EQUAL session queries.");
        }

        auto artists = artists_.find(options.session_id);
        if (artists == artists_.end()) {
            return Status::NotFound("Session not found when querying artists.");
        }

        // Copy the entire list, so we can get a proper sorting.
        // After that, we'll trim the list based on any limits.
        copy(artists->second.begin(), artists->second.end(), back_inserter(set_data));

        switch (options.sort) {
            case SortType::Counts:
                sort(set_data.begin(), set_data.end(), countSort);
                break;
            case SortType::Votes:
                sort(set_data.begin(), set_data.end(), voteSort);
                break;
            default:
                break;
        }

        if (options.result_limit) {
            size_t erase_size = set_data.size() - (size_t)options.result_limit;
            if (erase_size > 0) {
                set_data.erase(set_data.end() - erase_size, set_data.end());
            }
        }

        return Status::OK();
    }

    Status MemoryStore::getGenres(ResultSet<Genre>& set, ReadOptions options) {
        lock_guard<mutex> lock(mutex_);

        // For now, just overwrite the result set
        // data. In the future, we can do more intelligent things.
        vector<Genre>& set_data = ResultSetMutator::getVector<Genre>(set);
        set_data.clear();

        // If no explicit option was set, just use the current one.
        if (!options.session_id) {
            options.session_id = session_id_;
        }

        if (options.session_id == -1) {
            return Status::NotImplemented("Memory Store does not implement NOT EQUAL session queries.");
        }

        auto genres = genres_.find(options.session_id);
        if (genres == genres_.end()) {
            return Status::NotFound("Session not found when querying genres.");
        }

        // Copy the entire list, so we can get a proper sorting.
        // After that, we'll trim the list based on any limits.
        copy(genres->second.begin(), genres->second.end(), back_inserter(set_data));

        switch (options.sort) {
            case SortType::Counts:
                sort(set_data.begin(), set_data.end(), countSort);
                break;
            case SortType::Votes:
                sort(set_data.begin(), set_data.end(), voteSort);
                break;
            default:
                break;
        }

        if (options.result_limit) {
            size_t erase_size = set_data.size() - (size_t)options.result_limit;
            if (erase_size > 0) {
                set_data.erase(set_data.end() - erase_size, set_data.end());
            }
        }

        return Status::OK();
    }

    Status MemoryStore::getPlayHistory(ResultSet<Song>& set, ReadOptions options) {
        lock_guard<mutex> lock(mutex_);

        // For now, just overwrite the result set
        // data. In the future, we can do more intelligent things.
        vector<Song>& set_data = ResultSetMutator::getVector<Song>(set);
        set_data.clear();

        if (options.session_id) {
            options.session_id = session_id_;
        }

        if (options.session_id == -1) {
            return Status::NotImplemented("Memory Store does not implement NOT EQUAL session queries.");
        }

        auto songs = songs_.find(options.session_id);
        if (songs == songs_.end()) {
            return Status::NotFound("Session not found when querying play history");
        }

        // Copy the entire list, so we can get a proper sorting.
        // After that, we'll trim the list based on any limits.
        copy(songs->second.begin(), songs->second.end(), back_inserter(set_data));

        // Now, let's sort based on play date.
        sort(set_data.begin(), set_data.end(), playSort);

        // Now trim to the desired size (if necessary)
        size_t erase_size = set_data.size() - (size_t)options.result_limit;
        if (erase_size > 0) {
            set_data.erase(set_data.end() - erase_size, set_data.end());
        }

        return Status::OK();
    }

    Status MemoryStore::getQueue(ResultSet<Song>& set) {
        lock_guard<mutex> lock(mutex_);

        /// For now, just overwrite the result set
        // data. In the future, we can do more intelligent things.
        vector<Song>& set_data = ResultSetMutator::getVector<Song>(set);
        set_data.clear();

        copy(song_queue_.begin(), song_queue_.end(), back_inserter(set_data));

        return Status::OK();
    }

    Status MemoryStore::queueSong(int song_id) {
        lock_guard<mutex> lock(mutex_);

        /// Make sure the song id actually exists!
        auto session = songs_.find(session_id_);
        if (session == songs_.end()) {
            return Status::Error("Could not queue song: session uninitialized");
        }

        auto song = find_if(session->second.begin(), session->second.end(), [song_id](const Song& s) {
            return s.id == song_id;
        });

        if (song == session->second.end()) {
            return Status::NotFound("Could not queue song");
        }

        song_queue_.push_back(*song);

        return Status::OK();
    }

    Status MemoryStore::songFinished() {
        lock_guard<mutex> lock(mutex_);

        if (song_queue_.empty()) {
            return Status::Error("Queue Empty");
        }

        Song song = song_queue_.front();
        song_queue_.erase(song_queue_.begin());

        uint64_t ts = timestamp();

        for (auto& it : songs_) {
            auto s = find_if(it.second.begin(), it.second.end(), [&song](const Song& s) {
                return s.id == song.id;
            });

            if (s == it.second.end()) {
               return Status::Error("Invalid State: Song existed in queue, but not in database");
            }

            s->last_played = ts;
        }

        return Status::OK();
    }

    Status MemoryStore::addSong(Song& song) {
        lock_guard<mutex> lock(mutex_);

        song.id = ++song_id_counter_;
        song.last_played = 0;
        song.count = 0;
        song.votes = 0;

        if (songs_.empty()) {
            return Status::Error("Invalid State: No Song Table.");
        }

        for (auto& it : songs_) {
            // If the song exists, don't do anything.
            // Note: We're relying on the fact that all
            // sessions have the same songs.
            auto s = find_if(it.second.begin(), it.second.end(), [&song] (const Song& s) {
                return s.id == song.id;
            });

            if (s != it.second.end()) {
                return Status::OK();
            }

            it.second.push_back(song);
        }

        return Status::OK();
    }

    Status MemoryStore::addArtist(Artist& artist) {
        lock_guard<mutex> lock(mutex_);

        artist.id = ++artist_id_counter_;
        artist.last_played = 0;
        artist.count = 0;
        artist.votes = 0;

        if (artists_.empty()) {
            return Status::Error("Invalid State: No Artist Table.");
        }

        for (auto& it : artists_) {
            if (it.first == session_id_) {
                Artist a = artist;
                a.count = 1;
                it.second.push_back(a);
            } else {
                it.second.push_back(artist);
            }
        }

        return Status::OK();
    }

    Status MemoryStore::addGenre(Genre& genre) {
        lock_guard<mutex> lock(mutex_);

        genre.id = ++genre_id_counter_;
        genre.last_played = 0;
        genre.count = 0;
        genre.votes = 0;

        if (genres_.empty()) {
            return Status::Error("Invalid State: No Genre Table.");
        }

        for (auto& it : genres_) {
            if (it.first == session_id_) {
                Genre g = genre;
                g.count = 1;
                it.second.push_back(g);
            } else {
                it.second.push_back(genre);
            }
        }

        return Status::OK();
    }

    Status MemoryStore::countSong(Song& song, WriteOptions options) {
        lock_guard<mutex> lock(mutex_);

        if (!options.session_id) {
            options.session_id = session_id_;
        }

        if (options.session_id == -1) {
            return Status::NotImplemented("Memory Store does not implement NOT EQUAL session queries");
        }

        auto session = songs_.find(options.session_id);
        if (session == songs_.end()) {
            return Status::Error("Invalid state: session not found.");
        }

        auto s = find_if(session->second.begin(), session->second.end(), [options, song] (const Song& s) {
            return song.id == s.id && s.session_id == options.session_id;
        });

        if (s == session->second.end()) {
            return Status::NotFound("Could not find song to count.");
        }

        s->count++;
        song.count = s->count;

        return Status::OK();
    }

    Status MemoryStore::countArtist(Artist& artist, WriteOptions options) {
        lock_guard<mutex> lock(mutex_);

        if (!options.session_id) {
            options.session_id = session_id_;
        }

        if (options.session_id == -1) {
            return Status::NotImplemented("Memory Store does not implement NOT EQUAL session queries");
        }

        auto session = artists_.find(options.session_id);
        if (session == artists_.end()) {
            return Status::Error("Invalid state: session not found.");
        }

        auto a = find_if(session->second.begin(), session->second.end(), [options, artist] (const Artist& a) {
            return artist.id == a.id && a.session_id == options.session_id;
        });

        if (a == session->second.end()) {
            return Status::NotFound("Could not find artist to count.");
        }

        a->count++;
        artist.count = a->count;

        return Status::OK();
;
    }

    Status MemoryStore::countGenre(Genre& genre, WriteOptions options) {
        lock_guard<mutex> lock(mutex_);

        if (!options.session_id) {
            options.session_id = session_id_;
        }

        if (options.session_id == -1) {
            return Status::NotImplemented("Memory Store does not implement NOT EQUAL session queries");
        }

        auto session = genres_.find(options.session_id);
        if (session == genres_.end()) {
            return Status::Error("Invalid state: session not found.");
        }

        auto g = find_if(session->second.begin(), session->second.end(), [options, genre] (const Genre& g) {
            return genre.id == g.id && g.session_id == options.session_id;
        });

        if (g == session->second.end()) {
            return Status::NotFound("Could not find genre to count.");
        }

        g->count++;
        genre.count = g->count;

        return Status::OK();
    }

    Status MemoryStore::voteSong(Song& song, int amount, WriteOptions options) {
        lock_guard<mutex> lock(mutex_);

        if (!options.session_id) {
            options.session_id = session_id_;
        }

        if (options.session_id == -1) {
            return Status::NotImplemented("Memory Store does not implement NOT EQUAL session queries.");
        }

        auto session = songs_.find(options.session_id);
        if (session == songs_.end()) {
            return Status::Error("Invalid state: session not found.");
        }

        auto s = find_if(session->second.begin(), session->second.end(), [options, song] (const Song& s) {
            return song.id == s.id && s.session_id == options.session_id;
        });

        if (s == session->second.end()) {
            return Status::NotFound("Could not find song to vote.");
        }

        s->votes += amount;
        song.votes = s->votes;

        return Status::OK();
    }

    Status MemoryStore::voteArtist(Artist& artist, int amount, WriteOptions options) {
        lock_guard<mutex> lock(mutex_);

        if (!options.session_id) {
            options.session_id = session_id_;
        }

        if (options.session_id == -1) {
            return Status::NotImplemented("Memory Store does not implement NOT EQUAL session queries.");
        }

        auto session = artists_.find(options.session_id);
        if (session == artists_.end()) {
            return Status::Error("Invalid state: session not found");
        }

        auto a = find_if(session->second.begin(), session->second.end(), [options, artist] (const Artist& a) {
            return artist.id == a.id && a.session_id == options.session_id;
        });

        if (a == session->second.end()) {
            return Status::NotFound("Could not find song to vote.");
        }

        a->votes += amount;
        artist.votes = a->votes;

        return Status::OK();
    }

    Status MemoryStore::voteGenre(Genre& genre, int amount, WriteOptions options) {
        lock_guard<mutex> lock(mutex_);

        if (!options.session_id) {
            options.session_id = session_id_;
        }

        if (options.session_id == -1) {
            return Status::NotImplemented("Memory Store does not implement NOT EQUAL session queries.");
        }

        auto session = genres_.find(options.session_id);
        if (session == genres_.end()) {
            return Status::Error("Invalid state: session not found");
        }

        auto g = find_if(session->second.begin(), session->second.end(), [options, genre] (const Genre& g) {
            return genre.id == g.id && g.session_id == options.session_id;
        });

        if (g == session->second.end()) {
            return Status::NotFound("Could not find song to vote.");
        }

        g->votes += amount;
        genre.votes = g->votes;

        return Status::OK();
    }

    Status MemoryStore::createSession() {
        int r = 0;
        return createSession(r);
    }

    Status MemoryStore::createSession(int& result) {
        lock_guard<mutex> lock(mutex_);

        // If a session exists, copy and reset state
        if (session_id_) {
            auto songs = songs_.find(session_id_);
            if (songs == songs_.end()) {
                return Status::Error("Could not create session, no songs in session.");
            }

            auto artists = artists_.find(session_id_);
            if (artists == artists_.end()) {
                return Status::Error("Could not create session, no artists in session.");
            }

            auto genres = genres_.find(session_id_);
            if (genres == genres_.end()) {
                return Status::Error("Could not create session, no genres in session.");
            }

            session_id_++;
            songs_[session_id_]    = songs->second;
            artists_[session_id_]  = artists->second;
            genres_[session_id_]   = genres->second;

            // Update all the session stuff. Lol so hacky...
            for (auto& s : songs_[session_id_]) {
                s.session_id = session_id_;
            }
            for (auto& a : artists_[session_id_]) {
                a.session_id = session_id_;
            }
            for (auto& g : genres_[session_id_]) {
                g.session_id = session_id_;
            }
        } else {
            session_id_++;

            songs_[session_id_]   = std::vector<Song>();
            artists_[session_id_] = std::vector<Artist>();
            genres_[session_id_]  = std::vector<Genre>();

        }

        sessions_.insert(session_id_);
        result = session_id_;

        return Status::OK();
    }

    Status MemoryStore::getSession(int& result) {
        lock_guard<mutex> lock(mutex_);

        result = session_id_;

        return Status::OK();
    }

    Status MemoryStore::getSessionCount(int& result) {
        lock_guard<mutex> lock(mutex_);

        result = sessions_.size();

        return Status::OK();
    }
}
}
