#include "data_read.h"
#include <gtest/gtest.h>

TEST(DataRead, ReadDataReadIntData) {
    std::vector<uint8_t> data_bytes = {0, '\x4', '\x7', 0, 0};
    auto data_result                = data::ReadData(data_bytes, 0);
    EXPECT_TRUE(data_result.IsOk());
    EXPECT_EQ(data_result.Get()->Type(), data::DataType::kInt);
}

TEST(DataRead, ReadDataReadCharData) {
    std::vector<uint8_t> data_bytes = {1, 4, '\x4', '\x7', 0, 0};
    auto data_result                = data::ReadData(data_bytes, 0);
    EXPECT_TRUE(data_result.IsOk());
    EXPECT_EQ(data_result.Get()->Type(), data::DataType::kChar);
}