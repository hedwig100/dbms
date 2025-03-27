#include "parser.h"
#include <gtest/gtest.h>

TEST(ParserTest, ParseSelect) {
    const std::string sql_stmt    = "SELECT a FROM table;";
    const sql::ParseResult result = sql::Parse(sql_stmt);
    ASSERT_TRUE(result.IsSuccess());
}