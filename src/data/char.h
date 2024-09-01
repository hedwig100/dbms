#ifndef _DATATYPE_CHAR_H
#define _DATATYPE_CHAR_H

#include "data/data.h"
#include "result.h"
#include <string>
#include <vector>

namespace data {

using namespace result;

// Char: fixed-length string. Its length is from 0 to 255 (represented as
// an unsigned 8-bit integer).
class Char : public DataItem {
  public:
    explicit Char(const std::string &value, uint8_t length);

    void WriteTypeParameter(std::vector<uint8_t> &bytes,
                            const size_t offset) const;

    inline DataType Type() const { return DataType::kChar; }

    void Write(std::vector<uint8_t> &bytes, const size_t offset) const;

    // Returns a string owned.
    inline std::string Value() const { return value_; }

    // Returns the type parameter i.e. length.
    inline uint8_t Length() const { return length_; }

  private:
    std::string value_;
    uint8_t length_;
};

// Reads the string of length `length` with the `offset` of `bytes`.
ResultV<std::string> ReadString(const std::vector<uint8_t> &bytes,
                                const int offset, const int length);

// Writes the string `value` with the `offset` of `bytes`. If `value` is smaller
// than `length`, the remaining space is intact.
Result WriteString(std::vector<uint8_t> &bytes, const int offset,
                   const int length, const std::string &value);

// Writes the string `value` with the `offset` of `bytes`. Unlike WriteString,
// this functions extends `bytes` when `value` does not fit `bytes`.
void WriteStringNoFail(std::vector<uint8_t> &bytes, const size_t offset,
                       const std::string &value);

} // namespace data

#endif // _DATATYPE_CHAR_H