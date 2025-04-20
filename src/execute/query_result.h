#ifndef _QUERY_RESULT_H_
#define _QUERY_RESULT_H_

#include "data/data.h"
#include <variant>

namespace execute {

using Row = std::vector<data::DataItem>;

class DefaultResult {
  public:
    bool operator==(const DefaultResult &other) const { return true; }
    bool operator!=(const DefaultResult &other) const {
        return !(*this == other);
    }
};

class SelectResult {
  public:
    SelectResult(const std::vector<std::string> &column_names)
        : column_names_(column_names) {}
    SelectResult(const std::vector<std::string> &column_names,
                 const std::vector<Row> &rows)
        : column_names_(column_names), rows_(rows) {}

    // Add a row to the result.
    void Add(const Row &row) { rows_.push_back(row); }

    // Get the column names.
    const std::vector<std::string> &ColumnNames() const {
        return column_names_;
    }

    // Get the rows.
    const std::vector<Row> &Rows() const { return rows_; }

    bool operator==(const SelectResult &other) const {
        return column_names_ == other.column_names_ && rows_ == other.rows_;
    }

    bool operator!=(const SelectResult &other) const {
        return !(*this == other);
    }

  private:
    std::vector<std::string> column_names_;
    std::vector<Row> rows_;
};

using QueryResult = std::variant<DefaultResult, SelectResult>;

} // namespace execute

#endif // _QUERY_RESULT_H_