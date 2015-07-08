#include <iostream>
#include <gtest/gtest.h>

#include "skrillex/result_set.hpp"

using namespace skrillex;
using namespace skrillex::internal;

TEST(ResultSetTests, Basic) {
    ResultSet<int> songs;

    int i = 0;
    for (auto it = mbegin<int>(songs); it != songs.end(); it++) {
        *it = i++;
    }

    i = 0;
    for (auto it = songs.begin(); it != songs.end(); it++) {
        std::cout << "Comparing i = " << i << " with *it = " << *it << std::endl;
        ASSERT_EQ(i++, *it);
    }
}
