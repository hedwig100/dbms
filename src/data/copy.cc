
#include "data/copy.h"

namespace data {
namespace internal {

Result Read(DataItem &item, const std::vector<uint8_t> &bytes,
            const size_t offset, int length) {
    if (offset < 0 || offset + length > bytes.size())
        return Error("data::internal::Read() offset should be fit the size.");
    item.resize(length);
    std::copy(bytes.begin() + offset, bytes.begin() + offset + length,
              item.begin());
    return Ok();
}

Result Write(const DataItem &item, std::vector<uint8_t> &bytes,
             const size_t offset, int length) {
    if (offset < 0 || offset + length > bytes.size())
        return Error("data::internal::Write() offset should be fit the size.");
    if (item.size() < length)
        return Error("data::internal::Write() item size should be larger than "
                     "length.");
    std::copy(item.begin(), item.begin() + length, bytes.begin() + offset);
    return Ok();
}

} // namespace internal
} // namespace data