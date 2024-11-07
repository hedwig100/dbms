#include "data/int.h"
#include "log.h"
#include "log_record.h"
#include "macro_test.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <vector>

TEST(LogLogBlock, InstantiateOffset) {
    using namespace dblog::internal;
    LogBlock block(32);

    EXPECT_EQ(block.Offset(), 4);
}

TEST(LogLogBlock, AppendSuccess) {
    using namespace dblog::internal;
    LogBlock block(32);

    std::vector<uint8_t> bytes = {'a', 'b', 'c'};
    EXPECT_TRUE(block.Append(bytes, /*bytes_offset=*/1).IsOk());
    EXPECT_EQ(block.Offset(), 6);
    std::vector<uint8_t> raw_content = block.RawBlock().Content();
    EXPECT_EQ(raw_content[4], 'b');
    EXPECT_EQ(raw_content[5], 'c');
}

TEST(LogLogBlock, AppendTooLongFail) {
    using namespace dblog::internal;
    LogBlock block(6);

    std::vector<uint8_t> bytes = {'a', 'b', 'c', 'd'};
    auto append_result         = block.Append(bytes, /*bytes_offset=*/1);
    EXPECT_TRUE(append_result.IsError());
    EXPECT_EQ(append_result.Error(), 3);
    EXPECT_EQ(block.Offset(), 6);
    std::vector<uint8_t> raw_content = block.RawBlock().Content();
    EXPECT_EQ(raw_content[4], 'b');
    EXPECT_EQ(raw_content[5], 'c');
}

FILE_EXISTENT_TEST(LogFileExistentTest, "0001 block content ----");
constexpr size_t kOffset0001 = ('1' << 24) | ('0' << 16) | ('0' << 8) | '0';
const std::vector<uint8_t> kExpectBlockContentSize10 = {
    '0', '0', '0', '1', ' ', 'b', 'l', 'o', 'c', 'k'};

TEST_F(LogFileExistentTest, ReadLogBlockSuccess) {
    using namespace dblog::internal;
    const size_t block_size = 10;
    LogBlock block(block_size);
    disk::DiskManager disk_manager(directory_path, block_size);

    auto read_result =
        block.ReadLogBlock(disk_manager, disk::BlockID(filename, 0));
    EXPECT_TRUE(read_result.IsOk()) << read_result.Error();
    EXPECT_EQ(block.Offset(), kOffset0001);
    EXPECT_EQ(block.RawBlock().Content(), kExpectBlockContentSize10);
}

FILE_NONEXISTENT_TEST(LogFileNonExistentTest);

TEST_F(LogFileNonExistentTest, ReadLogBlockFailWithoutFile) {
    using namespace dblog::internal;
    const size_t block_size = 10;
    LogBlock block(block_size);
    disk::DiskManager disk_manager(directory_path, block_size);

    auto read_result = block.ReadLogBlock(
        disk_manager, disk::BlockID(non_existent_filename, 0));
    EXPECT_TRUE(read_result.IsError());
}

TEST(Log, LogRecordWithHeaderSuccess) {
    dblog::LogTransactionBegin log_record(/*transaction_id=*/0);
    auto log_record_with_header = dblog::LogRecordWithHeader(log_record);
    EXPECT_EQ(log_record_with_header.size(),
              /*length of checksum*/ 4 + /*length of size of log*/ 4 +
                  log_record.LogBody().size() + /*length of size of log*/ 4);
}

FILE_EXISTENT_TEST(LogFileExistentLogManager, "abcdef");
FILE_NONEXISTENT_TEST(LogFileNonExistentLogManager);

TEST_F(LogFileExistentLogManager, InitSuccess) {
    dblog::LogManager log_manager(/*log_filename=*/filename,
                                  /*log_directory_name=*/directory_path,
                                  /*block_size=*/20);
    EXPECT_TRUE(log_manager.Init().IsOk());
}

TEST_F(LogFileNonExistentLogManager, InitFail) {
    dblog::LogManager log_manager(/*log_filename=*/non_existent_filename,
                                  /*log_directory_name=*/directory_path,
                                  /*block_size=*/20);
    EXPECT_TRUE(log_manager.Init().IsError());
}

TEST_F(LogFileExistentLogManager, WriteLogSuccess) {
    dblog::LogManager log_manager(/*log_filename=*/filename,
                                  /*log_directory_name=*/directory_path,
                                  /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());

    std::vector<uint8_t> bytes = {'a', 'b', 'c', 'd'};
    auto write_result          = log_manager.WriteLog(bytes);
    EXPECT_TRUE(write_result.IsOk()) << write_result.Error() << '\n';
}

