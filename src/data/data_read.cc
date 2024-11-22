#include "data_read.h"
#include "char.h"
#include "int.h"
#include <iostream>

namespace data {

ResultV<std::unique_ptr<DataType>>
ReadType(const std::vector<uint8_t> &type_bytes, const int type_offset) {
    const uint8_t type_parameter_flag = type_bytes[type_offset];
    if (type_parameter_flag == kTypeParameterInt)
        return ReadTypeInt(type_bytes, type_offset);
    else if (type_parameter_flag == kTypeParameterChar)
        return ReadTypeChar(type_bytes, type_offset);
    return Error("data::ReadType() undefined type parameter.");
}

ResultV<std::unique_ptr<DataItem>>
ReadData(const DataType &datatype, const std::vector<uint8_t> &data_bytes,
         const int data_offset) {
    if (datatype.BaseType() == BaseDataType::kInt)
        return ReadDataInt(data_bytes, data_offset);
    else if (datatype.BaseType() == BaseDataType::kChar) {
        // 2 is the length of type parameter (type flag and length).
        const int length = datatype.ValueLength();
        return ReadDataChar(data_bytes, data_offset, length);
    }
    return Error("data::ReadData() undefined type parameter.");
}

ResultV<std::unique_ptr<DataItem>>
ReadTypeData(const std::vector<uint8_t> &bytes, const int offset) {
    ResultV<std::unique_ptr<DataType>> type_result = ReadType(bytes, offset);
    if (type_result.IsError())
        return type_result + Error("data::ReadTypeData() failed to read type.");
    return ReadData(*type_result.Get(), bytes,
                    offset + type_result.Get()->TypeParameterLength());
}

} // namespace data