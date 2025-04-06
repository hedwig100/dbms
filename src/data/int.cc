#include "int.h"
#include "data/copy.h"
#include <cstring>

namespace data {

Result TypeInt::Read(DataItem &item, const std::vector<uint8_t> &bytes,
                     const size_t offset) const {
    FIRST_TRY(internal::Read(item, bytes, offset, kIntBytesize));
    return Ok();
}

Result TypeInt::Write(const DataItem &item, std::vector<uint8_t> &bytes,
                      const size_t offset) const {
    FIRST_TRY(internal::Write(item, bytes, offset, kIntBytesize));
    return Ok();
}

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

} // namespace data