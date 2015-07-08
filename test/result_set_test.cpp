#include <iostream>
#include <gtest/gtest.h>

#include "skrillex/result_set.hpp"

using namespace skrillex;

TEST(ResultSetTests, Basic) {
    ResultSet<int> songs;

    int base = 0;
    for (auto it = songs.begin(); it != songs.end(); it++) {
        ASSERT_EQ(base++, *it);
    }
}
