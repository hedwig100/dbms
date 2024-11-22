#include "data_read.h"
#include <gtest/gtest.h>

using namespace result;

TEST(DataRead, ReadDataReadIntData) {
    std::vector<uint8_t> data_bytes = {0, '\x4', '\x7', 0, 0};
    ResultV<std::unique_ptr<data::DataType>> type_result =
        data::ReadType(data_bytes, 0);
    EXPECT_TRUE(type_result.IsOk());

    auto data_result = data::ReadData(*type_result.Get(), data_bytes, 1);
    EXPECT_TRUE(data_result.IsOk());
    EXPECT_EQ(data_result.Get()->Type().BaseType(), data::BaseDataType::kInt);
}

TEST(DataRead, ReadDataReadCharData) {
    std::vector<uint8_t> data_bytes = {1, 4, '\x4', '\x7', 0, 0};
    ResultV<std::unique_ptr<data::DataType>> type_result =
        data::ReadType(data_bytes, 0);
    EXPECT_TRUE(type_result.IsOk());

    auto data_result = data::ReadData(*type_result.Get(), data_bytes, 2);
    EXPECT_TRUE(data_result.IsOk());
    EXPECT_EQ(data_result.Get()->Type().BaseType(), data::BaseDataType::kChar);
}

TEST(DataRead, ReadTypeDataReadCharData) {
    std::vector<uint8_t> data_bytes = {1, 4, '\x4', '\x7', 0, 0};
    ResultV<std::unique_ptr<data::DataItem>> data_result =
        data::ReadTypeData(data_bytes, 0);
    EXPECT_TRUE(data_result.IsOk());
    EXPECT_EQ(data_result.Get()->Type().BaseType(), data::BaseDataType::kChar);
}