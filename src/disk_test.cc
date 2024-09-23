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

TEST(BlockID, CorrectlyAdvanceBlockID) {
    disk::BlockID block_id0("filename", 3);

    auto block_id1 = block_id0 + 3;
    EXPECT_EQ(block_id1.Filename(), "filename");
    EXPECT_EQ(block_id1.BlockIndex(), 6);

    block_id1 += 5;
    EXPECT_EQ(block_id1.Filename(), "filename");
    EXPECT_EQ(block_id1.BlockIndex(), 11);
}

TEST(BlockID, CorrectlyStepBackwardBlockID) {
    disk::BlockID block_id0("filename", 9);

    auto block_id1 = block_id0 - 3;
    EXPECT_EQ(block_id1.Filename(), "filename");
    EXPECT_EQ(block_id1.BlockIndex(), 6);

    block_id1 -= 3;
    EXPECT_EQ(block_id1.Filename(), "filename");
    EXPECT_EQ(block_id1.BlockIndex(), 3);
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

TEST(BlockID, CorrectlyTestNotEqual) {
    const disk::BlockID block_id0("file0.tbl", 0), block_id1("file0.tbl", 0),
        block_id2("file1.tbl", 0), block_id3("file0.tbl", 1),
        block_id4("file1.tbl", 1);

    EXPECT_FALSE(block_id0 != block_id1);
    EXPECT_TRUE(block_id0 != block_id2);
    EXPECT_TRUE(block_id0 != block_id3);
    EXPECT_TRUE(block_id0 != block_id4);
}

TEST(DiskPosition, InstantiationSuccess) {
    disk::DiskPosition position(disk::BlockID("filename", 1), 3);

    EXPECT_EQ(position.BlockID().Filename(), "filename");
    EXPECT_EQ(position.BlockID().BlockIndex(), 1);
    EXPECT_EQ(position.Offset(), 3);
}

TEST(DiskPosition, MoveForward) {
    disk::DiskPosition position(disk::BlockID("filename", 1), 3);

    // Inside the block
    auto moved_position = position.Move(/*displacement=*/1, /*block_size=*/5);
    EXPECT_EQ(moved_position.BlockID().BlockIndex(), 1);
    EXPECT_EQ(moved_position.Offset(), 4);

    // Move next block
    moved_position = position.Move(/*displacement=*/5, /*block_size=*/5);
    EXPECT_EQ(moved_position.BlockID().BlockIndex(), 2);
    EXPECT_EQ(moved_position.Offset(), 3);

    // Move next block with offset 0
    moved_position = position.Move(/*displacement=*/7, /*block_size=*/5);
    EXPECT_EQ(moved_position.BlockID().BlockIndex(), 3);
    EXPECT_EQ(moved_position.Offset(), 0);
}

TEST(DiskPosition, MoveBackward) {
    disk::DiskPosition position(disk::BlockID("filename", 2), 3);

    // Inside the block
    auto moved_position = position.Move(/*displacement=*/-1, /*block_size=*/5);
    EXPECT_EQ(moved_position.BlockID().BlockIndex(), 2);
    EXPECT_EQ(moved_position.Offset(), 2);

    // Move next block
    moved_position = position.Move(/*displacement=*/-6, /*block_size=*/5);
    EXPECT_EQ(moved_position.BlockID().BlockIndex(), 1);
    EXPECT_EQ(moved_position.Offset(), 2);

    // Move next block with offset 0
    moved_position = position.Move(/*displacement=*/-13, /*block_size=*/5);
    EXPECT_EQ(moved_position.BlockID().BlockIndex(), 0);
    EXPECT_EQ(moved_position.Offset(), 0);
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

TEST(Block, CorrectBlockSize) {
    char hello[] = "hello";
    const disk::Block block(5, hello);
    EXPECT_EQ(block.BlockSize(), 5);
}

TEST(Block, ReadByteWithOutsideIndex) {
    char hello[] = "hello";
    const disk::Block block(5, hello);
    EXPECT_TRUE(block.ReadByte(5).IsError());
    EXPECT_TRUE(block.ReadByte(6).IsError());
}

TEST(Block, CorrectlyWriteByte) {
    disk::Block block(10);
    EXPECT_TRUE(block.WriteByte(0, 'h').IsOk());
    EXPECT_TRUE(block.WriteByte(1, 'e').IsOk());

    EXPECT_EQ(block.ReadByte(0).Get(), 'h');
    EXPECT_EQ(block.ReadByte(1).Get(), 'e');
}

TEST(Block, WriteByteWithOutsideIndex) {
    disk::Block block(3);
    EXPECT_TRUE(block.WriteByte(-1, 'a').IsError());
    EXPECT_TRUE(block.WriteByte(3, 'b').IsError());
    EXPECT_TRUE(block.WriteByte(5, 'b').IsError());
}

TEST(Block, CorrectlyReadBytes) {
    char hello[] = "hello";
    const disk::Block block(5, hello);

    std::vector<uint8_t> read_value;
    EXPECT_TRUE(block.ReadBytes(1, /*length=*/3, read_value).IsOk());

    const std::vector<uint8_t> expect_value = {'e', 'l', 'l'};
    EXPECT_EQ(read_value, expect_value);
}

TEST(BLock, ReadBytesWithOutsideIndex) {
    const disk::Block block(3);

    std::vector<uint8_t> read_value;
    EXPECT_TRUE(
        block.ReadBytes(/*offset=*/-1, /*length=*/2, read_value).IsError());
    EXPECT_TRUE(
        block.ReadBytes(/*offset=*/3, /*length=*/2, read_value).IsError());
    EXPECT_TRUE(
        block.ReadBytes(/*offset=*/5, /*length=*/2, read_value).IsError());
}

TEST(Block, CorrectlyWriteBytes) {
    disk::Block block(10);
    const std::vector<uint8_t> write_value = {'h', 'e', 'l', 'l', 'o'};

    EXPECT_TRUE(
        block.WriteBytes(/*offset=*/3, /*length=*/2, write_value).IsOk());
    EXPECT_EQ(block.ReadByte(3).Get(), 'h');
    EXPECT_EQ(block.ReadByte(4).Get(), 'e');

    EXPECT_TRUE(
        block.WriteBytes(/*offset=*/5, /*length=*/5, write_value).IsOk());
    EXPECT_EQ(block.ReadByte(3).Get(), 'h');
    EXPECT_EQ(block.ReadByte(4).Get(), 'e');
    EXPECT_EQ(block.ReadByte(5).Get(), 'h');
    EXPECT_EQ(block.ReadByte(6).Get(), 'e');
    EXPECT_EQ(block.ReadByte(7).Get(), 'l');
    EXPECT_EQ(block.ReadByte(8).Get(), 'l');
    EXPECT_EQ(block.ReadByte(9).Get(), 'o');
}

TEST(Block, WriteBytesWithOutsideIndex) {
    disk::Block block(3);
    const std::vector<uint8_t> write_value = {'h'};

    EXPECT_TRUE(
        block.WriteBytes(/*offset=*/-1, /*length=*/1, write_value).IsError());
    EXPECT_TRUE(
        block.WriteBytes(/*offset=*/3, /*length=*/1, write_value).IsError());
    EXPECT_TRUE(
        block.WriteBytes(/*offset=*/1, /*length=*/2, write_value).IsError());
}

TEST(Block, WriteBytesWithOffsetSuccess) {
    disk::Block block(5);
    const std::vector<uint8_t> write_value  = {'a', 'b', 'c', 'd'},
                               expect_value = {'c', 'd'};

    EXPECT_TRUE(block.WriteBytesWithOffset(1, write_value, 2).IsOk());

    std::vector<uint8_t> read_value(2);
    auto read_result = block.ReadBytes(1, 2, read_value);
    ASSERT_TRUE(read_result.IsOk());
    EXPECT_EQ(read_value, expect_value);
}

TEST(Block, WriteBytesWithOffsetTooLong) {
    disk::Block block(5);
    const std::vector<uint8_t> write_value  = {'a', 'b', 'c', 'd', 'e', 'f'},
                               expect_value = {'a', 'b', 'c', 'd'};

    auto write_result = block.WriteBytesWithOffset(
        /*offset=*/1, /*value=*/write_value, /*value_offset=*/0);
    EXPECT_TRUE(write_result.IsError());
    EXPECT_EQ(write_result.Error(), 4);

    std::vector<uint8_t> read_value(4);
    auto read_result = block.ReadBytes(1, 4, read_value);
    ASSERT_TRUE(read_result.IsOk());
    EXPECT_EQ(read_value, expect_value);
}

TEST(Block, CorrectlyReadInt) {
    disk::Block block(5);

    ASSERT_TRUE(block.WriteByte(1, /*0b01010110=0x56=*/'V').IsOk());

    auto expect_int = block.ReadInt(1);
    EXPECT_TRUE(expect_int.IsOk());
    EXPECT_EQ(expect_int.Get(), /*0b01010110=*/86);

    expect_int = block.ReadInt(0);
    EXPECT_TRUE(expect_int.IsOk());
    EXPECT_EQ(expect_int.Get(), /*0b0101011000000000=*/86 << 8);
}

TEST(Block, ReadIntWithOutsideIndex) {
    disk::Block block(5);

    EXPECT_TRUE(block.ReadInt(-1).IsError());
    EXPECT_TRUE(block.ReadInt(2).IsError());
}

TEST(Block, CorrectlyWriteInt) {
    disk::Block block(10);

    EXPECT_TRUE(block.WriteInt(5, 1313109832).IsOk());

    auto expect_int = block.ReadInt(5);
    ASSERT_TRUE(expect_int.IsOk());
    EXPECT_EQ(expect_int.Get(), 1313109832);
}

TEST(Block, WriteIntWithOutsideIndex) {
    disk::Block block(10);
    EXPECT_TRUE(block.WriteInt(-1, 0).IsError());
    EXPECT_TRUE(block.WriteInt(7, 0).IsError());
    EXPECT_TRUE(block.WriteInt(9, 0).IsError());
}

TEST(Block, CorrectlyReadString) {
    disk::Block block(10, "abcdefghi");

    auto expect_str = block.ReadString(/*offset=*/1, /*length=*/4);
    EXPECT_TRUE(expect_str.IsOk());
    EXPECT_EQ(expect_str.Get(), "bcde");

    expect_str = block.ReadString(/*offset=*/4, /*length=*/1);
    EXPECT_TRUE(expect_str.IsOk());
    EXPECT_EQ(expect_str.Get(), "e");
}

TEST(Block, ReadStringWithOutsideIndex) {
    disk::Block block(10, "abcdefghi");

    EXPECT_TRUE(block.ReadString(/*offset=*/-1, /*length=*/3).IsError());
    EXPECT_TRUE(block.ReadString(/*offset=*/8, /*length=*/3).IsError());
}

TEST(Block, CorrectlyWriteString) {
    disk::Block block(20);

    EXPECT_TRUE(block.WriteString(/*offset=*/3, "abc").IsOk());

    auto expect_str = block.ReadString(/*offset=*/3, /*length=*/3);
    ASSERT_TRUE(expect_str.IsOk());
    EXPECT_EQ(expect_str.Get(), "abc");
    expect_str = block.ReadString(/*offset=*/5, /*length=*/1);
    ASSERT_TRUE(expect_str.IsOk());
    EXPECT_EQ(expect_str.Get(), "c");
}

TEST(Block, WriteStringWithOutsideIndex) {
    disk::Block block(20);

    EXPECT_TRUE(block.WriteString(/*offset=*/-1, "").IsError());
    EXPECT_TRUE(block.WriteString(/*offset=*/9, "aaaaaaaaaaaa").IsError());
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
    disk::Block block_write(3, goodbye), block_read0, block_read1;

    EXPECT_TRUE(
        disk_manager.Write(disk::BlockID(filename, 0), block_write).IsOk());
    EXPECT_TRUE(
        disk_manager.Write(disk::BlockID(filename, 1), block_write).IsOk());

    EXPECT_TRUE(
        disk_manager.Read(disk::BlockID(filename, 0), block_read0).IsOk());
    EXPECT_TRUE(
        disk_manager.Read(disk::BlockID(filename, 1), block_read1).IsOk());

    // block_read must be "goo"
    EXPECT_EQ(block_read0.ReadByte(0).Get(), 'g');
    EXPECT_EQ(block_read0.ReadByte(1).Get(), 'o');
    EXPECT_EQ(block_read0.ReadByte(2).Get(), 'o');
    EXPECT_TRUE(block_read0.ReadByte(3).IsError());

    // block must be "goo"
    EXPECT_EQ(block_read1.ReadByte(0).Get(), 'g');
    EXPECT_EQ(block_read1.ReadByte(1).Get(), 'o');
    EXPECT_EQ(block_read1.ReadByte(2).Get(), 'o');
    EXPECT_TRUE(block_read1.ReadByte(3).IsError());
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
              (block_index + 1) * block_size);
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
        (block_index + 1) * block_size);
}

