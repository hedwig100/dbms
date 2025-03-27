#ifndef _PARSER_PARSER_H
#define _PARSER_PARSER_H

#include "bison.h"
#include "flex.h"
#include "result.h"
#include "sql.h"
#include <iostream>
#include <string>

namespace sql {

using namespace ::result;

class Parser {
  public:
    Parser() { yylex_init(&scanner_); }

    ~Parser() { yylex_destroy(scanner_); }

    ResultV<ParseResult> Parse(const std::string &sql_stmt);

  private:
    yyscan_t scanner_;
    YY_BUFFER_STATE state_;
};

} // namespace sql

#endif // _PARSER_PARSER_H