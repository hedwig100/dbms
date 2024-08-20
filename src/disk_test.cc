#include "disk.h"
#include "macro_test.h"
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>

TEST(BlockID, ReturnsCorrectFilename) {
    const disk::BlockID block_id("metadata.tbl", 0);
    EXPECT_EQ(block_id.Filename(), "metadata.tbl");
}

TEST(BlockID, ReturnsCorrectBlockIndex) {
    const disk::BlockID block_id("metatable.tbl", 0);
    EXPECT_EQ(block_id.BlockIndex(), 0);
}

TEST(BlockID, CorrectlyCompare) {
    const disk::BlockID block_id0("file0.tbl", 0), block_id1("file0.tbl", 0),
        block_id2("file1.tbl", 0), block_id3("file0.tbl", 1),
        block_id4("file1.tbl", 1);

    EXPECT_TRUE(block_id0 == block_id1);
    EXPECT_FALSE(block_id0 == block_id2);
    EXPECT_FALSE(block_id0 == block_id3);
    EXPECT_FALSE(block_id0 == block_id4);
}

TEST(Block, InstanciationAndReadByte) {
    char hello[] = "hello";
    const disk::Block block(5, hello);
    EXPECT_EQ(block.ReadByte(0).Get(), 'h');
    EXPECT_EQ(block.ReadByte(1).Get(), 'e');
    EXPECT_EQ(block.ReadByte(2).Get(), 'l');
    EXPECT_EQ(block.ReadByte(3).Get(), 'l');
    EXPECT_EQ(block.ReadByte(4).Get(), 'o');
}

TEST(Block, ReadByteWithOutsideIndex) {
    char hello[] = "hello";
    const disk::Block block(5, hello);
    EXPECT_TRUE(block.ReadByte(5).IsError());
    EXPECT_TRUE(block.ReadByte(6).IsError());
}

FILE_EXISTENT_TEST(TempFileTest, "hello ");
FILE_NONEXISTENT_TEST(NonExistentFileTest);

TEST_F(TempFileTest, DiskManagerCorrectlyReads) {
    const disk::DiskManager disk_manager(directory_path, /*block_size=*/3);
    disk::Block block;

    EXPECT_TRUE(disk_manager.Read(disk::BlockID(filename, 0), block).IsOk());

    // block must be "hel"
    EXPECT_EQ(block.ReadByte(0).Get(), 'h');
    EXPECT_EQ(block.ReadByte(1).Get(), 'e');
    EXPECT_EQ(block.ReadByte(2).Get(), 'l');
    EXPECT_TRUE(block.ReadByte(3).IsError());

    EXPECT_TRUE(disk_manager.Read(disk::BlockID(filename, 1), block).IsOk());

    // block must be "lo "
    EXPECT_EQ(block.ReadByte(0).Get(), 'l');
    EXPECT_EQ(block.ReadByte(1).Get(), 'o');
    EXPECT_EQ(block.ReadByte(2).Get(), ' ');
    EXPECT_TRUE(block.ReadByte(3).IsError());
}

TEST_F(NonExistentFileTest, DiskManagerReadFail) {
    const disk::DiskManager disk_manager(directory_path,
                                         /*block_size=*/3);
    disk::Block block;

    EXPECT_TRUE(
        disk_manager.Read(disk::BlockID(non_existent_filename, 0), block)
            .IsError());
}

TEST_F(TempFileTest, DiskManagerCorrectlyWrites) {
    const disk::DiskManager disk_manager(directory_path, /*block_size=*/3);

    char goodbye[] = "goodbye";
    disk::Block block_write(3, goodbye), block_read;

    EXPECT_TRUE(
        disk_manager.Write(disk::BlockID(filename, 0), block_write).IsOk());
    EXPECT_TRUE(
        disk_manager.Read(disk::BlockID(filename, 0), block_read).IsOk());

    // block_read must be "goo"
    EXPECT_EQ(block_read.ReadByte(0).Get(), 'g');
    EXPECT_EQ(block_read.ReadByte(1).Get(), 'o');
    EXPECT_EQ(block_read.ReadByte(2).Get(), 'o');
    EXPECT_TRUE(block_read.ReadByte(3).IsError());

    EXPECT_TRUE(
        disk_manager.Write(disk::BlockID(filename, 1), block_write).IsOk());
    EXPECT_TRUE(
        disk_manager.Read(disk::BlockID(filename, 1), block_read).IsOk());

    // block must be "goo"
    EXPECT_EQ(block_read.ReadByte(0).Get(), 'g');
    EXPECT_EQ(block_read.ReadByte(1).Get(), 'o');
    EXPECT_EQ(block_read.ReadByte(2).Get(), 'o');
    EXPECT_TRUE(block_read.ReadByte(3).IsError());
}

TEST_F(NonExistentFileTest, DiskManagerWriteFail) {
    const disk::DiskManager disk_manager(directory_path,
                                         /*block_size=*/3);
    disk::Block block;

    EXPECT_TRUE(
        disk_manager.Write(disk::BlockID(non_existent_filename, 0), block)
            .IsError());
}

TEST_F(TempFileTest, DiskManagerFlushSucceeds) {
    const disk::DiskManager disk_manager(directory_path, /*block_size=*/3);
    EXPECT_TRUE(disk_manager.Flush(filename).IsOk());
}

TEST_F(NonExistentFileTest, DiskManagerFlushFails) {
    const disk::DiskManager disk_manager(directory_path, /*block_size=*/3);
    EXPECT_TRUE(disk_manager.Flush(non_existent_filename).IsError());
}

TEST_F(TempFileTest, DiskManagerSizeSucceeds) {
    const disk::DiskManager disk_manager(directory_path, /*block_size=*/3);
    const auto expect_ok = disk_manager.Size(filename);
    EXPECT_TRUE(expect_ok.IsOk());
    EXPECT_EQ(expect_ok.Get(), 2);
}

TEST_F(NonExistentFileTest, DiskManagerSizeFails) {
    const disk::DiskManager disk_manager(directory_path, /*block_size=*/3);
    const auto expect_ok = disk_manager.Size(non_existent_filename);
    EXPECT_TRUE(expect_ok.IsError());
}

TEST_F(TempFileTest, DiskManagerAllocateNewFileSucceeds) {
    const int block_size  = 3;
    const int block_index = 10;

    const disk::DiskManager disk_manager(directory_path, block_size);
    EXPECT_TRUE(
        disk_manager.AllocateNewBlocks(disk::BlockID(filename, block_index))
            .IsOk());
    EXPECT_EQ(std::filesystem::file_size(directory_path + filename),
              block_index * block_size);
}

TEST_F(NonExistentFileTest, DiskManagerAllocateNewFileSucceedsWithoutFile) {
    const int block_size  = 3;
    const int block_index = 10;

    const disk::DiskManager disk_manager(directory_path, block_size);
    EXPECT_TRUE(disk_manager
                    .AllocateNewBlocks(
                        disk::BlockID(non_existent_filename, block_index))
                    .IsOk());
    EXPECT_EQ(
        std::filesystem::file_size(directory_path + non_existent_filename),
        block_index * block_size);
}