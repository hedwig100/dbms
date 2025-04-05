#ifndef _DATA_INT_H
#define _DATA_INT_H

#include "data/data.h"
#include "result.h"
#include <cstring>
#include <memory>
#include <vector>

namespace data {

using namespace result;

constexpr int kIntBytesize = 4;

// TypeInt represents the type of Int.
class TypeInt : public DataType {
  public:
    inline TypeInt() {}

    inline BaseDataType BaseType() const { return BaseDataType::kInt; }

    inline int ValueLength() const { return kIntBytesize; }

    Result Read(DataItem &item, const std::vector<uint8_t> &bytes,
                const size_t offset) const;

    Result Write(const DataItem &item, std::vector<uint8_t> &bytes,
                 const size_t offset) const;
};

const TypeInt kTypeInt;

// Reads the int with the `offset` of `bytes`. The value is read as
// little-endian.
ResultV<int> ReadInt(const std::vector<uint8_t> &bytes, const int offset);

inline ResultV<int> ReadInt(const data::DataItem &item) {
    if (item.size() < kIntBytesize)
        return Error("data::ReadInt() item size should be larger than "
                     "kIntBytesize.");
    int value = 0;
    std::memcpy(&value, item.begin(), kIntBytesize);
    return Ok(value);
}

// Writes the int `value` with the `offset` of `bytes`. The value is written as
// little-endian.
Result WriteInt(std::vector<uint8_t> &bytes, const int offset, const int value);

// Writes the int `value` with the `offset` of `bytes`. The value is written as
// little-endian. Unlike WriteInt, this functions extends `bytes` when `value`
// does not fit `bytes`.
void WriteIntNoFail(std::vector<uint8_t> &bytes, const size_t offset,
                    const int value);

inline DataItem Int(const int value) {
    DataItem item(kIntBytesize);
    std::memcpy(item.begin(), &value, kIntBytesize);
    return item;
}

} // namespace data

#endif // _DATA_INT_H
