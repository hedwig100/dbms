#ifndef _PARSER_PARSER_H
#define _PARSER_PARSER_H

#include "bison.h"
#include "flex.h"
#include "sql.h"
#include <iostream>
#include <string>

namespace sql {

// TODO: too simple implementation
ParseResult Parse(const std::string &sql_stmt);

} // namespace sql

#endif // _PARSER_PARSER_H