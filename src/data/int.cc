#include "int.h"
#include <cstring>

namespace data {

ResultV<int> ReadInt(const std::vector<uint8_t> &bytes, const int offset) {
    if (offset < 0 || offset + kIntBytesize > bytes.size())
        return Error("data::ReadInt() offset should be fit the size.");
    int read_value = 0;
    std::memcpy(&read_value, &(bytes[offset]), kIntBytesize);
    return Ok(read_value);
}

Result WriteInt(std::vector<uint8_t> &bytes, const int offset,
                const int value) {
    if (offset < 0 || offset + kIntBytesize > bytes.size())
        return Error("data::WriteInt() offset should be fit the size.");
    std::memcpy(&(bytes[offset]), &value, kIntBytesize);
    return Ok();
}

void WriteIntNoFail(std::vector<uint8_t> &bytes, const size_t offset,
                    const int value) {
    if (offset + kIntBytesize > bytes.size())
        bytes.resize(offset + kIntBytesize);
    WriteInt(bytes, offset, value);
}

void TypeInt::WriteTypeParameter(std::vector<uint8_t> &bytes,
                                 const size_t offset) const {
    if (offset + 1 > bytes.size()) bytes.resize(offset + 1);
    bytes[offset] = kTypeParameterInt;
}

Result Int::Write(std::vector<uint8_t> &bytes, const size_t offset) const {
    return WriteInt(bytes, offset, value_);
}

void Int::WriteNoFail(std::vector<uint8_t> &bytes, const size_t offset) const {
    return WriteIntNoFail(bytes, offset, value_);
}

ResultV<std::unique_ptr<DataItem>>
ReadDataInt(const std::vector<uint8_t> &data_bytes, int data_offset) {
    ResultV<int> int_result = ReadInt(data_bytes, data_offset + 1);
    if (int_result.IsError()) {
        return int_result +
               Error("data::ReadDataInt() failed to read int data.");
    }
    return ResultV<std::unique_ptr<DataItem>>(
        std::move(std::make_unique<Int>(int_result.Get())));
}

} // namespace data