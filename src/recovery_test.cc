#include "data/int.h"
#include "log.h"
#include "log_record.h"
#include "macro_test.h"
#include "recovery.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

FILE_EXISTENT_TEST(RecoveryManagerTest, "");

TEST_F(RecoveryManagerTest, WriteLogSuccess) {
    dblog::LogManager log_manager(/*log_filename=*/filename,
                                  /*log_directory_path=*/directory_path,
                                  /*block_size=*/12);
    ASSERT_TRUE(log_manager.Init().IsOk());
    recovery::RecoveryManager manager(std::move(log_manager));

    dblog::LogTransactionBegin log0(/*transaction_id=*/5);
    dblog::LogCheckpointing log1;

    ResultV<dblog::LogSequenceNumber> write_result = manager.WriteLog(log0);
    EXPECT_TRUE(write_result.IsOk());

    write_result = manager.WriteLog(log1);
    EXPECT_TRUE(write_result.IsOk());
}

TEST_F(RecoveryManagerTest, CommitSuccess) {
    dblog::LogManager log_manager(/*log_filename=*/filename,
                                  /*log_directory_path=*/directory_path,
                                  /*block_size=*/12);
    ASSERT_TRUE(log_manager.Init().IsOk());
    recovery::RecoveryManager manager(std::move(log_manager));

    dblog::LogTransactionBegin log0(/*transaction_id=*/5);
    ResultV<dblog::LogSequenceNumber> write_result = manager.WriteLog(log0);
    ASSERT_TRUE(write_result.IsOk());

    Result commit_result = manager.Commit(/*transaction_id=*/5);
    EXPECT_TRUE(commit_result.IsOk());
}

TWO_FILE_EXISTENT_TEST(RecoveryManagerTwoFileTest, "", "");

TEST_F(RecoveryManagerTwoFileTest, RollbackSuccess) {
    const int block_size = 12;
    dblog::LogManager log_manager(/*log_filename=*/filename0,
                                  /*log_directory_path=*/directory_path,
                                  /*block_size=*/block_size);
    ASSERT_TRUE(log_manager.Init().IsOk());
    recovery::RecoveryManager manager(std::move(log_manager));
    disk::DiskManager disk_manager(/*directory_name=*/directory_path,
                                   /*block_size=*/12);
    ASSERT_TRUE(
        disk_manager
            .AllocateNewBlocks(disk::BlockID(filename1, /*block_index=*/9))
            .IsOk());
    dblog::TransactionID transaction_id = 5;

    dblog::LogTransactionBegin log0(transaction_id);
    ResultV<dblog::LogSequenceNumber> write_result = manager.WriteLog(log0);
    ASSERT_TRUE(write_result.IsOk());
    const int expect_value = 4;
    disk::DiskPosition position1(
        disk::DiskPosition(disk::BlockID(filename1, 4), 3));
    dblog::LogOperation log1(transaction_id, dblog::ManiplationType::kUpdate,
                             position1,
                             std::make_unique<data::Int>(expect_value),
                             std::make_unique<data::Int>(7));
    ASSERT_TRUE(manager.WriteLog(log1).IsOk());
    disk::DiskPosition position2 =
        disk::DiskPosition(disk::BlockID(filename1, 8), 3);
    dblog::LogOperation log2(0, dblog::ManiplationType::kUpdate, position2,
                             std::make_unique<data::Int>(2),
                             std::make_unique<data::Int>(5));
    ASSERT_TRUE(manager.WriteLog(log2).IsOk());

    Result rollback_result = manager.Rollback(transaction_id, disk_manager);
    EXPECT_TRUE(rollback_result.IsOk());
    disk::Block block(block_size);
    Result read_result = disk_manager.Read(position1.BlockID(), block);
    ASSERT_TRUE(read_result.IsOk());
    ResultV<int> value = block.ReadInt(position1.Offset());
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Get(), expect_value);
    read_result = disk_manager.Read(position2.BlockID(), block);
    ASSERT_TRUE(read_result.IsOk());
    std::cout << position2.Offset() << '\n';
    value = block.ReadInt(position2.Offset());
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Get(), 0);
}