#include "data/data.h"
#include <gtest/gtest.h>

TEST(DataCopy, ReadSuccess) {
    std::vector<uint8_t> bytes = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'};
    data::DataItem item;

    EXPECT_TRUE(data::Read(item, bytes, 1, 4).IsOk());
    EXPECT_EQ(item.size(), 4);
    EXPECT_EQ(item[0], 'b');
    EXPECT_EQ(item[1], 'c');
    EXPECT_EQ(item[2], 'd');
    EXPECT_EQ(item[3], 'e');
}

TEST(DataCopy, WriteSuccess) {
    std::vector<uint8_t> bytes = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'};
    std::vector<uint8_t> expect_bytes = {'a', 'b', 'c', 'd', 'e',
                                         'b', 'c', 'd', 'e'};
    data::DataItem item;
    item.resize(4);
    item[0] = 'b';
    item[1] = 'c';
    item[2] = 'd';
    item[3] = 'e';

    EXPECT_TRUE(data::Write(item, bytes, 5, 4).IsOk());
    EXPECT_EQ(bytes, expect_bytes);
}