TEST_F(LogFileExistentLogManager, FlushSuccess) {
    dblog::LogManager log_manager(/*log_filename=*/filename,
                                  /*log_directory_name=*/directory_path,
                                  /*block_size=*/20);
    ASSERT_TRUE(log_manager.Init().IsOk());

    std::vector<uint8_t> bytes = {'a', 'b', 'c', 'd'};
    auto write_result          = log_manager.WriteLog(bytes);
    ASSERT_TRUE(write_result.IsOk()) << write_result.Error() << '\n';
    dblog::LogSequenceNumber lsn = write_result.Get();
    auto flush_result            = log_manager.Flush(lsn);
    EXPECT_TRUE(flush_result.IsOk());
}

FILE_EXISTENT_TEST(LogFileEmptyLogManager, "");

TEST_F(LogFileEmptyLogManager, WriteAndReadLastLog) {
    dblog::LogManager log_manager(/*log_filename=*/filename,
                                  /*log_directory_name=*/directory_path,
                                  /*block_size=*/32);
    ASSERT_TRUE(log_manager.Init().IsOk());
    std::vector<uint8_t> log_record_bytes = {
        /*checksum*/ '\0',     '\0', '\0', '\0',
        /*log body length*/ 4, '\0', '\0', '\0',
        /*log body*/ 'a',      'b',  'c',  'd',
        /*log body length*/ 4, '\0', '\0', '\0'};
    std::vector<uint8_t> log_body_bytes = {
        'a',
        'b',
        'c',
        'd',
    };

    auto write_result = log_manager.WriteLog(log_record_bytes);
    ASSERT_TRUE(write_result.IsOk()) << write_result.Error() << '\n';
    ASSERT_TRUE(log_manager.Flush().IsOk());

    auto last_log_iterator_result = log_manager.LastLog();
    EXPECT_TRUE(last_log_iterator_result.IsOk())
        << last_log_iterator_result.Error() << '\n';

    auto last_log_iterator = last_log_iterator_result.Get();
    auto log_body_result   = last_log_iterator.LogBody();
    EXPECT_TRUE(log_body_result.IsOk()) << log_body_result.Error() << '\n';
    EXPECT_EQ(log_body_result.Get(), log_body_bytes);
}

TEST_F(LogFileEmptyLogManager, WriteAndReadTooLongLastLog) {
    dblog::LogManager log_manager(/*log_filename=*/filename,
                                  /*log_directory_name=*/directory_path,
                                  /*block_size=*/16);
    ASSERT_TRUE(log_manager.Init().IsOk());
    std::vector<uint8_t> log_record_bytes = {
        '\0', '\0', '\0', '\0', 26,  '\0', '\0', '\0', 'a', 'b',
        'c',  'd',  'e',  'f',  'g', 'h',  'i',  'j',  'k', 'l',
        'm',  'n',  'o',  'p',  'q', 'r',  's',  't',  'u', 'v',
        'w',  'x',  'y',  'z',  26,  '\0', '\0', '\0'};
    std::vector<uint8_t> log_body_bytes = {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    };

    auto write_result = log_manager.WriteLog(log_record_bytes);
    ASSERT_TRUE(write_result.IsOk()) << write_result.Error() << '\n';
    ASSERT_TRUE(log_manager.Flush().IsOk());

    auto last_log_iterator_result = log_manager.LastLog();
    EXPECT_TRUE(last_log_iterator_result.IsOk())
        << last_log_iterator_result.Error() << '\n';

    auto last_log_iterator = last_log_iterator_result.Get();
    auto log_body_result   = last_log_iterator.LogBody();
    EXPECT_TRUE(log_body_result.IsOk()) << log_body_result.Error() << '\n';
    EXPECT_EQ(log_body_result.Get(), log_body_bytes);
}

class LogFileInitialized : public ::testing::Test {
  protected:
    LogFileInitialized()
        : directory_path("parent_dir/"), filename("filename"), block_size(16) {
        if (!std::filesystem::exists(directory_path)) {
            std::filesystem::create_directories(directory_path);
        }
        std::ofstream file(directory_path + filename);
        file.close();

        dblog::LogManager log_manager(filename, directory_path, block_size);
        log_manager.Init();

        std::vector<uint8_t> log_record_bytes1 = {
            '\0', '\0', '\0', '\0', 26,  '\0', '\0', '\0', 'a', 'b',
            'c',  'd',  'e',  'f',  'g', 'h',  'i',  'j',  'k', 'l',
            'm',  'n',  'o',  'p',  'q', 'r',  's',  't',  'u', 'v',
            'w',  'x',  'y',  'z',  26,  '\0', '\0', '\0'};
        log_manager.WriteLog(log_record_bytes1);

        std::vector<uint8_t> log_record_bytes2 = {'\0', '\0', '\0', '\0', 1,
                                                  '\0', '\0', '\0', 'a',  1,
                                                  '\0', '\0', '\0'};
        log_manager.WriteLog(log_record_bytes2);

        std::vector<uint8_t> log_record_bytes3 = {
            '\0', '\0', '\0', '\0', 13,   '\0', '\0', '\0', 'a',
            'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',
            'k',  'l',  'm',  13,   '\0', '\0', '\0'};
        log_manager.WriteLog(log_record_bytes3);

        log_manager.Flush();
    }

