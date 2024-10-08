#include "data_read.h"
#include "char.h"
#include "int.h"

namespace data {

ResultV<std::unique_ptr<DataItem>>
ReadData(const std::vector<uint8_t> &data_bytes, const int data_offset) {
    const uint8_t type_parameter_flag = data_bytes[data_offset];
    if (type_parameter_flag == kTypeParameterInt)
        return ReadDataInt(data_bytes, data_offset);
    else if (type_parameter_flag == kTypeParameterChar)
        return ReadDataChar(data_bytes, data_offset);
    return Error("data::ReadData() undefined type parameter.");
}

} // namespace data