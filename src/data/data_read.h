#ifndef _DATA_DATA_READ_H
#define _DATA_DATA_READ_H

#include "data.h"
#include "result.h"
#include <memory>

namespace data {

// Read data from `data_bytes`. The bytes have type paremeter and the data
// itself in continuous domain.
ResultV<std::unique_ptr<DataItem>>
ReadData(const std::vector<uint8_t> &data_bytes, const int data_offset);

} // namespace data

#endif