TEST_F(
    TempFileTest,
    DiskManagerCorrectlyReadAndWriteBytesIncludingNullCharacterAndTrailingSpace) {
    const size_t block_size = 32;
    disk::DiskManager manager(/*directory_path=*/directory_path,
                              /*block_size=*/block_size);
    ASSERT_TRUE(manager.AllocateNewBlocks(disk::BlockID(filename, 64)).IsOk());

    disk::BlockID block_id(filename, 1);
    disk::Block block(block_size, "abc ");
    block.WriteByte(/*offset=*/1, '\0');
    auto original_content = block.Content();
    ASSERT_TRUE(manager.Write(block_id, block).IsOk());
    ASSERT_TRUE(manager.Read(block_id, block).IsOk());

    EXPECT_EQ(block.Content(), original_content);
}

TEST_F(TempFileTest, DiskReadBytesAcrossBlocksOneBlockSuccess) {
    const size_t block_size = 5;
    const disk::DiskManager manager(directory_path, block_size);
    disk::BlockID block_id(filename, 0);
    disk::Block block;
    int offset = 2;

    auto result = manager.Read(block_id, block);
    ASSERT_TRUE(result.IsOk()) << result.Error() << '\n';

    std::vector<uint8_t> bytes;
    result =
        disk::ReadBytesAcrossBlocks(block_id, offset, block, 2, bytes, manager);
    EXPECT_TRUE(result.IsOk()) << result.Error() << '\n';

    EXPECT_EQ(bytes.size(), 2);
    const std::string expect_content = "ll"; // he'll'o
    const std::vector<uint8_t> expect_bytes(expect_content.begin(),
                                            expect_content.end());
    EXPECT_EQ(bytes, expect_bytes);
}

