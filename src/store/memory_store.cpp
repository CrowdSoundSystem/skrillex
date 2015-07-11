#include <algorithm>
#include <chrono>
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
    : MemoryStore(0)
    {
    }

    MemoryStore::MemoryStore(int session_id)
    : song_id_counter_(0)
    , artist_id_counter_(0)
    , genre_id_counter_(0)
    , session_id_(session_id)
    {
    }

    MemoryStore::~MemoryStore() {
    }

    Status MemoryStore::getSongs(ResultSet<Song>& set, ReadOptions options) {
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

        size_t erase_size = set_data.size() - (size_t)options.result_limit;
        if (erase_size > 0) {
            set_data.erase(set_data.end() - erase_size, set_data.end());
        }

        return Status::OK();
}

    Status MemoryStore::getArtists(ResultSet<Artist>& set, ReadOptions options) {
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

        size_t erase_size = set_data.size() - (size_t)options.result_limit;
        if (erase_size > 0) {
            set_data.erase(set_data.end() - erase_size, set_data.end());
        }

        return Status::OK();
    }

    Status MemoryStore::getGenres(ResultSet<Genre>& set, ReadOptions options) {
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

        size_t erase_size = set_data.size() - (size_t)options.result_limit;
        if (erase_size > 0) {
            set_data.erase(set_data.end() - erase_size, set_data.end());
        }

        return Status::OK();
    }

    Status MemoryStore::getPlayHistory(ResultSet<Song>& set, ReadOptions options) {
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
        // For now, just overwrite the result set
        // data. In the future, we can do more intelligent things.
        vector<Song>& set_data = ResultSetMutator::getVector<Song>(set);
        set_data.clear();

        copy(songs_.begin(), songs_.end(), back_inserter(set_data));

        return Status::OK();
    }

    Status MemoryStore::queueSong(int song_id) {
        // Make sure the song id actually exists!
        auto song = find_if(songs_.begin(), songs_.end(), [song_id](const Song& s) {
            return s.id == song_id;
        });

        if (song == songs_.end()) {
            return Status::NotFound("Could not queue song");
        }

        song_queue_.push(*song);

        return Status::OK();
    }

    Status MemoryStore::songFinished() {
        if (song_queue_.empty()) {
            return Status::Error("Queue Empty");
        }

        Song song = song_queue_.front();
        song_queue_.pop();

        auto it = find_if(songs_.begin(), songs_.end(), [&song](const Song& s) {
            return s.id == song.id;
        });

        if (it == songs_.end()) {
            return Status::Error("Invalid State: Song existed in queue, but not database");
        }

        it->last_played = timestamp();

        return Status::OK();
    }

    Status MemoryStore::addSong(Song& song) {
        song.id = ++song_id_counter_;
        song.count = 0;
        song.votes = 0;

        for (auto& it : songs_) {
            if (it.first == session_id_) {
                Song s = song;
                s.count = 1;
                it.second.push_back(s);
            } else {
                it.second.push_back(song);
            }
        }

        return Status::OK();
    }

    Status MemoryStore::addArtist(Artist& artist) {
        artist.id = ++artist_id_counter_;
        artist.count = 0;
        artist.votes = 0;

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
        genre.id = ++genre_id_counter_;
        genre.count = 0;
        genre.votes = 0;

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

    Status MemoryStore::voteSong(Song& song, int amount, WriteOptions options) {
        if (!options.session_id) {
            options.session_id = session_id_;
        }

        auto s = find_if(songs_.begin(), songs_.end(), [options, song] (const Song& s) {
            return song.id == s.id && s.session_id == options.session_id;
        });

        if (s == songs_.end()) {
            return Status::NotFound("Could not find song to vote.");
        }

        s->votes += amount;
        song.votes = s->votes;

        return Status::OK();
    }

    Status MemoryStore::voteArtist(Artist& artist, int amount, WriteOptions options) {
        if (!options.session_id) {
            options.session_id = session_id_;
        }

        auto a = find_if(artists_.begin(), artists_.end(), [options, artist] (const Artist& a) {
            return artist.id == a.id && a.session_id == options.session_id;
        });

        if (a == artists_.end()) {
            return Status::NotFound("Could not find artist to vote.");
        }

        a->votes += amount;
        artist.votes = a->votes;

        return Status::OK();
    }

    Status MemoryStore::voteGenre(Genre& genre, int amount, WriteOptions options) {
        if (!options.session_id) {
            options.session_id = session_id_;
        }

        auto g = find_if(genres_.begin(), genres_.end(), [options, genre] (const Genre& g) {
            return genre.id == g.id && g.session_id == options.session_id;
        });

        if (g == genres_.end()) {
            return Status::NotFound("Could not find genre to vote.");
        }

        g->votes += amount;
        genre.votes = g->votes;

        return Status::OK();
    }

    Status MemoryStore::createSession() {
        session_id_++;

        return Status::OK();
    }
    Status MemoryStore::createSession(int& result) {
        result = ++session_id_;

        return Status::OK();
    }

    Status MemoryStore::getSession(int& result) {
        result = session_id_;

        return Status::OK();
    }
}
}
