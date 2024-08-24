#include "char.h"
#include <cstring>

namespace data {

ResultV<std::string> ReadString(const std::vector<uint8_t> &bytes,
                                const int offset, const int length) {
    if (offset < 0 || offset + length > bytes.size())
        return Error("offset should be fit the size");
    std::string read_value;
    read_value.resize(length);
    std::memcpy(&read_value[0], &bytes[offset], length);
    return Ok(read_value);
}

Result WriteString(std::vector<uint8_t> &bytes, const int offset,
                   const int length, const std::string &value) {
    if (offset < 0 || offset + length > bytes.size())
        return Error("offset should be fit the size");
    if (value.size() > length)
        return Error("value should be smaller than length");
    std::memcpy(&bytes[offset], &value[0], value.size());
    return Ok();
}

} // namespace data