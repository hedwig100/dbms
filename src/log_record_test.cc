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