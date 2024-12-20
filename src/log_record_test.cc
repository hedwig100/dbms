#include "data/int.h"
#include "log_record.h"
#include "macro_test.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>

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
    const data::Int int_value(4);
    dblog::LogOperation log_op(id, dblog::ManiplationType::kInsert,
                               disk::DiskPosition(block_id, 4),
                               std::make_unique<data::Int>(int_value), nullptr);

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

TEST(LogRecordOperation, InsertWriteReadCorrectly) {
    const data::Int int_value(4);
    dblog::LogOperation log_record(
        /*transaction_id=*/6, dblog::ManiplationType::kInsert,
        disk::DiskPosition(disk::BlockID("xxx.txt", 4), 2), nullptr,
        std::make_unique<data::Int>(int_value));

    auto log_body = log_record.LogBody();
    ResultV<std::unique_ptr<dblog::LogRecord>> log_record_ptr_result =
        dblog::ReadLogRecord(log_body);
    EXPECT_TRUE(log_record_ptr_result.IsOk());

    std::unique_ptr<dblog::LogRecord> log_record_ptr =
        log_record_ptr_result.MoveValue();
    EXPECT_EQ(log_record_ptr->Type(), dblog::LogType::kOperation);
    EXPECT_EQ(log_record_ptr->LogBody(), log_body);
}

TEST(LogRecordOperation, UpdateWriteReadCorrectly) {
    const data::Int previous_value(4), new_value(6);
    dblog::LogOperation log_record(
        /*transaction_id=*/6, dblog::ManiplationType::kUpdate,
        disk::DiskPosition(disk::BlockID("xxx.txt", 4), 3),
        std::make_unique<data::Int>(previous_value),
        std::make_unique<data::Int>(new_value));

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

FILE_EXISTENT_TEST(LogRecordTransactionBeginWithFile, "");

TEST_F(LogRecordTransactionBeginWithFile, UnDoCorrectly) {
    const disk::DiskManager disk_manager(directory_path, 20);
    dblog::LogTransactionBegin log_record(4);
    EXPECT_TRUE(log_record.UnDo(disk_manager).IsOk());
}

TEST_F(LogRecordTransactionBeginWithFile, ReDoCorrectly) {
    const disk::DiskManager disk_manager(directory_path, 20);
    dblog::LogTransactionBegin log_record(4);
    EXPECT_TRUE(log_record.ReDo(disk_manager).IsOk());
}

FILE_EXISTENT_TEST(LogRecordOperationWithFile, "");

TEST_F(LogRecordOperationWithFile, UnDoCorrectly) {
    const disk::DiskManager disk_manager(directory_path, 20);
    const int int_value = 4;
    const int offset    = 7;

    dblog::LogOperation log_record(
        /*transaction_id=*/4, dblog::ManiplationType::kDelete,
        disk::DiskPosition(disk::BlockID(filename, 0), offset),
        std::make_unique<data::Int>(int_value), nullptr);

    ASSERT_TRUE(
        disk_manager.AllocateNewBlocks(disk::BlockID(filename, 2)).IsOk());
    EXPECT_TRUE(log_record.UnDo(disk_manager).IsOk());
    ASSERT_TRUE(disk_manager.Flush(filename).IsOk());

    disk::Block block;
    Result read_result = disk_manager.Read(disk::BlockID(filename, 0), block);
    EXPECT_TRUE(read_result.IsOk());

    ResultV<int> int_result = block.ReadInt(offset);
    EXPECT_TRUE(int_result.IsOk());
    EXPECT_EQ(int_result.Get(), int_value);
}

TEST_F(LogRecordOperationWithFile, ReDoCorrectly) {
    const disk::DiskManager disk_manager(directory_path, 20);
    const int int_value = 4;
    const int offset    = 7;

    dblog::LogOperation log_record(
        /*transaction_id=*/4, dblog::ManiplationType::kInsert,
        disk::DiskPosition(disk::BlockID(filename, 0), offset), nullptr,
        std::make_unique<data::Int>(int_value));

    ASSERT_TRUE(
        disk_manager.AllocateNewBlocks(disk::BlockID(filename, 2)).IsOk());
    EXPECT_TRUE(log_record.ReDo(disk_manager).IsOk());
    ASSERT_TRUE(disk_manager.Flush(filename).IsOk());

    disk::Block block;
    Result read_result = disk_manager.Read(disk::BlockID(filename, 0), block);
    EXPECT_TRUE(read_result.IsOk());

    ResultV<int> int_result = block.ReadInt(offset);
    EXPECT_TRUE(int_result.IsOk());
    EXPECT_EQ(int_result.Get(), int_value);
}

TEST_F(LogRecordOperationWithFile, ReDoFailWhenNewItemIsNull) {
    const disk::DiskManager disk_manager(directory_path, 20);
    const int int_value = 4;
    const int offset    = 7;

    dblog::LogOperation log_record(
        /*transaction_id=*/4, dblog::ManiplationType::kInsert,
        disk::DiskPosition(disk::BlockID(filename, 0), offset), nullptr,
        nullptr);

    ASSERT_TRUE(
        disk_manager.AllocateNewBlocks(disk::BlockID(filename, 2)).IsOk());
    EXPECT_TRUE(log_record.ReDo(disk_manager).IsError());
}

FILE_EXISTENT_TEST(LogRecordTransactionEndWithFile, "");

TEST_F(LogRecordTransactionEndWithFile, UnDoCorrectly) {
    const disk::DiskManager disk_manager(directory_path, 20);
    dblog::LogTransactionEnd log_record(4, dblog::TransactionEndType::kCommit);
    EXPECT_TRUE(log_record.UnDo(disk_manager).IsOk());
}

TEST_F(LogRecordTransactionEndWithFile, ReDoCorrectly) {
    const disk::DiskManager disk_manager(directory_path, 20);
    dblog::LogTransactionEnd log_record(4, dblog::TransactionEndType::kCommit);
    EXPECT_TRUE(log_record.ReDo(disk_manager).IsOk());
}

FILE_EXISTENT_TEST(LogRecordCheckpointingWithFile, "");

TEST_F(LogRecordCheckpointingWithFile, UnDoCorrectly) {
    const disk::DiskManager disk_manager(directory_path, 20);
    dblog::LogCheckpointing log_record;
    EXPECT_TRUE(log_record.UnDo(disk_manager).IsOk());
}

TEST_F(LogRecordCheckpointingWithFile, ReDoCorrectly) {
    const disk::DiskManager disk_manager(directory_path, 20);
    dblog::LogCheckpointing log_record;
    EXPECT_TRUE(log_record.ReDo(disk_manager).IsOk());
}