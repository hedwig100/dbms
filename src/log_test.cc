#include "log.h"
#include <gtest/gtest.h>

TEST(BlockWriteAppend, InputBytesFitSuccess) {
    dblog::LogBlock block(16);
    std::vector<uint8_t> bytes = {'a', 'b', 'c', 'd', 'e',
                                  'f', 'g', 'h', 'i', 'j'};

    EXPECT_TRUE(block.WriteAppend(bytes.begin(), bytes.end()).IsOk());
    EXPECT_EQ(block.ReadByte(0).Get(), 'a');
    EXPECT_EQ(block.ReadByte(1).Get(), 'b');
    EXPECT_EQ(block.ReadByte(2).Get(), 'c');
    EXPECT_EQ(block.ReadByte(3).Get(), 'd');
    EXPECT_EQ(block.ReadByte(4).Get(), 'e');
    EXPECT_EQ(block.ReadByte(5).Get(), 'f');
    EXPECT_EQ(block.ReadByte(6).Get(), 'g');
    EXPECT_EQ(block.ReadByte(7).Get(), 'h');
    EXPECT_EQ(block.ReadByte(8).Get(), 'i');
    EXPECT_EQ(block.ReadByte(9).Get(), 'j');
}

TEST(BlockWriteAppend, InputBytesDontFitError) {
    dblog::LogBlock block(5);
    std::vector<uint8_t> bytes = {'a', 'b', 'c', 'd', 'e',
                                  'f', 'g', 'h', 'i', 'j'};

    const auto expect_error = block.WriteAppend(bytes.begin(), bytes.end());
    EXPECT_TRUE(expect_error.IsError());
    EXPECT_EQ(expect_error.Error(), 5);

    EXPECT_EQ(block.ReadByte(0).Get(), 'a');
    EXPECT_EQ(block.ReadByte(1).Get(), 'b');
    EXPECT_EQ(block.ReadByte(2).Get(), 'c');
    EXPECT_EQ(block.ReadByte(3).Get(), 'd');
    EXPECT_EQ(block.ReadByte(4).Get(), 'e');
}