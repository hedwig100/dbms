#include "data/data.h"
#include "data/int.h"
#include <gtest/gtest.h>

TEST(Data, Equal) {
    data::DataItem item1;
    data::DataItem item2;
    item1.resize(4);
    item2.resize(4);
    item1[0] = 'a';
    item1[1] = 'b';
    item1[2] = 'c';
    item1[3] = 'd';
    item2[0] = 'a';
    item2[1] = 'b';
    item2[2] = 'c';
    item2[3] = 'd';

    EXPECT_TRUE(item1 == item2);
}

TEST(Data, NotEqual) {
    data::DataItem item1;
    data::DataItem item2;
    item1.resize(4);
    item2.resize(4);
    item1[0] = 'a';
    item1[1] = 'b';
    item1[2] = 'c';
    item1[3] = 'd';

    item2[0] = 'a';
    item2[1] = 'b';
    item2[2] = 'c';
    item2[3] = 'e';

    EXPECT_TRUE(item1 != item2);
}

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

TEST(DataItemWithType, EqualSucess) {
    data::DataItem item1 = data::Int(1234);
    data::DataItem item2 = data::Int(1234);
    data::DataItem item3 = data::Int(5678);

    data::DataItemWithType item_with_type1(item1, data::kTypeInt);
    data::DataItemWithType item_with_type2(item2, data::kTypeInt);
    data::DataItemWithType item_with_type3(item3, data::kTypeInt);

    EXPECT_TRUE(item_with_type1 == item_with_type2);
    EXPECT_TRUE(item_with_type1 != item_with_type3);
}