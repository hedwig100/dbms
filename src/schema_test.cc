#include "data/char.h"
#include "data/int.h"
#include "schema.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(Layout, ComputeOffset) {
    std::vector<schema::Field> fields = {
        schema::Field("a", data::kTypeInt),
        schema::Field("b", data::TypeChar(7)),
        schema::Field("c", data::kTypeInt),
    };
    schema::Schema schema(fields);
    schema::Layout layout(schema);

    EXPECT_EQ(layout.Offset("a"), 1);
    EXPECT_EQ(layout.Offset("b"), 5);
    EXPECT_EQ(layout.Offset("c"), 12);
    EXPECT_EQ(layout.Length("a"), 4);
    EXPECT_EQ(layout.Length("b"), 7);
    EXPECT_EQ(layout.Length("c"), 4);
    EXPECT_EQ(layout.Length(), 16);
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
    std::unordered_map<std::string, int> offsets = {
        {"a", 1},
        {"b", 5},
        {"c", 12},
    };
    schema::Layout layout(16, field_lengths, offsets);

    EXPECT_THAT(layout.FieldNames(), ::testing::ElementsAre("a", "b", "c"));
}