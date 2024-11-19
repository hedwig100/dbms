#ifndef _DATA_DATA_READ_H
#define _DATA_DATA_READ_H

#include "data.h"
#include "result.h"
#include <memory>

namespace data {

// Read type from `type_bytes`. The bytes have type paremeter.
ResultV<std::unique_ptr<DataType>>
ReadType(const std::vector<uint8_t> &type_bytes, const int type_offset);

// Read data from `data_bytes`. The bytes have only the data.
ResultV<std::unique_ptr<DataItem>>
ReadData(const DataType &datatype, const std::vector<uint8_t> &data_bytes,
         const int data_offset);

// Read type and data from `data_bytes` in consecutive domain.
ResultV<std::unique_ptr<DataItem>>
ReadTypeData(const std::vector<uint8_t> &data_bytes, const int data_offset);

} // namespace data

#endif