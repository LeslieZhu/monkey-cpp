#include <gtest/gtest.h>

#include <vector>
#include <memory>

#include "objects/objects.hpp"

TEST(TestStringHashKey, BasicAssertions)
{
    auto hello1 = objects::String("Hello World");
    auto hello2 = objects::String("Hello World");

    auto diff1 = objects::String("My name is Leslie");
    auto diff2 = objects::String("My name is Leslie");

    EXPECT_EQ(hello1.GetHashKey(), hello2.GetHashKey());
    EXPECT_EQ(diff1.GetHashKey(), diff2.GetHashKey());

    EXPECT_NE(hello1.GetHashKey(), diff1.GetHashKey());
}
