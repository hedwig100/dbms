#include "parser.h"

namespace sql {

ResultV<ParseResult> Parser::Parse(const std::string &sql_stmt) {
    {
        const char *sql_stmt_cchr = sql_stmt.c_str();
        state_                    = yy_scan_string(sql_stmt_cchr, scanner_);
        ParseResult result;
        yyparse(&result, scanner_);
        yy_delete_buffer(state_, scanner_);

        if (result.IsError()) {
            return Error(result.Error());
        } else {
            return Ok(result);
        }
    }
}

} // namespace sql