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

DataItem Int(const int value) {
    DataItem item(kIntBytesize);
    std::memcpy(item.begin(), &value, kIntBytesize);
    return item;
}

int ReadInt(const data::DataItem &item) {
    int value = 0;
    std::memcpy(&value, item.begin(), kIntBytesize);
    return value;
}

} // namespace data