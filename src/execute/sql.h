#ifndef _EXECUTE_SQL_H
#define _EXECUTE_SQL_H

#include "execute/query_result.h"
#include "result.h"
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
    bool IsColumnName();

    // Returns the column name if it is a column name.
    std::string ColumnName() const;

    // Returns the constant integer if it is a constant integer.
    int ConstInteger() const;

  private:
    std::variant<std::string, int> column_name_or_const_integer_;
};

class Statement {
  public:
    virtual Result Execute(transaction::Transaction &transaction,
                           execute::QueryResult &result) = 0;
};

// SelectStatement class represents a SELECT statement.
class SelectStatement : public Statement {
  public:
    SelectStatement(Column *column, Table *table)
        : column_(column), table_(table) {}

    Column *GetColumn() const { return column_; }
    Table *GetTable() const { return table_; }

    // TODO: Implement the SELECT statement execution logic.
    Result Execute(transaction::Transaction &transaction,
                   execute::QueryResult &result) {
        return Ok();
    }

  private:
    Column *column_;
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