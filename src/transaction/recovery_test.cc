#include "buffer.h"
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
    recovery::RecoveryManager manager(log_manager);

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
    recovery::RecoveryManager manager(log_manager);

    dblog::LogTransactionBegin log0(/*transaction_id=*/5);
    ResultV<dblog::LogSequenceNumber> write_result = manager.WriteLog(log0);
    ASSERT_TRUE(write_result.IsOk());

    Result commit_result = manager.Commit(/*transaction_id=*/5);
    EXPECT_TRUE(commit_result.IsOk());
}

TWO_FILE_EXISTENT_TEST(RecoveryManagerTwoFileTest, "", "");

TEST_F(RecoveryManagerTwoFileTest, RollbackSuccessWithVariousBlockSize) {
    for (int block_size : {12, 128}) {
        dblog::LogManager log_manager(/*log_filename=*/filename0,
                                      /*log_directory_path=*/directory_path,
                                      /*block_size=*/block_size);
        ASSERT_TRUE(log_manager.Init().IsOk());
        recovery::RecoveryManager manager(log_manager);
        disk::DiskManager disk_manager(/*directory_name=*/directory_path,
                                       /*block_size=*/12);
        buffer::SimpleBufferManager buffer_manager(/*buffer_size=*/4,
                                                   disk_manager, log_manager);
        ASSERT_TRUE(
            disk_manager
                .AllocateNewBlocks(disk::BlockID(filename1, /*block_index=*/9))
                .IsOk());
        dblog::TransactionID transaction_id = 5;

        dblog::LogTransactionBegin log0(transaction_id);
        ResultV<dblog::LogSequenceNumber> write_result = manager.WriteLog(log0);
        ASSERT_TRUE(write_result.IsOk());
        const int expect_value                    = 4;
        const std::vector<uint8_t> previous_value = {expect_value, 0, 0, 0},
                                   dummy_value1   = {2, 0, 0, 0};
        const data::Int dummy_value0(7), dummy_value2(5);
        disk::DiskPosition position1(
            disk::DiskPosition(disk::BlockID(filename1, 4), 3));
        dblog::LogOperation log1(transaction_id, position1, previous_value,
                                 dummy_value0);
        ASSERT_TRUE(manager.WriteLog(log1).IsOk());
        disk::DiskPosition position2 =
            disk::DiskPosition(disk::BlockID(filename1, 8), 3);
        dblog::LogOperation log2(0, position2, dummy_value1, dummy_value2);
        ASSERT_TRUE(manager.WriteLog(log2).IsOk());

        Result rollback_result =
            manager.Rollback(transaction_id, buffer_manager);
        EXPECT_TRUE(rollback_result.IsOk()) << rollback_result.Error();
        EXPECT_TRUE(buffer_manager.FlushAll().IsOk());

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
}

TEST_F(RecoveryManagerTwoFileTest, RecoverSuccess) {
    const int block_size = 12;
    dblog::LogManager log_manager(/*log_filename=*/filename0,
                                  /*log_directory_path=*/directory_path,
                                  /*block_size=*/block_size);
    ASSERT_TRUE(log_manager.Init().IsOk());
    recovery::RecoveryManager manager(log_manager);
    disk::DiskManager disk_manager(/*directory_name=*/directory_path,
                                   /*block_size=*/block_size);
    buffer::SimpleBufferManager buffer_manager(/*buffer_size=*/4, disk_manager,
                                               log_manager);
    ASSERT_TRUE(
        disk_manager
            .AllocateNewBlocks(disk::BlockID(filename1, /*block_index=*/9))
            .IsOk());
    dblog::TransactionID rollbacked_transaction_id = 5,
                         committed_transaction_id  = 6;

    dblog::LogTransactionBegin log0(rollbacked_transaction_id);
    ASSERT_TRUE(manager.WriteLog(log0).IsOk());
    dblog::LogTransactionBegin log1(committed_transaction_id);
    ASSERT_TRUE(manager.WriteLog(log1).IsOk());
    const int expect_value0 = 4, expect_value1 = 3, expect_value2 = -7;
    const std::vector<uint8_t> expect_bytes0 = {expect_value0, 0, 0, 0},
                               expect_bytes1 = {expect_value1, 0, 0, 0},
                               expect_bytes2 = {0xf9, 0xff, 0xff, 0xff} /*-7*/,
                               dummy_bytes   = {0, 0, 0, 0};
    const data::Int expected_data0(expect_value0),
        expected_data1(expect_value1), expected_data2(expect_value2),
        dummy_data(0);
    disk::DiskPosition position0(
        disk::DiskPosition(disk::BlockID(filename1, 4), 3));
    dblog::LogOperation log2(committed_transaction_id, position0, dummy_bytes,
                             expected_data0);
    ASSERT_TRUE(manager.WriteLog(log2).IsOk());
    dblog::LogOperation log3(rollbacked_transaction_id, position0,
                             expect_bytes0, dummy_data);
    ASSERT_TRUE(manager.WriteLog(log3).IsOk());
    disk::DiskPosition position1 =
        disk::DiskPosition(disk::BlockID(filename1, 8), 3);
    dblog::LogOperation log4(rollbacked_transaction_id, position1,
                             expect_bytes1, dummy_data);
    ASSERT_TRUE(manager.WriteLog(log4).IsOk());
    disk::DiskPosition position2 =
        disk::DiskPosition(disk::BlockID(filename1, 0), 3);
    dblog::LogOperation log5(committed_transaction_id, position2, dummy_bytes,
                             expected_data2);
    ASSERT_TRUE(manager.WriteLog(log5).IsOk());
    Result commit_result = manager.Commit(committed_transaction_id);
    ASSERT_TRUE(commit_result.IsOk());

    Result recover_result = manager.Recover(buffer_manager);
    EXPECT_TRUE(recover_result.IsOk());
    EXPECT_TRUE(buffer_manager.FlushAll().IsOk());

    disk::Block block(block_size);
    Result read_result = disk_manager.Read(position0.BlockID(), block);
    ASSERT_TRUE(read_result.IsOk());
    ResultV<int> value = block.ReadInt(position0.Offset());
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Get(), expect_value0);

    read_result = disk_manager.Read(position1.BlockID(), block);
    ASSERT_TRUE(read_result.IsOk());
    value = block.ReadInt(position1.Offset());
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Get(), expect_value1);

    read_result = disk_manager.Read(position2.BlockID(), block);
    ASSERT_TRUE(read_result.IsOk());
    value = block.ReadInt(position2.Offset());
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Get(), expect_value2);
}