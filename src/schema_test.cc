#include "data/char.h"
#include "data/int.h"
#include "schema.h"
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
    EXPECT_EQ(layout.Length(), 16);
}