#include "byte.h"
#include <gtest/gtest.h>

TEST(DataByte, CorrectlyReadByte) {
    std::vector<uint8_t> bytes = {'\0', 'V', '\0', '\0', '\0'};

    auto expect_Byte = data::ReadByte(bytes, 1);
    EXPECT_TRUE(expect_Byte.IsOk());
    EXPECT_EQ(expect_Byte.Get(), /*0b01010110=*/86);

    expect_Byte = data::ReadByte(bytes, 0);
    EXPECT_TRUE(expect_Byte.IsOk());
    EXPECT_EQ(expect_Byte.Get(), 0);
}

TEST(DataByte, ReadByteWithOutsideIndex) {
    std::vector<uint8_t> bytes = {'\0', 'V', '\0', '\0', '\0'};

    EXPECT_TRUE(data::ReadByte(bytes, -1).IsError());
    EXPECT_TRUE(data::ReadByte(bytes, 5).IsError());
}

TEST(DataByte, CorrectlyWriteByte) {
    std::vector<uint8_t> bytes(10);

    EXPECT_TRUE(data::WriteByte(bytes, 5, 0x93).IsOk());

    auto expect_Byte = data::ReadByte(bytes, 5);
    ASSERT_TRUE(expect_Byte.IsOk());
    EXPECT_EQ(expect_Byte.Get(), 0x93);
}

TEST(DataByte, WriteByteWithOutsideIndex) {
    std::vector<uint8_t> bytes(10);
    EXPECT_TRUE(data::WriteByte(bytes, -1, 0).IsError());
    EXPECT_TRUE(data::WriteByte(bytes, 10, 0).IsError());
}

TEST(DataByte, WriteByteNoFailSuccess) {
    std::vector<uint8_t> bytes(20);

    data::WriteByteNoFail(bytes, 4, 0x3c);

    auto expect_Byte = data::ReadByte(bytes, 4);
    ASSERT_TRUE(expect_Byte.IsOk());
    EXPECT_EQ(expect_Byte.Get(), 0x3c);
}

TEST(DataByte, WriteByteNoFailSuccessWithOutsideIndex) {
    std::vector<uint8_t> bytes(20);
    data::WriteByteNoFail(bytes, 28, 0xff);

    auto expect_Byte = data::ReadByte(bytes, 28);
    ASSERT_TRUE(expect_Byte.IsOk());
    EXPECT_EQ(expect_Byte.Get(), 0xff);
}

TEST(DataByte, ToByte) {
    data::DataItem x;
    x = data::Byte(0x3c);
    EXPECT_EQ(x[0], 0x3c);
}

TEST(DataByte, ReadFromDataItem) {
    data::DataItem x;
    x = data::Byte(0x3c);

    uint8_t expect_byte = data::ReadByte(x);
    EXPECT_EQ(expect_byte, 0x3c);
}