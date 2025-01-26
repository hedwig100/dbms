#include "bytes.h"
#include <gtest/gtest.h>

TEST(WriteBytesWithOffsetNoFail, WriteBytesWithOffsetNoFail) {
    std::vector<uint8_t> bytes(10);
    const std::vector<uint8_t> value = {'a', 'b', 'c', 'd'};

    data::WriteBytesWithOffsetNoFail(bytes, 3, value, 1);

    EXPECT_EQ(bytes,
              std::vector<uint8_t>({0, 0, 0, 'b', 'c', 'd', 0, 0, 0, 0}));
}