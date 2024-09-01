#include "uint32.h"
#include <gtest/gtest.h>

TEST(DataUint32, CorrectlyReadUint32) {
    std::vector<uint8_t> bytes = {'\0', 'V', '\0', '\0', '\0'};

    auto expect_uint32 = data::ReadUint32(bytes, 1);
    EXPECT_TRUE(expect_uint32.IsOk());
    EXPECT_EQ(expect_uint32.Get(), /*0b01010110=*/86);

    expect_uint32 = data::ReadUint32(bytes, 0);
    EXPECT_TRUE(expect_uint32.IsOk());
    EXPECT_EQ(expect_uint32.Get(), /*0b0101011000000000=*/86 << 8);
}

TEST(DataUint32, ReadUint32WithOutsideIndex) {
    std::vector<uint8_t> bytes = {'\0', 'V', '\0', '\0', '\0'};

    EXPECT_TRUE(data::ReadUint32(bytes, -1).IsError());
    EXPECT_TRUE(data::ReadUint32(bytes, 2).IsError());
}

TEST(DataUint32, CorrectlyWriteUint32) {
    std::vector<uint8_t> bytes(10);

    EXPECT_TRUE(data::WriteUint32(bytes, 5, 1313109832).IsOk());

    auto expect_uint32 = data::ReadUint32(bytes, 5);
    ASSERT_TRUE(expect_uint32.IsOk());
    EXPECT_EQ(expect_uint32.Get(), 1313109832);
}

TEST(DataUint32, WriteUint32WithOutsideIndex) {
    std::vector<uint8_t> bytes(10);
    EXPECT_TRUE(data::WriteUint32(bytes, -1, 0).IsError());
    EXPECT_TRUE(data::WriteUint32(bytes, 7, 0).IsError());
    EXPECT_TRUE(data::WriteUint32(bytes, 9, 0).IsError());
}

TEST(DataUint32, WriteUint32NoFailSuccess) {
    std::vector<uint8_t> bytes(20);

    data::WriteUint32NoFail(bytes, 4, 32);

    auto expect_uint32 = data::ReadUint32(bytes, 4);
    ASSERT_TRUE(expect_uint32.IsOk());
    EXPECT_EQ(expect_uint32.Get(), 32);
}

TEST(DataUint32, WriteUint32NoFailWithOutsideIndexSuccess) {
    std::vector<uint8_t> bytes(20);

    data::WriteUint32NoFail(bytes, 25, 1010101);

    auto expect_uint32 = data::ReadUint32(bytes, 25);
    ASSERT_TRUE(expect_uint32.IsOk());
    EXPECT_EQ(expect_uint32.Get(), 1010101);
}