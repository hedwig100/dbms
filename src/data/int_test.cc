#include "int.h"
#include <gtest/gtest.h>

TEST(DataInt, DataItemIntTypeValue) {
    data::Int two(2);

    EXPECT_EQ(two.Type().BaseType(), data::BaseDataType::kInt);
    EXPECT_EQ(two.Value(), 2);
}

TEST(DataInt, DataItemIntWriteOfSmallBytes) {
    data::Int two(2);
    std::vector<uint8_t> bytes;
    const size_t offset = 3;

    two.WriteNoFail(bytes, offset);

    EXPECT_EQ(bytes.size(), offset + 4);
    auto expect_int = data::ReadInt(bytes, offset);
    EXPECT_TRUE(expect_int.IsOk());
    EXPECT_EQ(expect_int.Get(), 2);
}

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

TEST(DataInt, CorrectlyWriteNegativeInteger) {
    std::vector<uint8_t> bytes(10);

    EXPECT_TRUE(data::WriteInt(bytes, 6, -132343).IsOk());

    auto expect_int = data::ReadInt(bytes, 6);
    ASSERT_TRUE(expect_int.IsOk());
    EXPECT_EQ(expect_int.Get(), -132343);
}

TEST(DataInt, WriteIntWithOutsideIndex) {
    std::vector<uint8_t> bytes(10);
    EXPECT_TRUE(data::WriteInt(bytes, -1, 0).IsError());
    EXPECT_TRUE(data::WriteInt(bytes, 7, 0).IsError());
    EXPECT_TRUE(data::WriteInt(bytes, 9, 0).IsError());
}

TEST(DataInt, WriteIntNoFailSuccess) {
    std::vector<uint8_t> bytes(20);

    data::WriteIntNoFail(bytes, 4, -20);

    auto expect_int = data::ReadInt(bytes, 4);
    ASSERT_TRUE(expect_int.IsOk());
    EXPECT_EQ(expect_int.Get(), -20);
}

TEST(DataInt, WriteIntNoFailSuccessWithOutsideIndex) {
    std::vector<uint8_t> bytes(20);
    data::WriteIntNoFail(bytes, 28, -3);

    auto expect_int = data::ReadInt(bytes, 28);
    ASSERT_TRUE(expect_int.IsOk());
    EXPECT_EQ(expect_int.Get(), -3);
}