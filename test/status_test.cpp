#include <gtest/gtest.h>

#include "skrillex/status.hpp"

using namespace skrillex;

TEST(StatusTest, OK) {
    Status empty;
    Status ok = Status::OK();

    EXPECT_TRUE(empty.ok());
    EXPECT_TRUE(ok.ok());
    EXPECT_EQ("", empty.message());
    EXPECT_EQ("", ok.message());
    EXPECT_EQ("Status: OK", empty.string());
    EXPECT_EQ("Status: OK", ok.string());
}

