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

    Status MemoryStore::getSongs(ResultSet<Song>& set, ReadOptions options) {
        // For now, just overwrite the result set
        // data. In the future, we can do more intelligent things.
        vector<Song>& set_data = ResultSetMutator::getVector<Song>(set);
        set_data.clear();

        vector<Song>::iterator it;
        if (!options.result_limit || (size_t)options.result_limit > songs_.size()) {
            it = songs_.end();
        } else {
            it = songs_.begin() + options.result_limit;
        }

        copy(songs_.begin(), songs_.end(), back_inserter(set_data));

        return Status::OK();
    }

    Status MemoryStore::getArtists(ResultSet<Artist>& set, ReadOptions options) {
        // For now, just overwrite the result set
        // data. In the future, we can do more intelligent things.
        vector<Artist>& set_data = ResultSetMutator::getVector<Artist>(set);
        set_data.clear();

        vector<Artist>::iterator it;
        if (!options.result_limit || (size_t)options.result_limit > artists_.size()) {
            it = artists_.end();
        } else {
            it = artists_.begin() + options.result_limit;
        }

        copy(artists_.begin(), it, back_inserter(set_data));

        return Status::OK();
    }

    Status MemoryStore::getGenres(ResultSet<Genre>& set, ReadOptions options) {
        // For now, just overwrite the result set
        // data. In the future, we can do more intelligent things.
        vector<Genre>& set_data = ResultSetMutator::getVector<Genre>(set);
        set_data.clear();

        vector<Genre>::iterator it;
        if (!options.result_limit || (size_t)options.result_limit > genres_.size()) {
            it = genres_.end();
        } else {
            it = genres_.begin() + options.result_limit;
        }

        copy(genres_.begin(), it, back_inserter(set_data));

        return Status::OK();
    }

    Status MemoryStore::getPlayHistory(ResultSet<Song>& set, ReadOptions options) {
        // For now, just overwrite the result set
        // data. In the future, we can do more intelligent things.
        vector<Song>& set_data = ResultSetMutator::getVector<Song>(set);
        set_data.clear();

        // Copy the entire list, so we can get a proper sorting.
        // After that, we'll trim the list based on any limits.
        copy(songs_.begin(), songs_.end(), back_inserter(set_data));

        // Now, let's sort based on play date.
        sort(set_data.begin(), set_data.end(), playSort);

        // Now trim to the desired size (if necessary)
        size_t erase_size = set_data.size() - (size_t)options.result_limit;
        if (erase_size > 0) {
            set_data.erase(set_data.end() - erase_size, set_data.end());
        }

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
        songs_.push_back(song);

        return Status::OK();
    }

    Status MemoryStore::addArtist(Artist& artist) {
        artist.id = ++artist_id_counter_;
        artists_.push_back(artist);

        return Status::OK();
    }

    Status MemoryStore::addGenre(Genre& genre) {
        genre.id = ++song_id_counter_;
        genres_.push_back(genre);

        return Status::OK();
    }
}
}
