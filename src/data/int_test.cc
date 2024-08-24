#include "int.h"
#include <gtest/gtest.h>

TEST(DataInt, CorrectlyReadInt) {
    std::vector<uint8_t> bytes = {'\0', 'V', '\0', '\0', '\0'};

    auto expect_int = data::ReadInt(bytes, 1);
    EXPECT_TRUE(expect_int.IsOk());
    EXPECT_EQ(expect_int.Get(), /*0b01010110=*/86);

    expect_int = data::ReadInt(bytes, 0);
    EXPECT_TRUE(expect_int.IsOk());
    EXPECT_EQ(expect_int.Get(), /*0b0101011000000000=*/86 << 8);
}

TEST(DataInt, ReadIntWithOutsideIndex) {
    std::vector<uint8_t> bytes = {'\0', 'V', '\0', '\0', '\0'};

    EXPECT_TRUE(data::ReadInt(bytes, -1).IsError());
    EXPECT_TRUE(data::ReadInt(bytes, 2).IsError());
}

TEST(DataInt, CorrectlyWriteInt) {
    std::vector<uint8_t> bytes(10);

    EXPECT_TRUE(data::WriteInt(bytes, 5, 1313109832).IsOk());

    auto expect_int = data::ReadInt(bytes, 5);
    ASSERT_TRUE(expect_int.IsOk());
    EXPECT_EQ(expect_int.Get(), 1313109832);
}

TEST(DataInt, WriteIntWithOutsideIndex) {
    std::vector<uint8_t> bytes(10);
    EXPECT_TRUE(data::WriteInt(bytes, -1, 0).IsError());
    EXPECT_TRUE(data::WriteInt(bytes, 7, 0).IsError());
    EXPECT_TRUE(data::WriteInt(bytes, 9, 0).IsError());
}