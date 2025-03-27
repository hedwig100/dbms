#include "parser.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(ParserTest, ExampleSuccessTest) {
    sql::Parser parser;
    const std::string sql_stmt = "SELECT a FROM table;";
    auto result                = parser.Parse(sql_stmt);
    EXPECT_TRUE(result.IsOk());
}

TEST(ParserTest, ExampleFailureTest) {
    sql::Parser parser;
    const std::string sql_stmt = "SELECT a FROM;";
    auto result                = parser.Parse(sql_stmt);
    EXPECT_TRUE(result.IsError());
    std::cout << result.Error() << std::endl;
}