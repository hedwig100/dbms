#ifndef _DATA_DATA_H
#define _DATA_DATA_H

#include "result.h"
#include <cstdint>
#include <vector>

namespace data {

using namespace ::result;

enum class BaseDataType {
    // 32-bit integer
    kInt = 0,

    // Fixed length strings
    kChar = 1,

    // Byte
    kByte = 2,
};

// DataType represents the type of data such as integer, char with additional
// parameters such as length of the string.
class DataType {
  public:
    // Type of the data such as integer, char...
    virtual BaseDataType BaseType() const = 0;

    // Byte length of the value.
    virtual int ValueLength() const = 0;
};

// DataItem represents one data item such as a integer, a char.
class DataItem {
  public:
    // Type of the item such as integer, char...
    virtual const DataType &Type() const = 0;

    // Writes the item to `bytes` with the `offset`. If the `bytes` is not large
    // enough, returns Error.
    virtual Result Write(std::vector<uint8_t> &bytes,
                         const size_t offset) const = 0;

    // Writes the item to `bytes` with the `offset`. If the `bytes` is not large
    // enough, the `bytes` is extended.
    virtual void WriteNoFail(std::vector<uint8_t> &bytes,
                             const size_t offset) const = 0;
};

} // namespace data

#endif // _DATA_DATA_H