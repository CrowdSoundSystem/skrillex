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

    MemoryStore::MemoryStore()
    : current_song_id_(0)
    {
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

        copy(songs_.begin(), it, front_inserter(set_data));

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

        copy(artists_.begin(), it, front_inserter(set_data));

        return Status::OK();
    }

    Status MemoryStore::getGenres(ResultSet<Genre>& set, ReadOptions options) {
        // For now, just overwrite the result set
        // data. In the future, we can do more intelligent things.
        vector<Genre>& set_data = ResultSetMutator::getVector<Genre>(set);
        set_data.clear();

        vector<Artist>::iterator it;
        if (!options.result_limit || (size_t)options.result_limit > artists_.size()) {
            it = artists_.end();
        } else {
            it = artists_.begin() + options.result_limit;
        }

        copy(artists_.begin(), it, front_inserter(set_data));

        return Status::OK();
    }

    Status MemoryStore::getPlayHistory(ResultSet<Song>& set, ReadOptions options) {
        // For now, just overwrite the result set
        // data. In the future, we can do more intelligent things.
        vector<Song>& set_data = ResultSetMutator::getVector<Song>(set);
        set_data.clear();

        // Copy the entire list, so we can get a proper sorting.
        // After that, we'll trim the list based on any limits.
        copy(songs_.begin(), songs_.end(), front_inserter(set_data));

        // Now, let's sort based on play date.
        sort(set_data.begin(), set_data.end(), playSort);

        // Now trim to the desired size (if necessary)
        size_t erase_size = set_data.size() - (size_t)options.result_limit;
        if (erase_size > 0) {
            set_data.erase(set_data.end() - erase_size, set_data.end());
        }

        return Status::OK();
    }

    Status MemoryStore::setSongPlayed(Song& song, WriteOptions options) {
    }

    Status MemoryStore::setSongFinished(Song& song, WriteOptions options) {
        // Update song database locally, using a sexy sexy lambda.
        auto it = find(songs_.begin(), songs_.end(), [&song](const Song& s) {
                return s.id == song.id;
        });

        if (it == songs_.end()) {
            return Status::NotFound("Could not find song to update");
        }

        it->last_played  = song.last_played;
        current_song_id_ = 0;

        return Status::OK();
    }
}
}
