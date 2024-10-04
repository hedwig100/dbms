#include "data_read.h"
#include "char.h"
#include "int.h"

namespace data {

ResultV<std::unique_ptr<DataItem>>
ReadData(const std::vector<uint8_t> &datatype_bytes, int datatype_offset,
         const std::vector<uint8_t> &data_bytes, int data_offset) {

    const uint8_t type_parameter_flag = datatype_bytes[datatype_offset];
    if (type_parameter_flag == kTypeParameterInt)
        return ReadDataInt(data_bytes, data_offset);
    else if (type_parameter_flag == kTypeParameterChar)
        return ReadDataChar(datatype_bytes, datatype_offset, data_bytes,
                            data_offset);
    return Error("data::ReadData() undefined type parameter.");
}

} // namespace data