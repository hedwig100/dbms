#include "buffer.h"
#include "disk.h"
#include "result.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

TEST(BufferBlockID, CorrectlyReturnsBlockID) {
    const disk::BlockID block_id("filename", 1);
    const disk::Block block(/*block_size=*/7, "my dbms");
    const buffer::Buffer buf(block_id, block);
    EXPECT_EQ(buf.BlockID(), block_id);
}

TEST(BufferBlock, CorrectlyReturnsBlock) {
    const disk::BlockID block_id("filename", 1);
    const disk::Block block(/*block_size=*/7, "my dbms");
    const buffer::Buffer buf(block_id, block);
    EXPECT_EQ(buf.Block().Content(), block.Content());
}

TEST(BufferSetBlock, CorrectlySetBlock) {
    const disk::Block block(/*block_size=*/7, "my dbms");
    buffer::Buffer buf;

    buf.SetBlock(block);
    EXPECT_EQ(buf.Block().Content(), block.Content());
}

// TODO: move TempFilTest of disk_test.cc to util direcoty and use it from
// there.
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

// TODO: move NonExistentFileTest of disk_test.cc to util direcoty and use it
// from there.
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

bool DoesBufferPoolContainTheBlock(
    const std::vector<buffer::Buffer> &buffer_pool,
    const disk::BlockID &block_id) {
    for (const auto buffer : buffer_pool) {
        if (buffer.BlockID() == block_id) return true;
    }
    return false;
}

TEST_F(TempFileTest, BufferManagerReadCorrectlyReadAndCached) {
    const disk::DiskManager disk_manager(directory_path, /*block_size=*/3);
    buffer::BufferManager buffer_manager(/*buffer_size=*/5, disk_manager);
    const disk::BlockID block_id(filename, 0);
    disk::Block read_block(3, "xxx");

    auto read_result = buffer_manager.Read(block_id, read_block);
    EXPECT_TRUE(read_result.IsOk()) << read_result.Error();

    std::vector<uint8_t> expect = {'h', 'e', 'l'};
    EXPECT_EQ(read_block.Content(), expect);
    EXPECT_TRUE(
        DoesBufferPoolContainTheBlock(buffer_manager.BufferPool(), block_id));
}

TEST_F(NonExistentFileTest, BufferManagerReadFailWhenThereIsNoBlock) {
    const disk::DiskManager disk_manager(directory_path, /*block_size=*/3);
    buffer::BufferManager buffer_manager(/*buffer_size=*/5, disk_manager);
    const disk::BlockID block_id(non_existent_filename, 0);
    disk::Block read_block(3, "xxx");

    EXPECT_TRUE(buffer_manager.Read(block_id, read_block).IsError());
}

TEST_F(TempFileTest, BufferManagerWriteCorrectlyWriteAndCached) {
    const disk::DiskManager disk_manager(directory_path, /*block_size=*/3);
    buffer::BufferManager buffer_manager(/*buffer_size=*/5, disk_manager);
    const disk::BlockID block_id(filename, 0);
    const disk::Block write_block(3, "aiu");
    disk::Block read_block(3, "xxx");

    auto write_result = buffer_manager.Write(block_id, write_block);
    EXPECT_TRUE(write_result.IsOk()) << write_result.Error();
    EXPECT_TRUE(
        DoesBufferPoolContainTheBlock(buffer_manager.BufferPool(), block_id));

    ASSERT_TRUE(buffer_manager.Read(block_id, read_block).IsOk());
    std::vector<uint8_t> expect = {'a', 'i', 'u'};
    EXPECT_EQ(read_block.Content(), expect);
}

TEST_F(TempFileTest, BufferManagerWriteWriteToCache) {
    const disk::DiskManager disk_manager(directory_path, /*block_size=*/3);
    buffer::BufferManager buffer_manager(/*buffer_size=*/5, disk_manager);
    const disk::BlockID block_id(filename, 0);
    const disk::Block write_block(3, "aiu");
    disk::Block read_block(3, "xxx");

    auto read_result = buffer_manager.Read(block_id, read_block);
    ASSERT_TRUE(read_result.IsOk()) << read_result.Error();
    ASSERT_TRUE(
        DoesBufferPoolContainTheBlock(buffer_manager.BufferPool(), block_id));

    // Write to buffer pool here (a.k.a cache)
    auto write_result = buffer_manager.Write(block_id, write_block);
    EXPECT_TRUE(write_result.IsOk()) << write_result.Error();

    read_result = buffer_manager.Read(block_id, read_block);
    ASSERT_TRUE(read_result.IsOk()) << read_result.Error();
    std::vector<uint8_t> expect = {'a', 'i', 'u'};
    EXPECT_EQ(read_block.Content(), expect);
}

TEST_F(TempFileTest, BufferManagerFlushCorrectlyFlush) {
    const int BLOCK_SIZE = 4;
    const disk::DiskManager disk_manager(directory_path, BLOCK_SIZE);
    buffer::BufferManager buffer_manager(/*buffer_size=*/5, disk_manager);
    const disk::BlockID block_id(filename, 0);
    char expect_block_content[BLOCK_SIZE] = "aiu";
    const disk::Block write_block(BLOCK_SIZE, expect_block_content);

    EXPECT_TRUE(buffer_manager.Write(block_id, write_block).IsOk());
    EXPECT_TRUE(buffer_manager.Flush(block_id).IsOk());

    std::ifstream file(directory_path + filename, std::ios::binary);
    char actual_block_content[BLOCK_SIZE];
    file.read(actual_block_content, BLOCK_SIZE);
    EXPECT_FALSE(file.fail());
    EXPECT_STREQ(actual_block_content, expect_block_content)
        << "actual_block_content: " << actual_block_content;
    file.close();
}

TEST_F(NonExistentFileTest, BufferManagerFlushFailWhenThereIsNoBlock) {
    const int BLOCK_SIZE = 4;
    const disk::DiskManager disk_manager(directory_path, BLOCK_SIZE);
    buffer::BufferManager buffer_manager(/*buffer_size=*/5, disk_manager);
    const disk::BlockID block_id(non_existent_filename, 0);
    const disk::Block write_block(BLOCK_SIZE, "aiu");

    EXPECT_TRUE(buffer_manager.Flush(block_id).IsError());
}
