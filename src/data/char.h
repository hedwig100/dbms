#ifndef _DATATYPE_CHAR_H
#define _DATATYPE_CHAR_H

#include "data/data.h"
#include "result.h"
#include <memory>
#include <string>
#include <vector>

namespace data {

using namespace result;

// TypeChar represents the type of Char. It has a length parameter.
class TypeChar : public DataType {
  public:
    inline TypeChar(uint8_t length) : length_(length) {}

    inline BaseDataType BaseType() const { return BaseDataType::kChar; }

    inline int ValueLength() const { return length_; }

    // Returns the length parameter.
    inline uint8_t Length() const { return length_; }

  private:
    uint8_t length_;
};

// Char: fixed-length string. Its length is from 0 to 255 (represented as
// an unsigned 8-bit integer).
class Char : public DataItem {
  public:
    explicit Char(const std::string &value, uint8_t length);

    inline const TypeChar &Type() const { return type_; }

    Result Write(std::vector<uint8_t> &bytes, const size_t offset) const;

    void WriteNoFail(std::vector<uint8_t> &bytes, const size_t offset) const;

    // Returns a string owned.
    inline std::string Value() const { return value_; }

  private:
    std::string value_;
    TypeChar type_;
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