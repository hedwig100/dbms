#include "char.h"
#include "uint32.h"
#include <algorithm>
#include <cstring>
#include <memory>

namespace data {

ResultV<std::string> ReadString(const std::vector<uint8_t> &bytes,
                                const int offset, const int length) {
    if (offset < 0 || offset + length > bytes.size())
        return Error("data::ReadString() offset should be fit the size.");
    std::string read_value;
    read_value.resize(length);
    std::memcpy(&read_value[0], &bytes[offset], length);
    return Ok(read_value);
}

Result WriteString(std::vector<uint8_t> &bytes, const int offset,
                   const int length, const std::string &value) {
    if (offset < 0 || offset + length > bytes.size())
        return Error("data::WriteString() offset should be fit the size.");
    if (value.size() > length)
        return Error(
            "data::WriteString() value should be smaller than length.");
    std::memcpy(&bytes[offset], &value[0], value.size());
    return Ok();
}

void WriteStringNoFail(std::vector<uint8_t> &bytes, const size_t offset,
                       const std::string &value) {
    if (offset + value.size() > bytes.size())
        bytes.resize(offset + value.size());
    WriteString(bytes, offset, value.size(), value);
}

DataItem Char(const std::string &value, const int length) {
    DataItem item(length);
    std::memcpy(item.begin(), value.data(), value.size());
    return item;
}

std::string ReadChar(const data::DataItem &item, const int length) {
    std::string value(item.begin(), item.begin() + length);
    return value;
}

void RightTrim(std::string &value) {
    value.erase(std::find_if(value.rbegin(), value.rend(),
                             [](char ch) { return ch && !std::isspace(ch); })
                    .base(),
                value.end());
}

} // namespace data