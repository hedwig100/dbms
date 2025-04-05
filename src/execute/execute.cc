#include "execute.h"
#include "parser/parser.h"

namespace execute {

Result Execute(const std::string &sql, transaction::Transaction &transaction,
               execute::QueryResult &result) {
    sql::Parser parser;
    TRY_VALUE(parse_result, parser.Parse(sql));

    for (auto &statement : parse_result.Get().Statements()) {
        FIRST_TRY(statement->Execute(transaction, result));
    }

    return Ok();
}

} // namespace execute