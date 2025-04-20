#include "execute.h"
#include "parser/parser.h"

namespace execute {

Result Execute(const std::string &sql, execute::QueryResult &result,
               transaction::Transaction &transaction, const Environment &env) {
    sql::Parser parser;
    TRY_VALUE(parse_result, parser.Parse(sql));

    for (auto &statement : parse_result.Get().Statements()) {
        FIRST_TRY(statement->Execute(transaction, result, env));
    }

    return Ok();
}

} // namespace execute