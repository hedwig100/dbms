#ifndef _DATA_DATA_H
#define _DATA_DATA_H

#include "result.h"
#include <array>
#include <cstdint>
#include <iterator>
#include <memory>
#include <variant>
#include <vector>

namespace data {

using namespace ::result;

// DataItem represents all data. This class is a commmon class for all data
// types.
class DataItem {
  public:
    DataItem() {}

    DataItem(size_t length) { resize(length); }

    size_t size() const;

    void resize(size_t length);

    uint8_t *begin();

    const uint8_t *begin() const;

    uint8_t &operator[](size_t index) { return begin()[index]; }

    const uint8_t &operator[](size_t index) const { return begin()[index]; }

    uint8_t *end();

    const uint8_t *end() const;

    bool operator==(const DataItem &other) const;

    bool operator!=(const DataItem &other) const;

  private:
    constexpr static size_t kMaxItemLength = 4;
    using ConstLengthItem    = std::array<uint8_t, kMaxItemLength>;
    using VariableLengthItem = std::vector<uint8_t>;
    std::variant<ConstLengthItem, VariableLengthItem> item_;
};

enum class BaseDataType {
    // 32-bit integer
    kInt = 0,

    // Fixed length strings
    kChar = 1,

    // Byte
    kByte = 2,

    // Bytes
    kBytes = 3,
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

// Reads `item` from `bytes` with the `offset`. If the `bytes` is not large
// enough, returns Error.
Result Read(DataItem &item, const std::vector<uint8_t> &bytes,
            const size_t offset, int length);

// Writes `item` to `bytes` with the `offset`. If the `bytes` is not
// large enough, returns Error.
Result Write(const DataItem &item, std::vector<uint8_t> &bytes,
             const size_t offset, int length);

class DataItemWithType {
  public:
    DataItemWithType() {}

    DataItemWithType(const DataItem &data, const DataType &type)
        : item_(data), type_(type.BaseType()), length_(type.ValueLength()) {}

    DataItemWithType(const DataItem &data, BaseDataType type, int length)
        : item_(data), type_(type), length_(length) {}

    const DataItem &Item() const { return item_; }

    bool operator==(const DataItemWithType &other) const {
        return type_ == other.type_ && item_ == other.item_;
    }

    bool operator!=(const DataItemWithType &other) const {
        return !(*this == other);
    }

  private:
    DataItem item_;
    BaseDataType type_;
    int length_;
};

} // namespace data

#endif // _DATA_DATA_H