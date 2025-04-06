#ifndef _DATA_BYTE_H
#define _DATA_BYTE_H

#include "data/data.h"
#include "result.h"

namespace data {

using namespace result;

constexpr int kByteBytesize = 1;

// TypeByte represents the type of byte.
class TypeByte : public DataType {
  public:
    inline TypeByte() {}

    inline BaseDataType BaseType() const { return BaseDataType::kByte; }

    inline int ValueLength() const { return kByteBytesize; }

    Result Read(DataItem &item, const std::vector<uint8_t> &bytes,
                const size_t offset) const;

    Result Write(const DataItem &item, std::vector<uint8_t> &bytes,
                 const size_t offset) const;
};

const TypeByte kTypeByte;

// Reads a byte with the `offset` of `bytes`.
ResultV<uint8_t> ReadByte(const std::vector<uint8_t> &bytes, const int offset);

// Writes the byte `value` with the `offset` of `bytes`.
Result WriteByte(std::vector<uint8_t> &bytes, const int offset,
                 const uint8_t value);

// Writes the byte `value` with the `offset` of `bytes`. Unlike WriteByte, this
// functions extends `bytes` when `value` does not fit `bytes`.
void WriteByteNoFail(std::vector<uint8_t> &bytes, const size_t offset,
                     const uint8_t value);

inline DataItem Byte(const uint8_t value) {
    DataItem item;
    item.resize(kByteBytesize);
    item[0] = value;
    return item;
}

inline ResultV<uint8_t> ReadByte(const data::DataItem &byte) {
    if (byte.size() < kByteBytesize)
        return Error("data::Byte() byte size should be 1.");
    return Ok(byte[0]);
}

} // namespace data

#endif // _DATA_BYTE_H