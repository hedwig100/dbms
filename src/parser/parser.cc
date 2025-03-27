#include "parser.h"

namespace sql {

sql::ParseResult Parse(const std::string &sql_stmt) {
    yyscan_t scanner;
    YY_BUFFER_STATE state;
    yylex_init(&scanner);

    const char *sql_stmt_cchr = sql_stmt.c_str();
    state                     = yy_scan_string(sql_stmt_cchr, scanner);
    sql::ParseResult result(/*success=*/true);
    yyparse(&result, scanner);

    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    return result;
}

} // namespace sql