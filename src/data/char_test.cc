#include "char.h"
#include <gtest/gtest.h>

TEST(DataChar, DataItemCharTypeValueLength) {
    data::Char hello("hello", 5);

    EXPECT_EQ(hello.Type(), data::DataType::kChar);
    EXPECT_EQ(hello.Value(), "hello");
    EXPECT_EQ(hello.Length(), 5);
}

TEST(DataChar, DataItemLongCharTypeValueLength) {
    data::Char hello("hello world", 5);

    EXPECT_EQ(hello.Type(), data::DataType::kChar);
    EXPECT_EQ(hello.Value(), "hello");
    EXPECT_EQ(hello.Length(), 5);
}

TEST(DataChar, DataItemSmallCharTypeValueLength) {
    data::Char hello("hello", 6);

    EXPECT_EQ(hello.Type(), data::DataType::kChar);
    EXPECT_EQ(hello.Value(), "hello ");
    EXPECT_EQ(hello.Length(), 6);
}

TEST(DataChar, DataItemCharWriteTypeParameter) {
    data::Char hello("hello", 5);
    std::vector<uint8_t> bytes;

    hello.WriteTypeParameter(bytes, 0);
    EXPECT_TRUE(bytes.size() >= 2);
    EXPECT_EQ(bytes[1], 5);
}

TEST(DataChar, DataItemCharWriteOfSmallBytes) {
    const uint8_t length = 5;
    data::Char hello("hello", length);
    std::vector<uint8_t> bytes;
    const size_t offset = 3;

    hello.Write(bytes, offset);

    EXPECT_EQ(bytes.size(), offset + length);
    auto expect_char = data::ReadString(bytes, offset, length);
    EXPECT_TRUE(expect_char.IsOk());
    EXPECT_EQ(expect_char.Get(), "hello");
}

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

TEST(DataChar, WriteStringNoFailSuccess) {
    std::vector<uint8_t> bytes(20);
    data::WriteStringNoFail(bytes, 3, "abc");

    auto expect_str = data::ReadString(bytes, 3, 3);
    ASSERT_TRUE(expect_str.IsOk());
    EXPECT_EQ(expect_str.Get(), "abc");
}

TEST(DataChar, WriteStringNoFailWithOutsideIndexSuccess) {
    std::vector<uint8_t> bytes(3);
    data::WriteStringNoFail(bytes, 6, "abc");

    auto expect_str = data::ReadString(bytes, 6, 3);
    ASSERT_TRUE(expect_str.IsOk());
    EXPECT_EQ(expect_str.Get(), "abc");
}