#include "data/byte.h"
#include "data/char.h"
#include "data/int.h"
#include "execute.h"
#include "execute/environment.h"
#include "execute/query_result.h"
#include "sql.h"
#include "table_scan.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

struct ExecuteTestParam {
    ExecuteTestParam(const std::string &sql, const bool expect_success,
                     const execute::QueryResult &result)
        : sql(sql), expect_success(expect_success), result(result) {}

    std::string sql;
    bool expect_success;
    execute::QueryResult result;
};

const std::string data_directory_path = "data_dir/";
const std::string log_directory_path  = "log_dir/";
const std::string log_filename        = "filename0";
const std::string tablename           = "table_for_test";
const std::string data_filename       = scan::TableFileName(tablename);

class ExecuteTest : public ::testing::TestWithParam<ExecuteTestParam> {
  protected:
    ExecuteTest()
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
            TRY(table_scan.Update("field3", data::Char("test", 4)));
        }
        TRY(table_scan.Close());
        TRY(insert_transaction.Commit());
        return Ok();
    }

    virtual ~ExecuteTest() override {
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
        schema::Field("field3", data::TypeChar(4)),
    });

    disk::DiskManager data_disk_manager;
    dblog::LogManager log_manager;
    buffer::SimpleBufferManager buffer_manager;
    dbconcurrency::LockTable lock_table;
    execute::Environment environment;

    transaction::Transaction transaction;
    transaction::Transaction transaction_for_check;
};

TEST_P(ExecuteTest, Select) {
    const std::string sql       = GetParam().sql;
    execute::QueryResult result = execute::DefaultResult();

    Result execute_result =
        execute::Execute(sql, result, transaction, environment);

    if (GetParam().expect_success) {
        EXPECT_TRUE(execute_result.IsOk()) << execute_result.Error();
        EXPECT_EQ(result, GetParam().result);
    } else {
        EXPECT_TRUE(execute_result.IsError());
    }
}

INSTANTIATE_TEST_SUITE_P(
    ExecuteTestSuite, ExecuteTest,
    ::testing::Values(
        ExecuteTestParam("SELECT field1 FROM table_for_test;",
                         /*expect_success=*/true,
                         execute::SelectResult({"field1"},
                                               {
                                                   {data::Int(0)},
                                                   {data::Int(1)},
                                                   {data::Int(2)},
                                                   {data::Int(3)},
                                                   {data::Int(4)},
                                                   {data::Int(5)},
                                                   {data::Int(6)},
                                                   {data::Int(7)},
                                                   {data::Int(8)},
                                                   {data::Int(9)},
                                               })),
        ExecuteTestParam("SELECT field1, 0, field2 FROM table_for_test;",
                         /*expect_success=*/true,
                         execute::SelectResult(
                             {"field1", "0", "field2"},
                             {
                                 {data::Int(0), data::Int(0), data::Int(0)},
                                 {data::Int(1), data::Int(0), data::Int(-1)},
                                 {data::Int(2), data::Int(0), data::Int(-2)},
                                 {data::Int(3), data::Int(0), data::Int(-3)},
                                 {data::Int(4), data::Int(0), data::Int(-4)},
                                 {data::Int(5), data::Int(0), data::Int(-5)},
                                 {data::Int(6), data::Int(0), data::Int(-6)},
                                 {data::Int(7), data::Int(0), data::Int(-7)},
                                 {data::Int(8), data::Int(0), data::Int(-8)},
                                 {data::Int(9), data::Int(0), data::Int(-9)},
                             })),
        ExecuteTestParam(
            "SELECT * FROM table_for_test;",
            /*expect_success=*/true,
            execute::SelectResult(
                {"field1", "field2", "field3"},
                {
                    {data::Int(0), data::Int(0), data::Char("test", 4)},
                    {data::Int(1), data::Int(-1), data::Char("test", 4)},
                    {data::Int(2), data::Int(-2), data::Char("test", 4)},
                    {data::Int(3), data::Int(-3), data::Char("test", 4)},
                    {data::Int(4), data::Int(-4), data::Char("test", 4)},
                    {data::Int(5), data::Int(-5), data::Char("test", 4)},
                    {data::Int(6), data::Int(-6), data::Char("test", 4)},
                    {data::Int(7), data::Int(-7), data::Char("test", 4)},
                    {data::Int(8), data::Int(-8), data::Char("test", 4)},
                    {data::Int(9), data::Int(-9), data::Char("test", 4)},
                })),
        ExecuteTestParam(
            "SELECT field1, field2 FROM table_for_test WHERE 0 = 0;",
            /*expect_success=*/true,
            execute::SelectResult({"field1", "field2"},
                                  {
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
                                  })),
        ExecuteTestParam(
            "SELECT field1, field2 FROM table_for_test WHERE field1 = 7;",
            /*expect_success=*/true,
            execute::SelectResult({"field1", "field2"},
                                  {

                                      {data::Int(7), data::Int(-7)},

                                  })),
        ExecuteTestParam(
            "SELECT field1, field2 FROM table_for_test WHERE field1 = field2;",
            /*expect_success=*/true,
            execute::SelectResult({"field1", "field2"},
                                  {

                                      {data::Int(0), data::Int(0)},

                                  })),
        ExecuteTestParam(
            "SELECT field1, field2 FROM table_for_test WHERE field1 < field2;",
            /*expect_success=*/true,
            execute::SelectResult({"field1", "field2"}, {})),
        ExecuteTestParam(
            "SELECT field1, field2 FROM table_for_test WHERE field1 > field2;",
            /*expect_success=*/true,
            execute::SelectResult({"field1", "field2"},
                                  {
                                      {data::Int(1), data::Int(-1)},
                                      {data::Int(2), data::Int(-2)},
                                      {data::Int(3), data::Int(-3)},
                                      {data::Int(4), data::Int(-4)},
                                      {data::Int(5), data::Int(-5)},
                                      {data::Int(6), data::Int(-6)},
                                      {data::Int(7), data::Int(-7)},
                                      {data::Int(8), data::Int(-8)},
                                      {data::Int(9), data::Int(-9)},
                                  })),
        ExecuteTestParam(
            "SELECT field1, field2 FROM table_for_test WHERE field1 <= field2;",
            /*expect_success=*/true,
            execute::SelectResult({"field1", "field2"},
                                  {
                                      {data::Int(0), data::Int(0)},
                                  })),
        ExecuteTestParam(
            "SELECT field1, field2 FROM table_for_test WHERE field1 >= field2;",
            /*expect_success=*/true,
            execute::SelectResult({"field1", "field2"},
                                  {
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
                                  })),
        ExecuteTestParam("SELECT * FROM table_for_test WHERE field1 = field3;",
                         /*expect_success=*/false, execute::DefaultResult()),
        ExecuteTestParam("SELECT field1 > 0, 33 < 0, 42 <= 0, 33 >= 3 FROM "
                         "table_for_test WHERE field1 = 0;",
                         /*expect_success=*/true,
                         execute::SelectResult(
                             {"field1 > 0", "33 < 0", "42 <= 0", "33 >= 3"},
                             {
                                 {data::Byte(0), data::Byte(0), data::Byte(0),
                                  data::Byte(1)},
                             }))));