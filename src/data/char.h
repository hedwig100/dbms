#ifndef _DATATYPE_CHAR_H
#define _DATATYPE_CHAR_H

#include "data/data.h"
#include "result.h"
#include <cstring>
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

    Result Read(DataItem &item, const std::vector<uint8_t> &bytes,
                const size_t offset) const;

    Result Write(const DataItem &item, std::vector<uint8_t> &bytes,
                 const size_t offset) const;

  private:
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

inline DataItem String(const std::string &value) {
    DataItem item(value.size());
    std::memcpy(item.begin(), value.data(), value.size());
    return item;
}

inline ResultV<std::string> ReadString(const data::DataItem &item,
                                       const int length) {
    if (item.size() < length)
        return Error("data::Char() item size should be larger than length.");
    std::string value;
    value.resize(length);
    std::memcpy(&value[0], item.begin(), length);
    return Ok(value);
}

// Trims the trailing spaces of the string.
void RightTrim(std::string &value);

} // namespace data

#endif // _DATATYPE_CHAR_H