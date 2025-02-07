#include "byte.h"
#include <cstring>

namespace data {

ResultV<uint8_t> ReadByte(const std::vector<uint8_t> &bytes, const int offset) {
    if (offset < 0 || offset + kByteBytesize > bytes.size())
        return Error("data::ReadByte() offset should be fit the size.");
    return Ok(bytes[offset]);
}

Result WriteByte(std::vector<uint8_t> &bytes, const int offset,
                 const uint8_t value) {
    if (offset < 0 || offset + kByteBytesize > bytes.size())
        return Error("data::WriteByte() offset should be fit the size.");
    bytes[offset] = value;
    return Ok();
}

void WriteByteNoFail(std::vector<uint8_t> &bytes, const size_t offset,
                     const uint8_t value) {
    if (offset + kByteBytesize > bytes.size())
        bytes.resize(offset + kByteBytesize);
    WriteByte(bytes, offset, value);
}

Result Byte::Write(std::vector<uint8_t> &bytes, const size_t offset) const {
    return WriteByte(bytes, offset, value_);
}

void Byte::WriteNoFail(std::vector<uint8_t> &bytes, const size_t offset) const {
    return WriteByteNoFail(bytes, offset, value_);
}

} // namespace data