#include "data/char.h"
#include "data/int.h"
#include "schema.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::result;

TEST(Layout, ComputeOffset) {
    std::vector<schema::Field> fields = {
        schema::Field("a", data::kTypeInt),
        schema::Field("b", data::TypeChar(7)),
        schema::Field("c", data::kTypeInt),
    };
    schema::Schema schema(fields);
    schema::Layout layout(schema);

    ResultV<int> offset_a = layout.Offset("a");
    EXPECT_TRUE(offset_a.IsOk());
    EXPECT_EQ(offset_a.Get(), 1);

    ResultV<int> offset_b = layout.Offset("b");
    EXPECT_TRUE(offset_b.IsOk());
    EXPECT_EQ(offset_b.Get(), 5);

    ResultV<int> offset_c = layout.Offset("c");
    EXPECT_TRUE(offset_c.IsOk());
    EXPECT_EQ(offset_c.Get(), 12);

    ResultV<int> length_a = layout.Length("a");
    EXPECT_TRUE(length_a.IsOk());
    EXPECT_EQ(length_a.Get(), 4);

    ResultV<int> length_b = layout.Length("b");
    EXPECT_TRUE(length_b.IsOk());
    EXPECT_EQ(length_b.Get(), 7);

    ResultV<int> length_c = layout.Length("c");
    EXPECT_TRUE(length_c.IsOk());
    EXPECT_EQ(length_c.Get(), 4);

    EXPECT_EQ(layout.Length(), 16);
}

TEST(Layout, OffsetFailWithInvalidField) {
    std::vector<schema::Field> fields = {
        schema::Field("a", data::kTypeInt),
        schema::Field("b", data::TypeChar(7)),
        schema::Field("c", data::kTypeInt),
    };
    schema::Schema schema(fields);
    schema::Layout layout(schema);

    ResultV<int> offset = layout.Offset("invalid_field");
    EXPECT_TRUE(offset.IsError());
}

TEST(Layout, FieldNames) {
    std::vector<schema::Field> fields = {
        schema::Field("a", data::kTypeInt),
        schema::Field("b", data::TypeChar(7)),
        schema::Field("c", data::kTypeInt),
    };
    schema::Schema schema(fields);
    schema::Layout layout(schema);

    EXPECT_THAT(layout.FieldNames(), ::testing::ElementsAre("a", "b", "c"));
}

TEST(Layout, FieldNamesWithSecondConstructor) {
    std::unordered_map<std::string, int> field_lengths = {
        {"a", 4},
        {"b", 7},
        {"c", 4},
    };
    std::unordered_map<std::string, data::BaseDataType> field_types = {
        {"a", data::BaseDataType::kInt},
        {"b", data::BaseDataType::kChar},
        {"c", data::BaseDataType::kInt},
    };
    std::unordered_map<std::string, int> offsets = {
        {"a", 1},
        {"b", 5},
        {"c", 12},
    };
    schema::Layout layout(16, field_types, field_lengths, offsets);

    EXPECT_THAT(layout.FieldNames(), ::testing::ElementsAre("a", "b", "c"));
}