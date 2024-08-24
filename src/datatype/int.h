#ifndef _DATATYPE_INT_H
#define _DATATYPE_INT_H

#include "result.h"
#include <vector>

namespace datatype {

using namespace result;

// Reads the int with the `offset` of `bytes`. The value is read as
// little-endian.
ResultV<int> ReadInt(const std::vector<uint8_t> &bytes, const int offset);

// Writes the int `value` with the `offset` of `bytes`. The value is written as
// little-endian.
Result WriteInt(std::vector<uint8_t> &bytes, const int offset, const int value);

} // namespace datatype

#endif // _DATATYPE_INT_H
