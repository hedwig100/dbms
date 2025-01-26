#include "bytes.h"

namespace data {

void WriteBytesWithOffsetNoFail(std::vector<uint8_t> &bytes,
                                const size_t offset,
                                const std::vector<uint8_t> &value,
                                int value_offset) {
    if (offset + value.size() - value_offset > bytes.size())
        bytes.resize(offset + value.size() - value_offset);
    std::copy(value.begin() + value_offset, value.end(),
              bytes.begin() + offset);
}

} // namespace data