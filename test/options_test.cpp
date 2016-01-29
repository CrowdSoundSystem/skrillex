#include <gtest/gtest.h>

#include "skrillex/options.hpp"

using namespace skrillex;

TEST(OptionsTest, Options) {
    Options o;

    EXPECT_FALSE(o.create_if_missing);
    EXPECT_TRUE(o.enable_caching);
    EXPECT_EQ(0, o.session_id);
}

TEST(OptionsTest, ReadOptions) {
    ReadOptions o;

    EXPECT_EQ(0, o.session_id);
    EXPECT_EQ(0, o.result_limit);
    EXPECT_EQ(SortType::Counts, o.sort);
}

TEST(OptionsTest, WriteOptions) {
    WriteOptions o;

    EXPECT_EQ(0, o.session_id);
}

