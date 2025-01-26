#ifndef _BYTES_H
#define _BYTES_H

#include <cstdint>
#include <string>
#include <vector>

namespace data {

// Writes the bytes `value`[`value_offset`:] with the `offset` of `bytes`.
// This functions extends `bytes` when `value` does not fit `bytes`.
void WriteBytesWithOffsetNoFail(std::vector<uint8_t> &bytes,
                                const size_t offset,
                                const std::vector<uint8_t> &value,
                                int value_offset);

} // namespace data

#endif // _BYTES_H