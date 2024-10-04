#ifndef _DATA_DATA_READ_H
#define _DATA_DATA_READ_H

#include "data.h"
#include "result.h"
#include <memory>

namespace data {

// Read data from `datatype_bytes` and `data_bytes`.
ResultV<std::unique_ptr<DataItem>>
ReadData(const std::vector<uint8_t> &datatype_bytes, int datatype_offset,
         const std::vector<uint8_t> &data_bytes, int data_offset);

} // namespace data

#endif