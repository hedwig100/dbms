#include "parser.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(ParserTest, ExampleSuccessTest) {
    sql::Parser parser;
    const std::string sql_stmt = "SELECT a FROM table;";

    auto result = parser.Parse(sql_stmt);

    EXPECT_TRUE(result.IsOk()) << "Error: " << result.Error();
}

TEST(ParserTest, ExampleFailureTest) {
    sql::Parser parser;
    const std::string sql_stmt = "SELECT a FROM;";
    auto result                = parser.Parse(sql_stmt);
    EXPECT_TRUE(result.IsError());
    std::cout << result.Error() << std::endl;
}

TEST(ParserTest, CompareSuccess) {
    sql::Parser parser;
    const std::string sql_stmt = "SELECT a FROM b WHERE a <= 4;";
    auto result                = parser.Parse(sql_stmt);
    EXPECT_TRUE(result.IsOk()) << "Error: " << result.Error();
}

TEST(ParserTest, ExpressionColumns) {
    sql::Parser parser;
    const std::string sql_stmt =
        "SELECT a = 9, b > 0, c < 0, 42 <= 0, 33 >= 3 FROM table WHERE a = b;";
    auto result = parser.Parse(sql_stmt);
    EXPECT_TRUE(result.IsOk()) << "Error: " << result.Error();
}

TEST(ParserTest, ColumnAlias) {
    sql::Parser parser;
    const std::string sql_stmt =
        "SELECT a = 9 AS alias1, 43 AS const_value FROM table WHERE a = b;";
    auto result = parser.Parse(sql_stmt);
    EXPECT_TRUE(result.IsOk()) << "Error: " << result.Error();
}