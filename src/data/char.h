#ifndef _DATATYPE_CHAR_H
#define _DATATYPE_CHAR_H

#include "result.h"
#include <string>
#include <vector>

namespace data {

using namespace result;

// Reads the string of length `length` with the `offset` of `bytes`.
ResultV<std::string> ReadString(const std::vector<uint8_t> &bytes,
                                const int offset, const int length);

// Writes the string `value` with the `offset` of `bytes`. If `value` is smaller
// than `length`, the remaining space is intact.
Result WriteString(std::vector<uint8_t> &bytes, const int offset,
                   const int length, const std::string &value);

} // namespace data

#endif // _DATATYPE_CHAR_H