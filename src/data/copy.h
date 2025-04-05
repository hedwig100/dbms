#ifndef _DATA_COPY_H
#define _DATA_COPY_H

#include "data/data.h"
#include "result.h"
#include <cstdint>

namespace data {
namespace internal {

Result Read(DataItem &item, const std::vector<uint8_t> &bytes,
            const size_t offset, int length);

Result Write(const DataItem &item, std::vector<uint8_t> &bytes,
             const size_t offset, int length);

} // namespace internal
} // namespace data

#endif // _DATA_COPY_H