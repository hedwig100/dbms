#ifndef _MACRO_TEST_TRANSACTION_H
#define _MACRO_TEST_TRANSACTION_H

#include "transaction.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <vector>

const std::string data_directory_path = "data_dir/";
const std::string log_directory_path  = "log_dir/";
const std::string log_filename        = "filename0";
const std::string data_filename       = "filename1";

class TransactionTest : public ::testing::Test {
  protected:
    TransactionTest()
        : data_disk_manager(data_directory_path, /*block_size=*/16),
          log_manager(log_filename, log_directory_path, /*block_size=*/16),
          buffer_manager(/*buffer_size=*/16, data_disk_manager, log_manager),
          lock_table(/*wait_time_sec=*/0.1),
          transaction_for_check(data_disk_manager, buffer_manager, log_manager,
                                lock_table) {

        if (!std::filesystem::exists(data_directory_path)) {
            std::filesystem::create_directories(data_directory_path);
        }
        if (!std::filesystem::exists(log_directory_path)) {
            std::filesystem::create_directories(log_directory_path);
        }

        std::ofstream data_file(data_directory_path + data_filename);
        if (data_file.is_open()) { data_file.close(); }
        std::ofstream log_file(log_directory_path + log_filename);
        if (log_file.is_open()) { log_file.close(); }

        Result result = log_manager.Init();
        if (result.IsError()) {
            throw std::runtime_error("Failed to initialize log manager " +
                                     result.Error());
        }

        result = data_disk_manager.AllocateNewBlocks(
            disk::BlockID(data_filename, /*block_id=*/32));
        if (result.IsError()) {
            throw std::runtime_error("Failed to allocate new blocks " +
                                     result.Error());
        }
    }

    virtual ~TransactionTest() override {
        Result result = buffer_manager.FlushAll();
        if (result.IsError()) {
            std::cerr << "Failed to flush all buffers " << result.Error()
                      << std::endl;
        }
        std::filesystem::remove_all(data_directory_path);
        std::filesystem::remove_all(log_directory_path);
    }

    disk::DiskManager data_disk_manager;
    dblog::LogManager log_manager;
    buffer::SimpleBufferManager buffer_manager;
    dbconcurrency::LockTable lock_table;

    transaction::Transaction transaction_for_check;
};

#endif