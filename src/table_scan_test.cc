#include "data/int.h"
#include "table_scan.h"
#include "transaction.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

const std::string data_directory_path = "data_dir/";
const std::string log_directory_path  = "log_dir/";
const std::string log_filename        = "filename0";
const std::string tablename           = "table_for_test";
const std::string data_filename       = scan::TableFileName(tablename);

class TableScanTest : public ::testing::Test {
  protected:
    TableScanTest()
        : data_disk_manager(data_directory_path, /*block_size=*/16),
          log_manager(log_filename, log_directory_path, /*block_size=*/16),
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
    }

    virtual ~TableScanTest() override {
        Result result = buffer_manager.FlushAll();
        if (result.IsError()) {
            std::cerr << "Failed to flush all buffers " << result.Error()
                      << std::endl;
        }
        std::filesystem::remove_all(data_directory_path);
        std::filesystem::remove_all(log_directory_path);
    }

    std::string table_name = "table_for_test";
    schema::Layout layout  = schema::Layout(schema::Schema({
        schema::Field("field1", data::TypeInt()),
    }));

    disk::DiskManager data_disk_manager;
    dblog::LogManager log_manager;
    buffer::SimpleBufferManager buffer_manager;
    dbconcurrency::LockTable lock_table;

    transaction::Transaction transaction;
    transaction::Transaction transaction_for_check;
};

TEST_F(TableScanTest, InitSuccess) {
    scan::TableScan table_scan(transaction, table_name, layout);
    Result result = table_scan.Init();
    EXPECT_TRUE(result.IsOk()) << result.Error();
}

TEST_F(TableScanTest, InsertSuccess) {
    scan::TableScan table_scan(transaction, table_name, layout);
    Result result = table_scan.Init();
    ASSERT_TRUE(result.IsOk()) << result.Error();

    Result insert_result = table_scan.Insert();
    EXPECT_TRUE(insert_result.IsOk()) << insert_result.Error();
    insert_result = table_scan.Insert();
    EXPECT_TRUE(insert_result.IsOk()) << insert_result.Error();

    Result commit_result = transaction.Commit();
    EXPECT_TRUE(commit_result.IsOk()) << commit_result.Error();
    scan::TableScan table_scan_for_check(transaction_for_check, table_name,
                                         layout);
    result = table_scan_for_check.Init();
    EXPECT_TRUE(result.IsOk()) << result.Error();
    ResultV<bool> next_result = table_scan_for_check.Next();
    EXPECT_TRUE(next_result.IsOk()) << next_result.Error();
    EXPECT_TRUE(next_result.Get()); // because there is a row.
    next_result = table_scan_for_check.Next();
    EXPECT_TRUE(next_result.IsOk()) << next_result.Error();
    EXPECT_FALSE(next_result.Get()); // because there is no row.
}

TEST_F(TableScanTest, UpdateSuccess) {
    scan::TableScan table_scan(transaction, table_name, layout);
    Result result = table_scan.Init();
    ASSERT_TRUE(result.IsOk()) << result.Error();

    Result insert_result = table_scan.Insert();
    EXPECT_TRUE(insert_result.IsOk()) << insert_result.Error();
    Result update_result = table_scan.Update("field1", data::Int(123));
    EXPECT_TRUE(update_result.IsOk()) << update_result.Error();
    insert_result = table_scan.Insert();
    EXPECT_TRUE(insert_result.IsOk()) << insert_result.Error();
    update_result = table_scan.Update("field1", data::Int(456));
    EXPECT_TRUE(update_result.IsOk()) << update_result.Error();

    Result commit_result = transaction.Commit();
    EXPECT_TRUE(commit_result.IsOk()) << commit_result.Error();
    scan::TableScan table_scan_for_check(transaction_for_check, table_name,
                                         layout);
    result = table_scan_for_check.Init();
    EXPECT_TRUE(result.IsOk()) << result.Error();
    ResultV<int> int_result = table_scan_for_check.GetInt("field1");
    EXPECT_TRUE(int_result.IsOk()) << int_result.Error();
    EXPECT_EQ(int_result.Get(), 123);
    ResultV<bool> next_result = table_scan_for_check.Next();
    EXPECT_TRUE(next_result.IsOk()) << next_result.Error();
    EXPECT_TRUE(next_result.Get()); // because there is a row.
    int_result = table_scan_for_check.GetInt("field1");
    EXPECT_TRUE(int_result.IsOk()) << int_result.Error();
    EXPECT_EQ(int_result.Get(), 456);
    next_result = table_scan_for_check.Next();
    EXPECT_TRUE(next_result.IsOk()) << next_result.Error();
    EXPECT_FALSE(next_result.Get()); // because there is no row.
}

TEST_F(TableScanTest, DeleteSuccess) {
    scan::TableScan table_scan(transaction, table_name, layout);
    Result result = table_scan.Init();
    ASSERT_TRUE(result.IsOk()) << result.Error();

    Result insert_result = table_scan.Insert();
    EXPECT_TRUE(insert_result.IsOk()) << insert_result.Error();
    Result delete_result = table_scan.Delete();
    EXPECT_TRUE(delete_result.IsOk()) << delete_result.Error();

    Result commit_result = transaction.Commit();
    EXPECT_TRUE(commit_result.IsOk()) << commit_result.Error();
    scan::TableScan table_scan_for_check(transaction_for_check, table_name,
                                         layout);
    result = table_scan_for_check.Init();
    EXPECT_TRUE(result.IsOk()) << result.Error();
    EXPECT_TRUE(!result.Get()); // because there is a no row.
}