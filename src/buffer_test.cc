#include "buffer.h"
#include "disk.h"
#include "macro_test.h"
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
    buffer::Buffer buf(block_id, block);
    EXPECT_EQ(buf.Block().Content(), block.Content());
}

TEST(BufferSetBlock, CorrectlySetBlock) {
    const disk::Block block(/*block_size=*/7, "my dbms");
    buffer::Buffer buf;

    buf.SetBlock(block, /*lsn=*/0);
    EXPECT_EQ(buf.Block().Content(), block.Content());
}

TWO_FILE_EXISTENT_TEST(BufferManagerTest, "hello ", "");
FILE_NONEXISTENT_TEST(NonExistentFileTest);

bool DoesBufferPoolContainTheBlock(
    const std::vector<buffer::Buffer> &buffer_pool,
    const disk::BlockID &block_id) {
    for (const auto buffer : buffer_pool) {
        if (buffer.BlockID() == block_id) return true;
    }
    return false;
}

TEST_F(BufferManagerTest, BufferManagerReadCorrectlyReadAndCached) {
    disk::DiskManager disk_manager(directory_path, /*block_size=*/3);
    dblog::LogManager log_manager(filename1, directory_path,
                                  /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());
    buffer::SimpleBufferManager buffer_manager(/*buffer_size=*/5, disk_manager,
                                               log_manager);
    const disk::BlockID block_id(filename0, 0);
    disk::Block read_block(3, "xxx");

    auto read_result = buffer_manager.Read(block_id, read_block);

    EXPECT_TRUE(read_result.IsOk()) << read_result.Error();
    std::vector<uint8_t> expect = {'h', 'e', 'l'};
    EXPECT_EQ(read_block.Content(), expect);
    EXPECT_TRUE(
        DoesBufferPoolContainTheBlock(buffer_manager.BufferPool(), block_id));
}

TEST_F(NonExistentFileTest, BufferManagerReadFailWhenThereIsNoBlock) {
    disk::DiskManager disk_manager(directory_path, /*block_size=*/3);
    dblog::LogManager log_manager(non_existent_filename, directory_path,
                                  /*block_size=*/20);
    buffer::SimpleBufferManager buffer_manager(/*buffer_size=*/5, disk_manager,
                                               log_manager);
    const disk::BlockID block_id(non_existent_filename, 0);
    disk::Block read_block(3, "xxx");

    EXPECT_TRUE(buffer_manager.Read(block_id, read_block).IsError());
}

TEST_F(BufferManagerTest, BufferManagerWriteCorrectlyWriteAndCached) {
    disk::DiskManager disk_manager(directory_path, /*block_size=*/3);
    dblog::LogManager log_manager(filename1, directory_path,
                                  /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());
    buffer::SimpleBufferManager buffer_manager(/*buffer_size=*/5, disk_manager,
                                               log_manager);
    const disk::BlockID block_id(filename0, 0);
    const disk::Block write_block(3, "aiu");
    disk::Block read_block(3, "xxx");

    auto write_result = buffer_manager.Write(block_id, write_block, /*lsn=*/0);

    EXPECT_TRUE(write_result.IsOk()) << write_result.Error();
    EXPECT_TRUE(
        DoesBufferPoolContainTheBlock(buffer_manager.BufferPool(), block_id));
    ASSERT_TRUE(buffer_manager.Read(block_id, read_block).IsOk());
    std::vector<uint8_t> expect = {'a', 'i', 'u'};
    EXPECT_EQ(read_block.Content(), expect);
}

TEST_F(BufferManagerTest, BufferManagerWriteWriteToCache) {
    disk::DiskManager disk_manager(directory_path, /*block_size=*/3);
    dblog::LogManager log_manager(filename1, directory_path,
                                  /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());
    buffer::SimpleBufferManager buffer_manager(/*buffer_size=*/5, disk_manager,
                                               log_manager);
    const disk::BlockID block_id(filename0, 0);
    const disk::Block write_block(3, "aiu");
    disk::Block read_block(3, "xxx");
    auto read_result = buffer_manager.Read(block_id, read_block);
    ASSERT_TRUE(read_result.IsOk()) << read_result.Error();
    ASSERT_TRUE(
        DoesBufferPoolContainTheBlock(buffer_manager.BufferPool(), block_id));

    // Write to buffer pool here (a.k.a cache)
    auto write_result = buffer_manager.Write(block_id, write_block, /*lsn=*/0);
    EXPECT_TRUE(write_result.IsOk()) << write_result.Error();

    read_result = buffer_manager.Read(block_id, read_block);
    ASSERT_TRUE(read_result.IsOk()) << read_result.Error();
    std::vector<uint8_t> expect = {'a', 'i', 'u'};
    EXPECT_EQ(read_block.Content(), expect);
}

