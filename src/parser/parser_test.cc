#include "parser.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(ParserTest, ExampleSuccessTest) {
    sql::Parser parser;
    const std::string sql_stmt = "SELECT a FROM table;";

    auto result = parser.Parse(sql_stmt);

    EXPECT_TRUE(result.IsOk());

    sql::Statement *statement = result.Get().Statements()[0];
    sql::SelectStatement *select_statement =
        dynamic_cast<sql::SelectStatement *>(statement);
    ASSERT_TRUE(statement != nullptr);
    EXPECT_EQ(select_statement->GetColumn()->ColumnName(), "a");
    EXPECT_EQ(select_statement->GetTable()->TableName(), "table");
}

TEST(ParserTest, ExampleFailureTest) {
    sql::Parser parser;
    const std::string sql_stmt = "SELECT a FROM;";
    auto result                = parser.Parse(sql_stmt);
    EXPECT_TRUE(result.IsError());
    std::cout << result.Error() << std::endl;
}