#include <gtest/gtest.h>

#include "skrillex/status.hpp"

using namespace skrillex;

TEST(StatusTest, OK) {
    Status empty;
    Status ok = Status::OK();

    EXPECT_TRUE(empty.ok());
    EXPECT_FALSE(empty.notFound());
    EXPECT_FALSE(empty.error());

    EXPECT_TRUE(ok.ok());
    EXPECT_FALSE(ok.notFound());
    EXPECT_FALSE(ok.error());

    EXPECT_EQ("", empty.message());
    EXPECT_EQ("", ok.message());
    EXPECT_EQ("Status: OK", empty.string());
    EXPECT_EQ("Status: OK", ok.string());
}

TEST(StatusTest, NotFound) {
    Status notFound = Status::NotFound("abc");

    EXPECT_FALSE(notFound.ok());
    EXPECT_TRUE(notFound.notFound());
    EXPECT_FALSE(notFound.error());

    EXPECT_EQ("abc", notFound.message());
    EXPECT_EQ("Status: NotFound - abc", notFound.string());
}

TEST(StatusTest, Error) {
    Status error = Status::Error("def");

    EXPECT_FALSE(error.ok());
    EXPECT_FALSE(error.notFound());
    EXPECT_TRUE(error.error());

    EXPECT_EQ("def", error.message());
    EXPECT_EQ("Status: Error - def", error.string());
}
