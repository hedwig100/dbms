#ifndef _DATA_INT_H
#define _DATA_INT_H

#include "data/data.h"
#include "result.h"
#include <vector>

namespace data {

using namespace result;

// Int: 32-bit integer
class Int : public DataItem {
  public:
    explicit inline Int(int value) : value_(value) {}

    inline DataType Type() const { return DataType::kInt; }

    inline void WriteTypeParameter(std::vector<uint8_t> &bytes,
                                   const size_t offset) const {}

    void Write(std::vector<uint8_t> &bytes, const size_t offset) const;

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
