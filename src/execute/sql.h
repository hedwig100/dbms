#ifndef _EXECUTE_SQL_H
#define _EXECUTE_SQL_H

#include "execute/environment.h"
#include "execute/query_result.h"
#include "result.h"
#include "scan.h"
#include "transaction/transaction.h"
#include <string>
#include <variant>
#include <vector>

namespace sql {

// Table class represents a table in the SQL statement.
class Table {
  public:
    Table(const char *table_name) : table_name_(table_name) {}

    std::string TableName() const { return table_name_; }

  private:
    std::string table_name_;
};

// Column class represents a column in the SQL statement.
class Column {
  public:
    Column(const char *column_name)
        : column_name_or_const_integer_(column_name) {}
    Column(int const_integer) : column_name_or_const_integer_(const_integer) {}

    // Check if the column represents a column name.
    bool IsColumnName() const;

    // Returns the column name if it is a column name.
    std::string ColumnName() const;

    // Returns the constant integer if it is a constant integer.
    int ConstInteger() const;

    // This is used to get the name of the column or constant integer.
    std::string Name() const;

    // Get the column value using the scan.
    ResultV<data::DataItem> GetColumn(scan::Scan &scan) const;

    // Returns true if the column is valid in the given layout.
    bool IsValid(const schema::Layout &layout) const;

  private:
    std::variant<std::string, int> column_name_or_const_integer_;
};

class Columns {
  public:
    Columns() : is_all_column_(false) {}
    explicit Columns(bool is_all_column) : is_all_column_(is_all_column) {}

    // Populate all columns from layout. This is used when `*` symbol i used in
    // SQL.
    void PopulateColumns(const schema::Layout &layout);

    void AddColumn(Column *column) { columns_.push_back(column); }

    std::vector<Column *> GetColumns() const { return columns_; }

    std::vector<std::string> GetColmnNames() const {
        std::vector<std::string> column_names;
        for (const Column *column : columns_) {
            column_names.push_back(column->Name());
        }
        return column_names;
    }

  private:
    bool is_all_column_;
    std::vector<Column *> columns_;
};

class Statement {
  public:
    virtual Result Execute(transaction::Transaction &transaction,
                           execute::QueryResult &result,
                           const execute::Environment &env) = 0;
};

// SelectStatement class represents a SELECT statement.
class SelectStatement : public Statement {
  public:
    SelectStatement(Columns *columns, Table *table)
        : columns_(columns), table_(table) {}

    Columns *GetColumns() const { return columns_; }
    Table *GetTable() const { return table_; }

    // SELECT statement
    Result Execute(transaction::Transaction &transaction,
                   execute::QueryResult &result,
                   const execute::Environment &env);

  private:
    bool IsValidColumns(const schema::Layout &layout) const;
    Columns *columns_;
    Table *table_;
};

// ParseResult class represents the result of parsing.
// It contains a vector of statements and error handling.
class ParseResult {
  public:
    ParseResult() {}

    void AddStatement(Statement *statement) {
        statements_.push_back(statement);
    }

    std::vector<Statement *> Statements() const { return statements_; }

    /*
     * Functions below are for error handling
     */
    bool IsError() const { return error_; }

    void SetError(const char *error_message) {
        error_         = true;
        error_message_ = error_message;
    }

    std::string Error() const { return error_message_; }

  private:
    std::vector<Statement *> statements_;

    // Variables for error handling
    bool error_ = false;
    std::string error_message_;
};

} // namespace sql

#endif // _EXECUTE_SQL_H_