#include <algorithm>
#include <iostream>
#include <string>

#include "skrillex/dbo.hpp"
#include "skrillex/status.hpp"
#include "skrillex/testing/populator.hpp"
#include "store/store.hpp"
#include "mutator.hpp"

using namespace std;
using namespace skrillex::internal;

namespace skrillex {
namespace testing {
    PopulatorData get_populator_data(int num_songs, int num_artists, int num_genres) {
        PopulatorData data;

        for (int i = 0; i < num_genres; i++) {
            Genre g;
            g.id         = i + 1;
            g.name       = "g" + to_string(i);
            g.count      = 0;
            g.votes      = 0;

            data.genres.push_back(g);
        }

        for (int i = 0; i < num_artists; i++) {
            Artist a;
            a.id         = i + 1;
            a.name       = "a" + to_string(i);
            a.votes      = 0;
            a.count      = 0;

            data.artists.push_back(a);
        }

        for (int i = 0; i < num_songs; i++) {
            Song s;
            s.id         = i + 1;
            s.artist     = data.artists[i % num_artists];
            s.genre      = data.genres[i % num_genres];
            s.name       = "s" + to_string(i);
            s.votes      = 0;
            s.count      = 0;

            data.songs.push_back(s);
        }

        return data;
    }

    Status populate_empty(DB* db, int num_songs, int num_artists, int num_genres) {
        if (!db) {
            return Status::Error("Cannot populate null database");
        }

        if (!db->isOpen()) {
            return Status::Error("Database not open.");
        }

        PopulatorData data = get_populator_data(num_songs, num_artists, num_genres);
        Status status = Status::OK();

        for (auto& g : data.genres) {
            status = db->addGenre(g);
            if (status) {
                return status;
            }
        }

        for (auto& a : data.artists) {
            status = db->addArtist(a);
            if (status) {
                return status;
            }
        }

        for (auto& s : data.songs) {
            status = db->addSong(s);
            if (status) {
                return status;
            }
        }

        return status;
    }

    Status populate_full(DB* db, int num_songs, int num_artists, int num_genres, int num_sessions) {
        Status status = populate_empty(db, num_songs, num_artists, num_genres);
        if (status != Status::OK()) {
            return status;
        }

        Store* store = StoreMutator::getStore(db);
        int session_count = 0;

        status = store->getSessionCount(session_count);
        if (status != Status::OK()) {
            return status;
        }

        if (session_count > 1) {
            status = Status::Error("Cannot populate a non-empty database.");
        }

        ResultSet<Song>   songs;
        status = db->getSongs(songs);
        if (status != Status::OK()) {
            return status;
        }
		if (songs.size() != num_songs) {
            //cout << "Songs detected: " << songs.size() << endl;
			return Status::Error("Invalid number of songs");
		}

        ResultSet<Artist> artists;
        status = db->getArtists(artists);
        if (status != Status::OK()) {
            return status;
        }

        ResultSet<Genre>  genres;
        status = db->getGenres(genres);
        if (status != Status::OK()) {
            return status;
        }

        for (int i = 0; i < num_sessions; i++) {
            int j = 0;
            for (auto& s : songs) {
                Song t = s;

                // Count in an increasing order
                for (int k = 0; k < (2 * i + j + 1); k++) {
                    db->voteSong("u" + to_string(k), t, 0);
                }

                // Vote in a decreasing order
                db->voteSong("user", t, 2 * i + songs.size() - j);
                j++;
            }

            j = 0;
            for (auto& a : artists) {
                Artist t = a;

                // Count in an increasing order
                for (int k = 0; k < (2 * i + j + 1); k++) {
                    db->voteArtist("u" + to_string(k), t, 0);
                }

                // Vote in a decreasing order
                db->voteArtist("user", t, 2 * i + artists.size() - j);
                j++;
            }

            j = 0;
            for (auto& g : genres) {
                Genre t = g;

                // Count in an increasing order
                for (int k = 0; k < (2 * i + j + 1); k++) {
                    db->voteGenre("u" + to_string(k), t, 0);
                }

                // Vote in a decreasing order
                db->voteGenre("user", t, 2 * i + genres.size() - j);
                j++;
            }

            if (i != num_sessions - 1) {
                store->createSession();
            }
        }

        return Status::OK();
    }
}
}
