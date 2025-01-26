#ifndef _DATA_INT_H
#define _DATA_INT_H

#include "data/data.h"
#include "result.h"
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
};

const TypeInt kTypeInt;

// Int: 32-bit integer
class Int : public DataItem {
  public:
    explicit inline Int(int value) : value_(value) {}

    inline const TypeInt &Type() const { return kTypeInt; }

    Result Write(std::vector<uint8_t> &bytes, const size_t offset) const;

    void WriteNoFail(std::vector<uint8_t> &bytes, const size_t offset) const;

    // Returns an owned integer.
    inline int Value() const { return value_; }

  private:
    int value_;
};

// Reads the int with the `offset` of `bytes`. The value is read as
// little-endian.
ResultV<int> ReadInt(const std::vector<uint8_t> &bytes, const int offset);

// Writes the int `value` with the `offset` of `bytes`. The value is written as
// little-endian.
Result WriteInt(std::vector<uint8_t> &bytes, const int offset, const int value);

// Writes the int `value` with the `offset` of `bytes`. The value is written as
// little-endian. Unlike WriteInt, this functions extends `bytes` when `value`
// does not fit `bytes`.
void WriteIntNoFail(std::vector<uint8_t> &bytes, const size_t offset,
                    const int value);

} // namespace data

#endif // _DATA_INT_H
