#include "buffer.h"
#include "data/int.h"
#include "log.h"
#include "log_record.h"
#include "macro_test.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>

TEST(Log, LogRecordWithHeaderSuccess) {
    dblog::LogTransactionBegin log_record(/*transaction_id=*/0);
    auto log_record_with_header = dblog::LogRecordWithHeader(log_record);
    EXPECT_EQ(log_record_with_header.size(),
              /*length of checksum*/ 4 + /*length of size of log*/ 4 +
                  log_record.LogBody().size() + /*length of size of log*/ 4);
}

TEST(LogRecordLogTransactionBegin, InstantiationSuccess) {
    const dblog::TransactionID id = 10;
    dblog::LogTransactionBegin log_trans_begin(id);

    EXPECT_EQ(log_trans_begin.Type(), dblog::LogType::kTransactionBegin);

    auto log_body = log_trans_begin.LogBody();
    EXPECT_EQ(log_body[0], 0b00000000);
}

TEST(LogRecordLogOperation, InstantiationSuccess) {
    const dblog::TransactionID id = 10;
    const disk::BlockID block_id("file.txt", 3);
    const std::vector<uint8_t> int_value = {4, 0, 0, 0};
    const data::Int dummy_value(0);
    dblog::LogOperation log_op(id, disk::DiskPosition(block_id, 4),
                               /*previous_item=*/int_value,
                               /*new_item=*/dummy_value);

    EXPECT_EQ(log_op.Type(), dblog::LogType::kOperation);
    auto log_body = log_op.LogBody();
    EXPECT_EQ(log_body[0], 0b01000000);
}

TEST(LogRecordLogTransactionEnd, InstantiationSuccess) {
    const dblog::TransactionID id = 10;
    dblog::LogTransactionEnd log_trans_end(id,
                                           dblog::TransactionEndType::kCommit);

    EXPECT_EQ(log_trans_end.Type(), dblog::LogType::kTransactionEnd);

    auto log_body = log_trans_end.LogBody();
    EXPECT_EQ(log_body[0], 0b10000000);
}

TEST(LogRecordLogCheckpointing, InstantiationSuccess) {
    dblog::LogCheckpointing log_checkpointing;

    EXPECT_EQ(log_checkpointing.Type(), dblog::LogType::kCheckpointing);

    auto log_body = log_checkpointing.LogBody();
    EXPECT_EQ(log_body[0], 0b11000000);
}

TEST(LogRecordTransactionBegin, WriteReadCorrectly) {
    dblog::LogTransactionBegin log_record(/*transaction_id=*/6);

    auto log_body = log_record.LogBody();
    ResultV<std::unique_ptr<dblog::LogRecord>> log_record_ptr_result =
        dblog::ReadLogRecord(log_body);
    EXPECT_TRUE(log_record_ptr_result.IsOk());

    std::unique_ptr<dblog::LogRecord> log_record_ptr =
        log_record_ptr_result.MoveValue();
    EXPECT_EQ(log_record_ptr->Type(), dblog::LogType::kTransactionBegin);
    EXPECT_EQ(log_record_ptr->LogBody(), log_body);
}

TEST(LogRecordOperation, UpdateWriteReadCorrectly) {
    const std::vector<uint8_t> previous_value = {4, 0, 0, 0};
    const data::Int new_value(6);
    dblog::LogOperation log_record(
        /*transaction_id=*/6,
        disk::DiskPosition(disk::BlockID("xxx.txt", 4), 3), previous_value,
        new_value);

    auto log_body = log_record.LogBody();
    ResultV<std::unique_ptr<dblog::LogRecord>> log_record_ptr_result =
        dblog::ReadLogRecord(log_body);
    EXPECT_TRUE(log_record_ptr_result.IsOk());

    std::unique_ptr<dblog::LogRecord> log_record_ptr =
        log_record_ptr_result.MoveValue();
    EXPECT_EQ(log_record_ptr->Type(), dblog::LogType::kOperation);
    EXPECT_EQ(log_record_ptr->LogBody(), log_body);
}

TEST(LogRecordTransactionEnd, WriteReadCorrectly) {
    dblog::LogTransactionEnd log_record(
        /*transaction_id=*/6, dblog::TransactionEndType::kCommit);

    auto log_body = log_record.LogBody();
    ResultV<std::unique_ptr<dblog::LogRecord>> log_record_ptr_result =
        dblog::ReadLogRecord(log_body);
    EXPECT_TRUE(log_record_ptr_result.IsOk());

    std::unique_ptr<dblog::LogRecord> log_record_ptr =
        log_record_ptr_result.MoveValue();
    EXPECT_EQ(log_record_ptr->Type(), dblog::LogType::kTransactionEnd);
    EXPECT_EQ(log_record_ptr->LogBody(), log_body);
}

