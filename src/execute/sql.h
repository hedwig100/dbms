#ifndef _EXECUTE_SQL_H
#define _EXECUTE_SQL_H

#include "execute/environment.h"
#include "execute/query_result.h"
#include "result.h"
#include "scan.h"
#include "scans.h"
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

    // Get the column value using the scan.
    ResultV<data::DataItemWithType> Evaluate(scan::Scan &scan) const;

    // Returns the column name if it is a column name.
    // If it is a constant integer, an empty string is returned.
    std::string ColumnName() const;

    // This is used to get the name of the column or constant integer.
    std::string DisplayName() const;

  private:
    bool IsColumnName() const;
    int ConstInteger() const;

    std::variant<std::string, int> column_name_or_const_integer_;
};

enum class ComparisonOperator {
    Equal,
    Less,
    Greater,
    LessOrEqual,
    GreaterOrEqual
};

class BooleanPrimary {
  public:
    explicit BooleanPrimary(Column *left,
                            ComparisonOperator comparison_operator,
                            Column *right)
        : left_(left), comparison_operator_(comparison_operator),
          right_(right) {}

    // Evaluate the boolean expression
    ResultV<bool> Evaluate(scan::Scan &scan) const;

    // Get the column names used in the boolean expression
    std::vector<std::string> GetColumnNames() const {
        return {left_->ColumnName(), right_->ColumnName()};
    }

    // Get the display name of the boolean expression
    std::string DisplayName() const;

  private:
    Column *left_ = nullptr, *right_ = nullptr;
    ComparisonOperator comparison_operator_;
};

class Expression {
  public:
    Expression(BooleanPrimary *boolean_primary)
        : boolean_primary_(boolean_primary) {}

    // Evaluate the expression
    ResultV<data::DataItemWithType> Evaluate(scan::Scan &scan) const;

    // Get the column names used in the expression
    std::vector<std::string> GetColumnNames() const {
        return boolean_primary_->GetColumnNames();
    }

    // Get the display name of the expression
    std::string DisplayName() const { return boolean_primary_->DisplayName(); }

  public:
    BooleanPrimary *boolean_primary_ = nullptr;
};

class SelectExpression {
  public:
    explicit SelectExpression(Column *column) : column_(column) {}
    explicit SelectExpression(Expression *expression)
        : expression_(expression) {}

    // Evaluate returns the expression.
    ResultV<data::DataItemWithType> Evaluate(scan::Scan &scan) const;

    // Get the column names used in the expression
    std::vector<std::string> GetColumnNames() const {
        if (column_) { return {column_->ColumnName()}; }
        return expression_->GetColumnNames();
    }

    std::string DisplayName() const {
        if (column_) { return column_->DisplayName(); }
        return expression_->DisplayName();
    }

  private:
    Column *column_         = nullptr;
    Expression *expression_ = nullptr;
};

class Columns {
  public:
    Columns() : is_all_column_(false) {}
    explicit Columns(bool is_all_column) : is_all_column_(is_all_column) {}

    // Populate all columns from layout. This is used when `*` symbol i used in
    // SQL.
    void PopulateColumns(const schema::Layout &layout);

    void AddSelectExpression(SelectExpression *expression) {
        select_expressions_.push_back(expression);
    }

    ResultV<std::vector<data::DataItemWithType>>
    Evaluate(scan::Scan &scan) const;

    std::vector<std::string> GetColumnNames() const {
        std::vector<std::string> column_names;
        for (const SelectExpression *expression : select_expressions_) {
            auto names = expression->GetColumnNames();
            column_names.insert(column_names.end(), names.begin(), names.end());
        }
        return column_names;
    }

    std::vector<std::string> DisplayName() const {
        std::vector<std::string> names;
        for (const SelectExpression *expression : select_expressions_) {
            names.push_back(expression->DisplayName());
        }
        return names;
    }

  private:
    bool is_all_column_;
    std::vector<SelectExpression *> select_expressions_;
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
    SelectStatement(Columns *columns, Table *table,
                    BooleanPrimary *where_condition = nullptr)
        : columns_(columns), table_(table), where_condition_(where_condition) {}

    Table *GetTable() const { return table_; }

    // SELECT statement
    Result Execute(transaction::Transaction &transaction,
                   execute::QueryResult &result,
                   const execute::Environment &env);

    std::vector<std::string> GetColumnNames() const {
        return columns_->GetColumnNames();
    }

  private:
    // Check if the column names are valid in the given layout.
    bool IsValidColumns(const schema::Layout &layout) const;

    // Return true if the WHERE condition is true.
    ResultV<bool> WhereConditionIsTrue(scan::SelectScan &scan) const;

    Columns *columns_                = nullptr;
    Table *table_                    = nullptr;
    BooleanPrimary *where_condition_ = nullptr;
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