FILE_EXISTENT_TEST(
    LongFileTest,
    "aaaaabbbbbcccccdddddeeeeefffffggggghhhhhiiiiijjjjjkkkkklllllmmmmmnnnnnoooo"
    "opppppqqqqqrrrrrssssstttttuuuuuvvvvvwwwwwxxxxxyyyyyzzzzz");

TEST_F(LongFileTest, DiskReadBytesAcrossBlocksLongLengthSuccess) {
    const size_t block_size = 5;
    const disk::DiskManager manager(directory_path, block_size);
    disk::BlockID block_id(filename, 1);
    disk::Block block;
    int offset = 2;

    auto result = manager.Read(block_id, block);
    ASSERT_TRUE(result.IsOk()) << result.Error() << '\n';

    std::vector<uint8_t> bytes;
    result = disk::ReadBytesAcrossBlocks(block_id, offset, block, 34, bytes,
                                         manager);
    EXPECT_TRUE(result.IsOk()) << result.Error() << '\n';

    EXPECT_EQ(bytes.size(), 34);
    const std::string expect_content = "bbbcccccdddddeeeeefffffggggghhhhhi";
    const std::vector<uint8_t> expect_bytes(expect_content.begin(),
                                            expect_content.end());
    EXPECT_EQ(bytes, expect_bytes);
}