TEST(LogRecordCheckpointing, WriteReadCorrectly) {
    dblog::LogCheckpointing log_record;

    auto log_body = log_record.LogBody();
    ResultV<std::unique_ptr<dblog::LogRecord>> log_record_ptr_result =
        dblog::ReadLogRecord(log_body);
    EXPECT_TRUE(log_record_ptr_result.IsOk());

    std::unique_ptr<dblog::LogRecord> log_record_ptr =
        log_record_ptr_result.MoveValue();
    EXPECT_EQ(log_record_ptr->Type(), dblog::LogType::kCheckpointing);
    EXPECT_EQ(log_record_ptr->LogBody(), log_body);
}

TWO_FILE_EXISTENT_TEST(LogRecordTransactionBeginWithFile, "", "");

TEST_F(LogRecordTransactionBeginWithFile, UnDoCorrectly) {
    disk::DiskManager disk_manager(directory_path, 20);
    dblog::LogManager log_manager(filename1, directory_path, /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());
    buffer::SimpleBufferManager buffer_manager(/*buffer_size=*/4, disk_manager,
                                               log_manager);
    dblog::LogTransactionBegin log_record(4);
    EXPECT_TRUE(log_record.UnDo(buffer_manager).IsOk());
}

TEST_F(LogRecordTransactionBeginWithFile, ReDoCorrectly) {
    disk::DiskManager disk_manager(directory_path, 20);
    dblog::LogManager log_manager(filename1, directory_path, /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());
    buffer::SimpleBufferManager buffer_manager(/*buffer_size=*/4, disk_manager,
                                               log_manager);
    dblog::LogTransactionBegin log_record(4);
    EXPECT_TRUE(log_record.ReDo(buffer_manager).IsOk());
}

TWO_FILE_EXISTENT_TEST(LogRecordOperationWithFile, "", "");

TEST_F(LogRecordOperationWithFile, UnDoCorrectly) {
    disk::DiskManager disk_manager(directory_path, 20);
    dblog::LogManager log_manager(filename1, directory_path, /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());
    ASSERT_TRUE(log_manager.Init().IsOk());
    buffer::SimpleBufferManager buffer_manager(/*buffer_size=*/4, disk_manager,
                                               log_manager);
    const int expect_value               = 4;
    const std::vector<uint8_t> int_value = {expect_value, 0, 0, 0};
    const data::Int dummy_value(0);
    const disk::BlockID block_id(filename0, 0);
    const int offset = 7;
    dblog::LogOperation log_record(
        /*transaction_id=*/4, disk::DiskPosition(block_id, offset),
        /*previous_item=*/int_value, /*new_item=*/dummy_value);
    ASSERT_TRUE(
        disk_manager.AllocateNewBlocks(disk::BlockID(filename0, 2)).IsOk());

    EXPECT_TRUE(log_record.UnDo(buffer_manager).IsOk());
    Result flush_result = buffer_manager.Flush(block_id);
    ASSERT_TRUE(flush_result.IsOk()) << flush_result.Error();

    disk::Block block;
    Result read_result = disk_manager.Read(block_id, block);
    EXPECT_TRUE(read_result.IsOk());

    ResultV<int> int_result = block.ReadInt(offset);
    EXPECT_TRUE(int_result.IsOk());
    EXPECT_EQ(int_result.Get(), expect_value);
}

TEST_F(LogRecordOperationWithFile, InitializedFromLogBytesThenUnDoCorrectly) {
    disk::DiskManager disk_manager(directory_path, 20);
    dblog::LogManager log_manager(filename1, directory_path, /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());
    ASSERT_TRUE(log_manager.Init().IsOk());
    buffer::SimpleBufferManager buffer_manager(/*buffer_size=*/4, disk_manager,
                                               log_manager);
    const disk::BlockID block_id(filename0, 0);
    const int offset                     = 7;
    const int expect_value               = 4;
    const std::vector<uint8_t> log_bytes = {121, expect_value, 0, 0, 0, 0, 0, 0,
                                            0};
    dblog::LogOperation log_record(
        /*transaction_id=*/4, disk::DiskPosition(block_id, offset),
        /*log_bytes=*/log_bytes, /*data_offset_in_log_bytes=*/1);
    ASSERT_TRUE(
        disk_manager.AllocateNewBlocks(disk::BlockID(filename0, 2)).IsOk());

    EXPECT_TRUE(log_record.UnDo(buffer_manager).IsOk());
    Result flush_result = buffer_manager.Flush(block_id);
    ASSERT_TRUE(flush_result.IsOk()) << flush_result.Error();

    disk::Block block;
    Result read_result = disk_manager.Read(block_id, block);
    EXPECT_TRUE(read_result.IsOk());

    ResultV<int> int_result = block.ReadInt(offset);
    EXPECT_TRUE(int_result.IsOk());
    EXPECT_EQ(int_result.Get(), expect_value);
}

