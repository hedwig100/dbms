#include "uint32.h"
#include <cstring>

namespace data {

constexpr int kUint32Bytesize = 4;

ResultV<uint32_t> ReadUint32(const std::vector<uint8_t> &bytes,
                             const int offset) {
    if (offset < 0 || offset + kUint32Bytesize > bytes.size())
        return Error("data::ReadUint32() offset should be fit the size.");
    uint32_t read_value = 0;
    std::memcpy(&read_value, &(bytes[offset]), kUint32Bytesize);
    return Ok(read_value);
}

Result WriteUint32(std::vector<uint8_t> &bytes, const int offset,
                   const uint32_t value) {
    if (offset < 0 || offset + kUint32Bytesize > bytes.size())
        return Error("data::WriteUint32() offset should be fit the size.");
    std::memcpy(&(bytes[offset]), &value, kUint32Bytesize);
    return Ok();
}

void WriteUint32NoFail(std::vector<uint8_t> &bytes, const size_t offset,
                       const uint32_t value) {
    if (offset + kUint32Bytesize > bytes.size())
        bytes.resize(offset + kUint32Bytesize);
    WriteUint32(bytes, offset, value);
}

} // namespace data