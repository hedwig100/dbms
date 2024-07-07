#include "sample_lib.h"
#include <gtest/gtest.h>

TEST(HelloWorldWithValue, CorrectlyReturnsValue) {
    EXPECT_EQ(sample_lib::HelloWorldWithValue(5), 5);
}