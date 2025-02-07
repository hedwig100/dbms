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
};

const TypeByte kTypeByte;

class Byte : public DataItem {
  public:
    explicit inline Byte(uint8_t value) : value_(value) {}

    inline const TypeByte &Type() const { return kTypeByte; }

    Result Write(std::vector<uint8_t> &bytes, const size_t offset) const;

    void WriteNoFail(std::vector<uint8_t> &bytes, const size_t offset) const;

    // Returns an owned integer.
    inline uint8_t Value() const { return value_; }

  private:
    uint8_t value_;
};

// Reads a byte with the `offset` of `bytes`.
ResultV<uint8_t> ReadByte(const std::vector<uint8_t> &bytes, const int offset);

// Writes the byte `value` with the `offset` of `bytes`.
Result WriteByte(std::vector<uint8_t> &bytes, const int offset,
                 const uint8_t value);

// Writes the byte `value` with the `offset` of `bytes`. Unlike WriteByte, this
// functions extends `bytes` when `value` does not fit `bytes`.
void WriteByteNoFail(std::vector<uint8_t> &bytes, const size_t offset,
                     const uint8_t value);

} // namespace data

#endif // _DATA_BYTE_H