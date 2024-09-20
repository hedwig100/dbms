#include "data/int.h"
#include "log.h"
#include "log_record.h"
#include "macro_test.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <vector>

TEST(LogLogBlock, InstantiateOffset) {
    using namespace dblog::internal;
    LogBlock block(32);

    EXPECT_EQ(block.Offset(), 4);
}

TEST(LogLogBlock, AppendSuccess) {
    using namespace dblog::internal;
    LogBlock block(32);

    std::vector<uint8_t> bytes = {'a', 'b', 'c'};
    EXPECT_TRUE(block.Append(bytes, /*bytes_offset=*/1).IsOk());
    EXPECT_EQ(block.Offset(), 6);
    std::vector<uint8_t> raw_content = block.RawBlock().Content();
    EXPECT_EQ(raw_content[4], 'b');
    EXPECT_EQ(raw_content[5], 'c');
}

TEST(LogLogBlock, AppendTooLongFail) {
    using namespace dblog::internal;
    LogBlock block(6);

    std::vector<uint8_t> bytes = {'a', 'b', 'c', 'd'};
    auto append_result         = block.Append(bytes, /*bytes_offset=*/1);
    EXPECT_TRUE(append_result.IsError());
    EXPECT_EQ(append_result.Error(), 3);
    EXPECT_EQ(block.Offset(), 6);
    std::vector<uint8_t> raw_content = block.RawBlock().Content();
    EXPECT_EQ(raw_content[4], 'b');
    EXPECT_EQ(raw_content[5], 'c');
}

FILE_EXISTENT_TEST(LogFileExistentTest, "0001 block content ----");
constexpr size_t kOffset0001 = ('1' << 24) | ('0' << 16) | ('0' << 8) | '0';
const std::vector<uint8_t> kExpectBlockContentSize10 = {
    '0', '0', '0', '1', ' ', 'b', 'l', 'o', 'c', 'k'};

TEST_F(LogFileExistentTest, ReadLogBlockSuccess) {
    using namespace dblog::internal;
    const size_t block_size = 10;
    LogBlock block(block_size);
    disk::DiskManager disk_manager(directory_path, block_size);

    auto read_result =
        block.ReadLogBlock(disk_manager, disk::BlockID(filename, 0));
    EXPECT_TRUE(read_result.IsOk()) << read_result.Error();
    EXPECT_EQ(block.Offset(), kOffset0001);
    EXPECT_EQ(block.RawBlock().Content(), kExpectBlockContentSize10);
}

FILE_NONEXISTENT_TEST(LogFileNonExistentTest);

TEST_F(LogFileNonExistentTest, ReadLogBlockFailWithoutFile) {
    using namespace dblog::internal;
    const size_t block_size = 10;
    LogBlock block(block_size);
    disk::DiskManager disk_manager(directory_path, block_size);

    auto read_result = block.ReadLogBlock(
        disk_manager, disk::BlockID(non_existent_filename, 0));
    EXPECT_TRUE(read_result.IsError());
}

TEST(Log, LogRecordWithHeaderSuccess) {
    dblog::LogTransactionBegin log_record(/*transaction_id=*/0);
    auto log_record_with_header = dblog::LogRecordWithHeader(&log_record);
    EXPECT_EQ(log_record_with_header.size(),
              /*length of checksum*/ 4 + /*length of size of log*/ 4 +
                  log_record.LogBody().size() + /*length of size of log*/ 4);
}

FILE_EXISTENT_TEST(LogFileExistentLogManager, "abcdef");
FILE_NONEXISTENT_TEST(LogFileNonExistentLogManager);

TEST_F(LogFileExistentLogManager, InitSuccess) {
    dblog::LogManager log_manager(/*log_filename=*/filename,
                                  /*log_directory_name=*/directory_path,
                                  /*block_size=*/20);
    EXPECT_TRUE(log_manager.Init().IsOk());
}

TEST_F(LogFileNonExistentLogManager, InitFail) {
    dblog::LogManager log_manager(/*log_filename=*/non_existent_filename,
                                  /*log_directory_name=*/directory_path,
                                  /*block_size=*/20);
    EXPECT_TRUE(log_manager.Init().IsError());
}

TEST_F(LogFileExistentLogManager, WriteLogSuccess) {
    dblog::LogManager log_manager(/*log_filename=*/filename,
                                  /*log_directory_name=*/directory_path,
                                  /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());

    std::vector<uint8_t> bytes = {'a', 'b', 'c', 'd'};
    auto write_result          = log_manager.WriteLog(bytes);
    EXPECT_TRUE(write_result.IsOk()) << write_result.Error() << '\n';
}

TEST_F(LogFileExistentLogManager, FlushSuccess) {
    dblog::LogManager log_manager(/*log_filename=*/filename,
                                  /*log_directory_name=*/directory_path,
                                  /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());

    std::vector<uint8_t> bytes = {'a', 'b', 'c', 'd'};
    auto write_result          = log_manager.WriteLog(bytes);
    ASSERT_TRUE(write_result.IsOk()) << write_result.Error() << '\n';
    dblog::LogSequenceNumber lsn = write_result.Get();
    auto flush_result            = log_manager.Flush(lsn);
    EXPECT_TRUE(flush_result.IsOk());
}