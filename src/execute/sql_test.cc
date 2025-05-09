#include "data/int.h"
#include "execute/environment.h"
#include "execute/query_result.h"
#include "scans_test.h"
#include "sql.h"
#include "table_scan.h"
#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(Column, IsColumnName) {
    sql::Column column("field1");
    EXPECT_TRUE(column.IsColumnName());
    EXPECT_EQ(column.ColumnName(), "field1");
}

TEST(Column, ConstInteger) {
    sql::Column column(42);
    EXPECT_FALSE(column.IsColumnName());
    EXPECT_EQ(column.ConstInteger(), 42);
}

TEST(Column, Name) {
    sql::Column column("field1");
    EXPECT_EQ(column.Name(), "field1");

    sql::Column int_column(42);
    EXPECT_EQ(int_column.Name(), "42");
}

TEST(Column, ColumnNameGetColumn) {
    sql::Column column("field1");
    ScanForTest scan;

    auto result = column.GetColumn(scan);

    EXPECT_TRUE(result.IsOk());
    EXPECT_EQ(result.Get(), data::Int(1));
}

TEST(Column, ConstIntegerGetColumn) {
    sql::Column column(42);
    ScanForTest scan;

    auto result = column.GetColumn(scan);

    EXPECT_TRUE(result.IsOk());
    EXPECT_EQ(result.Get(), data::Int(42));
}

TEST(Columns, PopulateColumns) {
    sql::Columns all_columns(true);
    schema::Layout layout(
        schema::Schema({schema::Field("field1", data::kTypeInt),
                        schema::Field("field2", data::kTypeInt)}));

    all_columns.PopulateColumns(layout);

    EXPECT_THAT(all_columns.GetColmnNames(),
                ::testing::ElementsAre("field1", "field2"));
}

const std::string data_directory_path = "data_dir/";
const std::string log_directory_path  = "log_dir/";
const std::string log_filename        = "filename0";
const std::string tablename           = "table_for_test";
const std::string data_filename       = scan::TableFileName(tablename);

class SqlTest : public ::testing::Test {
  protected:
    SqlTest()
        : data_disk_manager(data_directory_path, /*block_size=*/128),
          log_manager(log_filename, log_directory_path, /*block_size=*/128),
          buffer_manager(/*buffer_size=*/16, data_disk_manager, log_manager),
          lock_table(/*wait_time_sec=*/0.1),
          transaction(data_disk_manager, buffer_manager, log_manager,
                      lock_table),
          transaction_for_check(data_disk_manager, buffer_manager, log_manager,
                                lock_table) {
        if (!std::filesystem::exists(data_directory_path)) {
            std::filesystem::create_directories(data_directory_path);
        }
        if (!std::filesystem::exists(log_directory_path)) {
            std::filesystem::create_directories(log_directory_path);
        }

        Result result = log_manager.Init();
        if (result.IsError()) {
            throw std::runtime_error("Failed to initialize log manager " +
                                     result.Error());
        }

        result = CreateAndInsertTableData();
        if (result.IsError()) {
            throw std::runtime_error("Failed to insert table data " +
                                     result.Error());
        }
    }

    Result CreateAndInsertTableData() {
        transaction::Transaction insert_transaction(
            data_disk_manager, buffer_manager, log_manager, lock_table);

        // Create the table
        FIRST_TRY(environment.GetTableManager().CreateTable(
            table_name, schema, insert_transaction));
        TRY_VALUE(layout, environment.GetTableManager().GetLayout(
                              table_name, insert_transaction));

        // Insert the data
        scan::TableScan table_scan(insert_transaction, table_name,
                                   layout.Get());
        TRY(table_scan.Init());
        for (int i = 0; i < 10; ++i) {
            TRY(table_scan.Insert());
            TRY(table_scan.Update("field1", data::Int(i)));
            TRY(table_scan.Update("field2", data::Int(-i)));
        }
        TRY(table_scan.Close());
        TRY(insert_transaction.Commit());
        return Ok();
    }

    virtual ~SqlTest() override {
        Result result = buffer_manager.FlushAll();
        if (result.IsError()) {
            std::cerr << "Failed to flush all buffers " << result.Error()
                      << std::endl;
        }
        std::filesystem::remove_all(data_directory_path);
        std::filesystem::remove_all(log_directory_path);
    }

    std::string table_name = "table_for_test";
    schema::Schema schema  = schema::Schema({
        schema::Field("field1", data::TypeInt()),
        schema::Field("field2", data::TypeInt()),
    });

    disk::DiskManager data_disk_manager;
    dblog::LogManager log_manager;
    buffer::SimpleBufferManager buffer_manager;
    dbconcurrency::LockTable lock_table;
    execute::Environment environment;

