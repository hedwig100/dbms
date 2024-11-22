#include "char.h"
#include "uint32.h"
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

void TypeChar::WriteTypeParameter(std::vector<uint8_t> &bytes,
                                  const size_t offset) const {
    if (offset + 2 > bytes.size()) bytes.resize(offset + 2);
    bytes[offset]     = kTypeParameterChar;
    bytes[offset + 1] = length_;
}

Char::Char(const std::string &value, uint8_t length) : type_(length) {
    value_ = value.substr(0, length);
    if (value_.size() < length) value_.resize(length, ' ');
}

Result Char::Write(std::vector<uint8_t> &bytes, const size_t offset) const {
    return WriteString(bytes, offset, type_.Length(), value_);
}

void Char::WriteNoFail(std::vector<uint8_t> &bytes, const size_t offset) const {
    WriteStringNoFail(bytes, offset, value_);
}

ResultV<std::unique_ptr<DataType>>
ReadTypeChar(const std::vector<uint8_t> &type_bytes, const int type_offset) {
    if (type_offset + 1 >= type_bytes.size())
        return Error("data::ReadTypeChar() type parameter is too short.");
    uint8_t length = type_bytes[type_offset + 1];
    return ResultV<std::unique_ptr<DataType>>(
        std::move(std::make_unique<TypeChar>(length)));
}

ResultV<std::unique_ptr<DataItem>>
ReadDataChar(const std::vector<uint8_t> &data_bytes, const int data_offset,
             const int length) {
    ResultV<std::string> char_result =
        ReadString(data_bytes, data_offset, length);
    if (char_result.IsError())
        return char_result +
               Error("data::ReadDataChar() failed to read string from bytes.");
    return ResultV<std::unique_ptr<DataItem>>(
        std::move(std::make_unique<Char>(char_result.Get(), length)));
}
} // namespace data