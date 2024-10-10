#include "data/int.h"
#include "log_record.h"
#include <gtest/gtest.h>

TEST(LogRecordLogTransactionBegin, InstantiationSuccess) {
    const dblog::TransactionID id = 10;
    dblog::LogTransactionBegin log_trans_begin(id);

    EXPECT_EQ(log_trans_begin.Type(), dblog::LogType::kTransactionBegin);

    auto log_body = log_trans_begin.LogBody();
    EXPECT_EQ(log_body[0], 0b00000000);
}

TEST(LogRecordLogOperation, InstantiationSuccess) {
    const dblog::TransactionID id = 10;
    const disk::BlockID log_pointer("file.txt", 3);
    const data::Int int_value(4);
    dblog::LogOperation log_op(id, dblog::ManiplationType::kInsert, log_pointer,
                               &int_value, nullptr);

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
        disk::BlockID("xxx.txt", 4), nullptr, &int_value);

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
        disk::BlockID("xxx.txt", 4), &previous_value, &new_value);

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