    virtual ~LogFileInitialized() override {
        std::filesystem::remove_all(directory_path);
    }

    const std::string directory_path;
    const std::string filename;
    const int block_size;
};

TEST_F(LogFileInitialized, LogManagerInitWhenLogExists) {
    dblog::LogManager log_manager(/*log_filename=*/filename,
                                  /*log_directory_name=*/directory_path,
                                  /*block_size=*/block_size);

    auto result = log_manager.Init();
    EXPECT_TRUE(result.IsOk()) << result.Error() << '\n';

    auto last_log_result = log_manager.LastLog();
    EXPECT_TRUE(last_log_result.IsOk()) << last_log_result.Error() << '\n';
    auto last_log = last_log_result.Get();

    std::vector<uint8_t> expect_last_log = {'a', 'b', 'c', 'd', 'e', 'f', 'g',
                                            'h', 'i', 'j', 'k', 'l', 'm'};

    auto last_log_body = last_log.LogBody();
    EXPECT_TRUE(last_log_body.IsOk()) << last_log_body.Error() << '\n';
    EXPECT_EQ(last_log_body.Get(), expect_last_log);
}

TEST_F(LogFileInitialized, LogIteratorPrevious) {
    dblog::LogManager log_manager(/*log_filename=*/filename,
                                  /*log_directory_name=*/directory_path,
                                  /*block_size=*/block_size);

    auto result = log_manager.Init();
    ASSERT_TRUE(result.IsOk()) << result.Error() << '\n';

    auto last_log_result = log_manager.LastLog();
    ASSERT_TRUE(last_log_result.IsOk()) << last_log_result.Error() << '\n';
    auto log = last_log_result.Get();

    // log_record 2
    EXPECT_TRUE(log.HasPrevious());
    result = log.Previous();
    EXPECT_TRUE(result.IsOk()) << result.Error() << '\n';
    auto log_body = log.LogBody();
    EXPECT_TRUE(log_body.IsOk()) << log_body.Error() << '\n';
    std::vector<uint8_t> expect_log = {'a'};
    EXPECT_EQ(log_body.Get(), expect_log);

    // log_record 1
    EXPECT_TRUE(log.HasPrevious());
    result = log.Previous();
    EXPECT_TRUE(result.IsOk()) << result.Error() << '\n';
    log_body = log.LogBody();
    EXPECT_TRUE(log_body.IsOk()) << log_body.Error() << '\n';
    expect_log = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
                  'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
                  's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
    EXPECT_EQ(log_body.Get(), expect_log);

    // No log record
    EXPECT_FALSE(log.HasPrevious());
    result = log.Previous();
    EXPECT_TRUE(result.IsError());
}

TEST_F(LogFileInitialized, LogIteratorNext) {
    dblog::LogManager log_manager(/*log_filename=*/filename,
                                  /*log_directory_name=*/directory_path,
                                  /*block_size=*/block_size);

    auto result = log_manager.Init();
    ASSERT_TRUE(result.IsOk()) << result.Error() << '\n';

    auto last_log_result = log_manager.LastLog();
    ASSERT_TRUE(last_log_result.IsOk()) << last_log_result.Error() << '\n';
    auto log = last_log_result.Get();

    result = log.Previous();
    EXPECT_TRUE(result.IsOk()) << result.Error() << '\n';
    result = log.Previous();
    EXPECT_TRUE(result.IsOk()) << result.Error() << '\n';

    // log record 2
    ResultV<bool> has_next = log.HasNext();
    EXPECT_TRUE(has_next.IsOk());
    EXPECT_TRUE(has_next.Get());
    result = log.Next();
    EXPECT_TRUE(result.IsOk()) << result.Error() << '\n';
    auto log_body = log.LogBody();
    EXPECT_TRUE(log_body.IsOk()) << log_body.Error() << '\n';
    std::vector<uint8_t> expect_log = {'a'};
    EXPECT_EQ(log_body.Get(), expect_log);

    // log record 3
    has_next = log.HasNext();
    EXPECT_TRUE(has_next.IsOk());
    EXPECT_TRUE(has_next.Get());
    result = log.Next();
    EXPECT_TRUE(result.IsOk()) << result.Error() << '\n';
    log_body = log.LogBody();
    EXPECT_TRUE(log_body.IsOk()) << log_body.Error() << '\n';
    expect_log = {'a', 'b', 'c', 'd', 'e', 'f', 'g',
                  'h', 'i', 'j', 'k', 'l', 'm'};
    EXPECT_EQ(log_body.Get(), expect_log);

    // No next log
    has_next = log.HasNext();
    EXPECT_TRUE(has_next.IsOk());
    EXPECT_FALSE(has_next.Get());
}
