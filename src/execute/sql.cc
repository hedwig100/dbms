#include "sql.h"
#include "data/int.h"
#include "debug.h"
#include "execute/query_result.h"
#include "scans.h"
#include "table_scan.h"
#include <memory>

namespace sql {

bool Column::IsColumnName() const {
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

std::string Column::Name() const {
    if (std::holds_alternative<std::string>(column_name_or_const_integer_)) {
        return std::get<std::string>(column_name_or_const_integer_);
    }
    return std::to_string(std::get<int>(column_name_or_const_integer_));
}

ResultV<data::DataItem> Column::GetColumn(scan::Scan &scan) const {
    if (IsColumnName()) {
        TRY_VALUE(item, scan.Get(ColumnName()));
        return Ok(item.Get());
    }
    return Ok(data::Int(ConstInteger()));
}

bool Column::IsValid(const schema::Layout &layout) const {
    if (IsColumnName()) { return layout.HasField(ColumnName()); }
    return true;
}

void Columns::PopulateColumns(const schema::Layout &layout) {
    if (!is_all_column_) { return; }
    for (const auto &fieldname : layout.FieldNames()) {
        columns_.push_back(new Column(fieldname.c_str()));
    }
    return;
}

Result SelectStatement::Execute(transaction::Transaction &transaction,
                                execute::QueryResult &result,
                                const execute::Environment &env) {
    DEBUG("SelectStatement::Execute() called");
    const metadata::TableManager &table_manager = env.GetTableManager();
    TRY_VALUE(layout,
              table_manager.GetLayout(table_->TableName(), transaction));
    columns_->PopulateColumns(layout.Get());
    if (!IsValidColumns(layout.Get())) {
        return Error("SelectStatement::Execute() Invalid columns in the SELECT "
                     "statement");
    }

    scan::TableScan table_scan(transaction, table_->TableName(), layout.Get());
    scan::SelectScan select_scan(table_scan);
    execute::SelectResult select_result(columns_->GetColmnNames());
    FIRST_TRY(select_scan.Init());
    while (true) {
        execute::Row row;
        for (const Column *column : columns_->GetColumns()) {
            TRY_VALUE(item, column->GetColumn(select_scan));
            row.push_back(item.Get());
        }
        select_result.Add(row);
        TRY_VALUE(has_next, select_scan.Next());
        if (!has_next.Get()) break;
    }

    result = select_result;
    return Ok();
}

bool SelectStatement::IsValidColumns(const schema::Layout &layout) const {
    for (const Column *column : columns_->GetColumns()) {
        if (!column->IsValid(layout)) { return false; }
    }
    return true;
}

} // namespace sql