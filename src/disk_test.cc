#include "disk.h"
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

class TempFileTest : public ::testing::Test {
  protected:
    TempFileTest() : directory_path("parent-dir/"), filename("metadata.tbl") {
        if (!std::filesystem::exists(directory_path)) {
            std::filesystem::create_directories(directory_path);
        }

        std::ofstream file(directory_path + filename);
        if (file.is_open()) {
            file << "hello ";
            file.close();
        }
    }

    virtual ~TempFileTest() override {
        std::filesystem::remove_all(directory_path);
    }

    const std::string directory_path;
    const std::string filename;
};

class NonExistentFileTest : public ::testing::Test {
  protected:
    NonExistentFileTest()
        : directory_path("non-existent-directory/"),
          non_existent_filename("deleted.txt") {
        if (std::filesystem::exists(directory_path + non_existent_filename)) {
            remove((directory_path + non_existent_filename).c_str());
        }
    }

    virtual ~NonExistentFileTest() override {
        if (std::filesystem::exists(directory_path)) {
            std::filesystem::remove_all(directory_path);
        }
    }

    const std::string directory_path;
    const std::string non_existent_filename;
};

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