    transaction::Transaction transaction;
    transaction::Transaction transaction_for_check;
};

TEST_F(SqlTest, SelectSuccess) {
    sql::Columns *columns = new sql::Columns();
    columns->AddColumn(new sql::Column("field1"));
    columns->AddColumn(new sql::Column("field2"));
    columns->AddColumn(new sql::Column(0));
    sql::SelectStatement select_statement(columns,
                                          new sql::Table(tablename.c_str()));
    execute::QueryResult result = execute::DefaultResult();

    Result execute_result =
        select_statement.Execute(transaction, result, environment);

    EXPECT_TRUE(execute_result.IsOk()) << execute_result.Error();
    execute::QueryResult expect_result =
        execute::SelectResult({"field1", "field2", "0"},
                              {
                                  {data::Int(0), data::Int(0), data::Int(0)},
                                  {data::Int(1), data::Int(-1), data::Int(0)},
                                  {data::Int(2), data::Int(-2), data::Int(0)},
                                  {data::Int(3), data::Int(-3), data::Int(0)},
                                  {data::Int(4), data::Int(-4), data::Int(0)},
                                  {data::Int(5), data::Int(-5), data::Int(0)},
                                  {data::Int(6), data::Int(-6), data::Int(0)},
                                  {data::Int(7), data::Int(-7), data::Int(0)},
                                  {data::Int(8), data::Int(-8), data::Int(0)},
                                  {data::Int(9), data::Int(-9), data::Int(0)},
                              });
    EXPECT_EQ(result, expect_result);
}

TEST_F(SqlTest, SelectSuccessWithAllColumns) {
    sql::Columns *columns = new sql::Columns(true);
    sql::SelectStatement select_statement(columns,
                                          new sql::Table(tablename.c_str()));
    execute::QueryResult result = execute::DefaultResult();

    Result execute_result =
        select_statement.Execute(transaction, result, environment);

    EXPECT_TRUE(execute_result.IsOk()) << execute_result.Error();
    execute::QueryResult expect_result = execute::SelectResult(
        {"field1", "field2"}, {
                                  {data::Int(0), data::Int(0)},
                                  {data::Int(1), data::Int(-1)},
                                  {data::Int(2), data::Int(-2)},
                                  {data::Int(3), data::Int(-3)},
                                  {data::Int(4), data::Int(-4)},
                                  {data::Int(5), data::Int(-5)},
                                  {data::Int(6), data::Int(-6)},
                                  {data::Int(7), data::Int(-7)},
                                  {data::Int(8), data::Int(-8)},
                                  {data::Int(9), data::Int(-9)},
                              });
    EXPECT_EQ(result, expect_result);
}

TEST_F(SqlTest, SelectFailureWithInvalidColumn) {
    sql::Columns *columns = new sql::Columns();
    columns->AddColumn(new sql::Column("invalid-field"));
    columns->AddColumn(new sql::Column("field2"));
    sql::SelectStatement select_statement(columns,
                                          new sql::Table(tablename.c_str()));
    execute::QueryResult result = execute::DefaultResult();

    Result execute_result =
        select_statement.Execute(transaction, result, environment);

    EXPECT_TRUE(execute_result.IsError());
}

TEST_F(SqlTest, SelectSuccessWithWhereCondition) {
    sql::Columns *columns            = new sql::Columns(true);
    sql::Expression *where_condition = new sql::Expression(
        new sql::BooleanPrimary(new sql::Column("field1"), new sql::Column(0)));
    sql::SelectStatement select_statement(
        columns, new sql::Table(tablename.c_str()), where_condition);
    execute::QueryResult result = execute::DefaultResult();

    Result execute_result =
        select_statement.Execute(transaction, result, environment);

    EXPECT_TRUE(execute_result.IsOk()) << execute_result.Error();
    execute::QueryResult expect_result = execute::SelectResult(
        {"field1", "field2"}, {
                                  {data::Int(0), data::Int(0)},
                              });
    EXPECT_EQ(result, expect_result);
}

TEST_F(SqlTest, SelectFailWithWhereConditionWithInvalidColumn) {
    sql::Columns *columns = new sql::Columns(true);
    sql::Expression *where_condition =
        new sql::Expression(new sql::BooleanPrimary(
            new sql::Column("invalid-field"), new sql::Column(0)));
    sql::SelectStatement select_statement(
        columns, new sql::Table(tablename.c_str()), where_condition);
    execute::QueryResult result = execute::DefaultResult();

    Result execute_result =
        select_statement.Execute(transaction, result, environment);

    EXPECT_TRUE(execute_result.IsError());
}