#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "skrillex/mapper.hpp"
#include "mapper/transforms.hpp"

using namespace std;
using namespace skrillex;
using namespace skrillex::internal;

TEST(MapperTests, Transforms) {
    EXPECT_EQ("Gsupercoolg3nre", normalize(FieldType::GenreField, "SuPer-cOol    g3nre"));
    EXPECT_EQ("Asupercoolarteest3", normalize(FieldType::ArtistField, "SuPer-cOol    arteest3"));
    EXPECT_EQ("supercools0ng", normalize(FieldType::None, "SuPer-cOol    s0ng"));
    EXPECT_EQ("Sk4ynegayfish", combine("gayFISH", "K4yne"));
}

TEST(MapperTests, Mapper) {
    DB* raw = 0;
    ASSERT_EQ(Status::OK(), open(raw, "test.db", Options::TestOptions()));

    shared_ptr<DB> db(raw);
    Mapper mapper(db);

    Song song;
    ResultSet<Genre> genres;
    ResultSet<Artist> artists;

    EXPECT_EQ(Status::OK(), mapper.map(song, "", "", "  Genre"));
    EXPECT_EQ(1, song.genre.id);
    EXPECT_EQ("Genre", song.genre.name);

    song = Song();
    EXPECT_EQ(Status::OK(), mapper.map(song, "", "", " Genre"));
    EXPECT_EQ(1, song.genre.id);
    EXPECT_EQ("Genre", song.genre.name);
    EXPECT_EQ(Status::OK(), db->getGenres(genres));
    EXPECT_EQ(1, genres.size());

    song = Song();
    EXPECT_EQ(Status::OK(), mapper.map(song, "", "   Kanye", ""));
    EXPECT_EQ(1, song.artist.id);
    EXPECT_EQ("Kanye", song.artist.name);

    song = Song();
    EXPECT_EQ(Status::OK(), mapper.map(song, "", "   Kanye", ""));
    EXPECT_EQ(1, song.artist.id);
    EXPECT_EQ("Kanye", song.artist.name);
    EXPECT_EQ(Status::OK(), db->getArtists(artists));
    EXPECT_EQ(1, artists.size());

    // Trying to relate and artist with a genre is an invalid operation.
    EXPECT_TRUE(mapper.map(song, "", " Kanye", "Genre  ").error());

    // Map just artist and song. The genre is _not_ linked yet.
    song = Song();
    EXPECT_EQ(Status::OK(), mapper.map(song, "Gay Fish",  "Kanye", ""));
    EXPECT_EQ(1, song.artist.id);
    EXPECT_EQ(0, song.genre.id);
    EXPECT_EQ(1, song.id);
    EXPECT_EQ("Gay Fish", song.name);

    // Link the genre
    song = Song();
    EXPECT_EQ(Status::OK(), mapper.map(song, "Gay Fish",  "Kanye", "Genre"));
    EXPECT_EQ(1, song.artist.id);
    EXPECT_EQ(1, song.genre.id);
    EXPECT_EQ(1, song.id);
    EXPECT_EQ("Gay Fish", song.name);

    // Now the genre should be pulled in implicitly
    song = Song();
    EXPECT_EQ(Status::OK(), mapper.map(song, "Gay Fish",  "Kanye", ""));
    EXPECT_EQ(1, song.artist.id);
    EXPECT_EQ(1, song.genre.id);
    EXPECT_EQ(1, song.id);
    EXPECT_EQ("Gay Fish", song.name);

    // A 'dirty' entry should reap full benefit.
    song = Song();
    EXPECT_EQ(Status::OK(), mapper.map(song, "Kanye - Gay Fish",  "", ""));
    EXPECT_EQ(1, song.artist.id);
    EXPECT_EQ(1, song.genre.id);
    EXPECT_EQ(1, song.id);
    EXPECT_EQ("Gay Fish", song.name);

    // And we should be able to perform a lookup
    song = Song();
    EXPECT_EQ(Status::OK(), mapper.lookup(song, "Kanye - Gay Fish",  ""));
    EXPECT_EQ(1, song.artist.id);
    EXPECT_EQ(1, song.genre.id);
    EXPECT_EQ(1, song.id);
    EXPECT_EQ("Gay Fish", song.name);

    // We can grab only artists
    Artist artist;
    EXPECT_EQ(Status::OK(), mapper.lookup(artist, "Kanye"));
    EXPECT_EQ(1, artist.id);
    EXPECT_EQ("Kanye", artist.name);

    // But fail for other cases
    song = Song();
    EXPECT_TRUE(mapper.lookup(song, "", "Kanye").error());
    EXPECT_TRUE(mapper.lookup(song, "Romeo", "Taylor Fish").notFound());
}
