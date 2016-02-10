#include "skrillex/mapper.hpp"

#include <iostream>
#include <memory>
#include <boost/algorithm/string.hpp>

#include "store/store.hpp"
#include "mapper/transforms.hpp"

using namespace std;
using namespace boost::algorithm;
using namespace skrillex::internal;

namespace skrillex {
    Mapper::Mapper(shared_ptr<DB> db) : db_(db) {}
    Mapper::~Mapper() {}

    Status Mapper::map(Song& result, string songName, string artistName, string genreName) {
        Status s = Status::OK();
        string nGenre  = "";
        string nArtist = "";

        trim(songName);
        trim(artistName);
        trim(genreName);

        if (songName == "" && artistName != "" && genreName != "") {
            return Status::Error("Invalid operation: Can not map <artist, genre>");
        }

        if (genreName != "") {
            nGenre = normalize(FieldType::GenreField, genreName);

            // Perform genre lookup.
            s = db_->store_->getNormalized(result, nGenre);

            // No mapping was found, so insert the new genre and link.
            if (s.notFound()) {
                // Insert genre
                result.genre.name = genreName;
                s = db_->addGenre(result.genre);
                if (s != Status::OK()) {
                    return s;
                }

                // Insert link
                s = db_->store_->insertNormalized(nGenre, 0, 0, result.genre.id);
                if (s != Status::OK()) {
                    return s;
                }
            } else if (s != Status::OK()) {
                return s;
            }
        }

        if (artistName != "") {
            nArtist = normalize(FieldType::ArtistField, artistName);

            // Perform artist lookup.
            s = db_->store_->getNormalized(result, nArtist);

            // No mapping was found, so insert thew new artist and link.
            if (s.notFound()) {
                // Insert artist
                result.artist.name = artistName;
                s = db_->addArtist(result.artist);
                if (s != Status::OK()) {
                    return s;
                }

                // Insert link
                s = db_->store_->insertNormalized(nArtist, 0, result.artist.id, 0);
                if (s != Status::OK()) {
                    return s;
                }
            } else if (s != Status::OK()) {
                return s;
            }
        }

        if (songName != "") {
            string nSong = combine(songName, artistName);

            // Song is special case in the fact that a genre
            // can be linked to a song after the fact (this is
            // _not_ true for artist. I will leave it up to the
            // reader to figure out why). Therefore, if a genre is
            // specified, and the normalized entry is _not_ linked
            // to a genre, we want to link it. This is the motivation
            // behind the third if statement.

            // Perform song lookup.
            s = db_->store_->getNormalized(result, nSong);

            // No mapping was found, so insert the new song and link.
            if (s.notFound()) {
                // Insert song
                result.name = songName;
                s = db_->addSong(result);
                if (s != Status::OK()) {
                    return s;
                }

                // Insert link
                s = db_->store_->insertNormalized(nSong, result.id, result.artist.id, result.genre.id);
                if (s != Status::OK()) {
                    return s;
                }
            } else if (s != Status::OK()) {
                return s;
            } else if (genreName != "") {
                // We need to view the normalized entry without our
                // genre entry contaminating, to see if we need to link.
                Song test;
                s = db_->store_->getNormalized(test, nSong);
                if (s != Status::OK()) {
                    return s;
                }

                // No previous genre, let's link.
                if (test.genre.id == 0) {
                    s = db_->store_->insertNormalized(nSong, result.id, result.artist.id, result.genre.id);
                    if (s != Status::OK()) {
                        return s;
                    }
                }
            }
        }

        return Status::OK();
    }

    Status Mapper::lookup(Song& result, std::string songName, std::string artistName) {
        trim(songName);
        trim(artistName);

        if (songName == "") {
            return Status::Error("Invalid Operation: Must speciy song name when performing lookup");
        }

        string nSong = combine(songName, artistName);
        return db_->store_->getNormalized(result, nSong);
    }
}
