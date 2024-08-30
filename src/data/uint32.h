#ifndef _DATA_UINT32_H
#define _DATA_UINT32_H

#include "data/data.h"
#include "result.h"
#include <vector>

namespace data {

using namespace result;

// Reads uint32_t with the `offset`. The value is read as little-endian.
ResultV<uint32_t> ReadUint32(const std::vector<uint8_t> &bytes,
                             const int offset);

// Writes uint32_t `value` with the `offset`. The value is written as
// little-endian.
Result WriteUint32(std::vector<uint8_t> &bytes, const int offset,
                   const uint32_t value);

// Writes uint32_t `value` with the `offset`. The value is written as
// little-endian. Unlike WriteUint32, this functions extends `bytes` when
// `value` does not fit `bytes`.
void WriteUint32NoFail(std::vector<uint8_t> &bytes, const size_t offset,
                       const uint32_t value);

} // namespace data

#endif // _DATA_UINT32_H