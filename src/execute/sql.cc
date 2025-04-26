#include "sql.h"
#include "execute/query_result.h"
#include "scans.h"
#include "table_scan.h"
#include <memory>

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

Result SelectStatement::Execute(transaction::Transaction &transaction,
                                execute::QueryResult &result,
                                const execute::Environment &env) {

    // TODO: Implement when column represents a const integer;
    for (auto column : columns_->GetColumns()) {
        if (!column->IsColumnName()) {
            return Error("[TODO] We have to implement when column_ is a const "
                         "integer");
        }
    }

    const metadata::TableManager &table_manager = env.GetTableManager();
    TRY_VALUE(layout,
              table_manager.GetLayout(table_->TableName(), transaction));
    scan::TableScan table_scan(transaction, table_->TableName(), layout.Get());
    scan::SelectScan select_scan(table_scan);

    execute::SelectResult select_result(columns_->GetColmnNames());
    FIRST_TRY(select_scan.Init());
    while (true) {
        execute::Row row;
        for (auto column : columns_->GetColumns()) {
            TRY_VALUE(item, select_scan.Get(column->ColumnName()));
            row.push_back(item.Get());
        }
        select_result.Add(row);
        TRY_VALUE(has_next, select_scan.Next());
        if (!has_next.Get()) break;
    }

    result = select_result;
    return Ok();
}

} // namespace sql