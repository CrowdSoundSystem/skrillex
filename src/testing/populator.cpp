#include <string>

#include "skrillex/dbo.hpp"
#include "skrillex/testing/populator.hpp"
#include "store/store.hpp"
#include "mutator.hpp"

using namespace std;
using namespace skrillex::internal;

namespace skrillex {
namespace testing {
    struct PopulatorData {
        vector<Song>   songs;
        vector<Artist> artists;
        vector<Genre>  genre;
    };

    PopulatorData getPopulatorData(int num_songs, int num_artists, int num_genres) {

    }

    Status populateEmpty(DB* db, int num_songs, int num_artists, int num_genres) {
        if (!db) {
            return Status::Error("Cannot populate null database");
        }

        if (!db->isOpen()) {
            return Status::Error("Database not open.");
        }

        vector<Genre> genres;
        vector<Artist> artists;
        Status status = Status::OK();
        for (int i = 0; i < num_genres; i++) {
            Genre g;
            g.name = "g" + to_string(i);

            status = db->addGenre(g);
            if (status != Status::OK()) {
                return status;
            }

            genres.push_back(g);
        }

        for (int i = 0; i < num_artists; i++) {
            Artist a;
            a.name = "a" + to_string(i);

            status = db->addArtist(a);
            if (status != Status::OK()) {
                return status;
            }

            artists.push_back(a);
        }

        for (int i = 0; i < num_songs; i++) {
            Song s;
            s.id     = i;
            s.artist = artists[i % num_artists];
            s.genre  = genres[i % num_genres];
            s.name   = "s" + to_string(i);

            status = db->addSong(s);
            if (status != Status::OK()) {
                return status;
            }
        }

        return status;
    }

    Status populateFull(DB* db, int num_songs, int num_artists, int num_genres, int num_sessions) {
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
            for (auto& s : songs) {
                Song t = s;
                for (int j = 0; j < (i + 1); i++) {
                    db->countSong(t);
                }
            }
            for (auto& a : artists) {
                Artist t = a;
                for (int j = 0; j < (i + 1); j++) {
                    db->countArtist(t);
                }
            }
            for (auto& g : genres) {
                Genre t = g;
                for (int j = 0; j < (i + 1); j++) {
                    db->countGenre(t);
                }
            }

            if (i == num_sessions - 1) {
                store->createSession();
            }
        }

        return Status::OK();
    }
}
}