TEST_F(BufferManagerTest, BufferManagerCorrectlyFlush) {
    const int BLOCK_SIZE = 4;
    disk::DiskManager disk_manager(directory_path, BLOCK_SIZE);
    dblog::LogManager log_manager(filename1, directory_path,
                                  /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());
    buffer::SimpleBufferManager buffer_manager(/*buffer_size=*/5, disk_manager,
                                               log_manager);
    const disk::BlockID block_id(filename0, 0);
    char expect_block_content[BLOCK_SIZE] = "aiu";
    const disk::Block write_block(BLOCK_SIZE, expect_block_content);

    EXPECT_TRUE(buffer_manager.Write(block_id, write_block, /*lsn=*/0).IsOk());
    auto flush_result = buffer_manager.Flush(block_id);

    EXPECT_TRUE(flush_result.IsOk()) << flush_result.Error();
    std::ifstream file(directory_path + filename0, std::ios::binary);
    char actual_block_content[BLOCK_SIZE];
    file.read(actual_block_content, BLOCK_SIZE);
    EXPECT_FALSE(file.fail());
    EXPECT_STREQ(actual_block_content, expect_block_content)
        << "actual_block_content: " << actual_block_content;
    file.close();
}

TEST_F(BufferManagerTest, BufferManagerFlushesAllBuffers) {
    const int BLOCK_SIZE = 4;
    disk::DiskManager disk_manager(directory_path, BLOCK_SIZE);
    dblog::LogManager log_manager(filename1, directory_path,
                                  /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());
    buffer::SimpleBufferManager buffer_manager(/*buffer_size=*/5, disk_manager,
                                               log_manager);
    const disk::BlockID block_id0(filename0, 0), block_id1(filename0, 1);
    char expect_block_content[BLOCK_SIZE] = "aiu";
    const disk::Block write_block(BLOCK_SIZE, expect_block_content);
    ASSERT_TRUE(buffer_manager.Write(block_id0, write_block, /*lsn=*/0).IsOk());
    ASSERT_TRUE(buffer_manager.Write(block_id1, write_block, /*lsn=*/0).IsOk());

    Result flush_result = buffer_manager.FlushAll();

    EXPECT_TRUE(flush_result.IsOk()) << flush_result.Error();
    std::ifstream file(directory_path + filename0, std::ios::binary);
    char actual_block_content[2 * BLOCK_SIZE];
    file.read(actual_block_content, 2 * BLOCK_SIZE);
    EXPECT_FALSE(file.fail());
    EXPECT_STREQ(actual_block_content, expect_block_content)
        << "actual_block_content: " << actual_block_content;
    EXPECT_STREQ(actual_block_content + BLOCK_SIZE, expect_block_content)
        << "actual_block_content: " << actual_block_content;
    file.close();
}

TEST_F(NonExistentFileTest, BufferManagerFlushFailWhenThereIsNoBlock) {
    const int BLOCK_SIZE = 4;
    disk::DiskManager disk_manager(directory_path, BLOCK_SIZE);
    dblog::LogManager log_manager(non_existent_filename, directory_path,
                                  /*block_size=*/20);
    buffer::SimpleBufferManager buffer_manager(/*buffer_size=*/5, disk_manager,
                                               log_manager);
    const disk::BlockID block_id(non_existent_filename, 0);

    EXPECT_TRUE(buffer_manager.Flush(block_id).IsError());
}

TEST_F(BufferManagerTest, LRUBufferManagerReadsCorrectly) {
    disk::DiskManager disk_manager(directory_path, /*block_size=*/3);
    dblog::LogManager log_manager(filename1, directory_path,
                                  /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());
    buffer::LRUBufferManager buffer_manager(/*buffer_size=*/5, disk_manager,
                                            log_manager);
    const disk::BlockID block_id(filename0, 0);
    disk::Block read_block(3, "xxx");

    auto read_result = buffer_manager.Read(block_id, read_block);

    EXPECT_TRUE(read_result.IsOk()) << read_result.Error();
    std::vector<uint8_t> expect = {'h', 'e', 'l'};
    EXPECT_EQ(read_block.Content(), expect);
}

TEST_F(BufferManagerTest, LRUBufferManagerWritesCorrectly) {
    disk::DiskManager disk_manager(directory_path, /*block_size=*/3);
    dblog::LogManager log_manager(filename1, directory_path,
                                  /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());
    buffer::LRUBufferManager buffer_manager(/*buffer_size=*/5, disk_manager,
                                            log_manager);
    const disk::BlockID block_id(filename0, 0);
    const disk::Block write_block(3, "aiu");
    disk::Block read_block(3, "xxx");

    auto write_result = buffer_manager.Write(block_id, write_block, /*lsn=*/0);

    EXPECT_TRUE(write_result.IsOk()) << write_result.Error();
    ASSERT_TRUE(buffer_manager.Read(block_id, read_block).IsOk());
    std::vector<uint8_t> expect = {'a', 'i', 'u'};
    EXPECT_EQ(read_block.Content(), expect);
}