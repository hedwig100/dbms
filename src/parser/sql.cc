#include "sql.h"

namespace sql {

bool Column::IsColumnName() {
    return std::holds_alternative<std::string>(column_name_or_const_integer_);
}

std::string Column::ColumnName() const {
    if (std::holds_alternative<std::string>(column_name_or_const_integer_)) {
        return std::get<std::string>(column_name_or_const_integer_);
    }
    return "";
}

int Column::ConstInteger() const {
    if (std::holds_alternative<int>(column_name_or_const_integer_)) {
        return std::get<int>(column_name_or_const_integer_);
    }
    return 0;
}

} // namespace sql