TEST_F(LogRecordOperationWithFile, ReDoCorrectly) {
    disk::DiskManager disk_manager(directory_path, 20);
    dblog::LogManager log_manager(filename1, directory_path, /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());
    buffer::SimpleBufferManager buffer_manager(/*buffer_size=*/4, disk_manager,
                                               log_manager);
    const std::vector<uint8_t> dummy_value = {0, 0, 0, 0};
    const data::Int int_value(4);
    const disk::BlockID block_id(filename0, 0);
    const int offset = 7;
    dblog::LogOperation log_record(
        /*transaction_id=*/4, disk::DiskPosition(block_id, offset),
        /*previous_item=*/dummy_value, /*new_item=*/int_value);
    ASSERT_TRUE(
        disk_manager.AllocateNewBlocks(disk::BlockID(filename0, 2)).IsOk());

    EXPECT_TRUE(log_record.ReDo(buffer_manager).IsOk());
    Result flush_result = buffer_manager.Flush(block_id);
    ASSERT_TRUE(flush_result.IsOk()) << flush_result.Error();

    disk::Block block;
    Result read_result = disk_manager.Read(block_id, block);
    EXPECT_TRUE(read_result.IsOk());

    ResultV<int> int_result = block.ReadInt(offset);
    EXPECT_TRUE(int_result.IsOk());
    EXPECT_EQ(int_result.Get(), int_value.Value());
}

TEST_F(LogRecordOperationWithFile, InitializedFromLogBytesThenReDoCorrectly) {
    disk::DiskManager disk_manager(directory_path, 20);
    dblog::LogManager log_manager(filename1, directory_path, /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());
    buffer::SimpleBufferManager buffer_manager(/*buffer_size=*/4, disk_manager,
                                               log_manager);
    const disk::BlockID block_id(filename0, 0);
    const int offset                     = 7;
    const int expect_value               = 4;
    const std::vector<uint8_t> log_bytes = {121,          0, 0, 0, 0,
                                            expect_value, 0, 0, 0};
    dblog::LogOperation log_record(
        /*transaction_id=*/4, disk::DiskPosition(block_id, offset),
        /*log_bytes=*/log_bytes, /*data_offset_in_log_bytes=*/1);
    ASSERT_TRUE(
        disk_manager.AllocateNewBlocks(disk::BlockID(filename0, 2)).IsOk());

    EXPECT_TRUE(log_record.ReDo(buffer_manager).IsOk());
    Result flush_result = buffer_manager.Flush(block_id);
    ASSERT_TRUE(flush_result.IsOk()) << flush_result.Error();

    disk::Block block;
    Result read_result = disk_manager.Read(block_id, block);
    EXPECT_TRUE(read_result.IsOk());

    ResultV<int> int_result = block.ReadInt(offset);
    EXPECT_TRUE(int_result.IsOk());
    EXPECT_EQ(int_result.Get(), expect_value);
}

TWO_FILE_EXISTENT_TEST(LogRecordTransactionEndWithFile, "", "");

TEST_F(LogRecordTransactionEndWithFile, UnDoCorrectly) {
    disk::DiskManager disk_manager(directory_path, 20);
    dblog::LogManager log_manager(filename1, directory_path, /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());
    buffer::SimpleBufferManager buffer_manager(/*buffer_size=*/4, disk_manager,
                                               log_manager);
    dblog::LogTransactionEnd log_record(4, dblog::TransactionEndType::kCommit);
    EXPECT_TRUE(log_record.UnDo(buffer_manager).IsOk());
}

TEST_F(LogRecordTransactionEndWithFile, ReDoCorrectly) {
    disk::DiskManager disk_manager(directory_path, 20);
    dblog::LogManager log_manager(filename1, directory_path, /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());
    buffer::SimpleBufferManager buffer_manager(/*buffer_size=*/4, disk_manager,
                                               log_manager);
    dblog::LogTransactionEnd log_record(4, dblog::TransactionEndType::kCommit);
    EXPECT_TRUE(log_record.ReDo(buffer_manager).IsOk());
}

TWO_FILE_EXISTENT_TEST(LogRecordCheckpointingWithFile, "", "");

TEST_F(LogRecordCheckpointingWithFile, UnDoCorrectly) {
    disk::DiskManager disk_manager(directory_path, 20);
    dblog::LogManager log_manager(filename1, directory_path, /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());
    buffer::SimpleBufferManager buffer_manager(/*buffer_size=*/4, disk_manager,
                                               log_manager);
    dblog::LogCheckpointing log_record;
    EXPECT_TRUE(log_record.UnDo(buffer_manager).IsOk());
}

TEST_F(LogRecordCheckpointingWithFile, ReDoCorrectly) {
    disk::DiskManager disk_manager(directory_path, 20);
    dblog::LogManager log_manager(filename1, directory_path, /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());
    buffer::SimpleBufferManager buffer_manager(/*buffer_size=*/4, disk_manager,
                                               log_manager);
    dblog::LogCheckpointing log_record;
    EXPECT_TRUE(log_record.ReDo(buffer_manager).IsOk());
}