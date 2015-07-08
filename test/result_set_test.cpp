#include <iostream>
#include <gtest/gtest.h>

#include "skrillex/result_set.hpp"
#include "mutator.hpp"

using namespace std;
using namespace skrillex;
using namespace skrillex::internal;

TEST(ResultSetTests, Basic) {
    ResultSet<int> songs;

    int& version = ResultSetMutator::getVersion<int>(songs);
    std::vector<int>& v = ResultSetMutator::getVector<int>(songs);

    EXPECT_EQ(0, version);
    EXPECT_TRUE(songs.empty());
    EXPECT_EQ(0, songs.size());

    version++;
    for (int i = 0; i < 10; i++) {
        v.push_back(i);
    }

    EXPECT_EQ(1, version);
    EXPECT_FALSE(songs.empty());
    EXPECT_EQ(10, songs.size());

    int i = 0;
    for (auto it = songs.begin(); it != songs.end(); it++) {
        ASSERT_EQ(i++, *it);
    }
}

