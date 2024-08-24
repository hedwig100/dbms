#include "char.h"
#include <gtest/gtest.h>

TEST(DataChar, CorrectlyReadString) {
    std::vector<uint8_t> bytes = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'};

    auto expect_str = data::ReadString(bytes, /*offset=*/1, /*length=*/4);
    EXPECT_TRUE(expect_str.IsOk());
    EXPECT_EQ(expect_str.Get(), "bcde");

    expect_str = data::ReadString(bytes, /*offset=*/4, /*length=*/1);
    EXPECT_TRUE(expect_str.IsOk());
    EXPECT_EQ(expect_str.Get(), "e");
}

TEST(DataChar, ReadStringWithOutsideIndex) {
    std::vector<uint8_t> bytes = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'};

    EXPECT_TRUE(data::ReadString(bytes, /*offset=*/-1, /*length=*/3).IsError());
    EXPECT_TRUE(data::ReadString(bytes, /*offset=*/8, /*length=*/3).IsError());
}

TEST(DataChar, CorrectlyWriteString) {
    std::vector<uint8_t> bytes(20);

    EXPECT_TRUE(
        data::WriteString(bytes, /*offset=*/3, /*length=*/3, "abc").IsOk());

    auto expect_str = data::ReadString(bytes, /*offset=*/3, /*length=*/3);
    ASSERT_TRUE(expect_str.IsOk());
    EXPECT_EQ(expect_str.Get(), "abc");
    expect_str = data::ReadString(bytes, /*offset=*/5, /*length=*/1);
    ASSERT_TRUE(expect_str.IsOk());
    EXPECT_EQ(expect_str.Get(), "c");
}

TEST(DataChar, WriteStringWithOutsideIndex) {
    std::vector<uint8_t> bytes(20);

    EXPECT_TRUE(
        data::WriteString(bytes, /*offset=*/-1, /*length=*/0, "").IsError());
    EXPECT_TRUE(
        data::WriteString(bytes, /*offset=*/9, /*length=*/12, "aaaaaaaaaaaa")
            .IsError());
}