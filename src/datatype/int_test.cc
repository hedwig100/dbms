#include "int.h"
#include <gtest/gtest.h>

TEST(DatatypeInt, CorrectlyReadInt) {
    std::vector<uint8_t> bytes = {'\0', 'V', '\0', '\0', '\0'};

    auto expect_int = datatype::ReadInt(bytes, 1);
    EXPECT_TRUE(expect_int.IsOk());
    EXPECT_EQ(expect_int.Get(), /*0b01010110=*/86);

    expect_int = datatype::ReadInt(bytes, 0);
    EXPECT_TRUE(expect_int.IsOk());
    EXPECT_EQ(expect_int.Get(), /*0b0101011000000000=*/86 << 8);
}

TEST(DatatypeInt, ReadIntWithOutsideIndex) {
    std::vector<uint8_t> bytes = {'\0', 'V', '\0', '\0', '\0'};

    EXPECT_TRUE(datatype::ReadInt(bytes, -1).IsError());
    EXPECT_TRUE(datatype::ReadInt(bytes, 2).IsError());
}

TEST(DatatypeInt, CorrectlyWriteInt) {
    std::vector<uint8_t> bytes(10);

    EXPECT_TRUE(datatype::WriteInt(bytes, 5, 1313109832).IsOk());

    auto expect_int = datatype::ReadInt(bytes, 5);
    ASSERT_TRUE(expect_int.IsOk());
    EXPECT_EQ(expect_int.Get(), 1313109832);
}

TEST(DatatypeInt, WriteIntWithOutsideIndex) {
    std::vector<uint8_t> bytes(10);
    EXPECT_TRUE(datatype::WriteInt(bytes, -1, 0).IsError());
    EXPECT_TRUE(datatype::WriteInt(bytes, 7, 0).IsError());
    EXPECT_TRUE(datatype::WriteInt(bytes, 9, 0).IsError());
}