#include "log.h"

#include <filesystem>
#include <fstream>

namespace dblog {

// Computes hash value of `log_length` + `log_body`.
std::vector<uint8_t> ComputeHashValue(const size_t log_length,
                                      const std::vector<uint8_t> &log_body) {
    std::vector<uint8_t> hash_value;
    // TODO: Computes the decent hash value.
    hash_value.assign(32, '\0');
    return hash_value;
}

// Computes the header of a `log_record` and its full log record bytes with the
// header.
std::vector<uint8_t> LogRecordWithHeader(const LogRecord *log_record) {
    const std::vector<uint8_t> log_body = log_record->LogBody();
    const size_t log_length             = log_body.size();
    std::vector<uint8_t> log_record_with_header =
        ComputeHashValue(log_length, log_body);

    log_record_with_header.reserve(log_record_with_header.size() +
                                   /*size of `log_length` in bytes*/ 8 +
                                   log_body.size());
    for (int shift_size = 0; shift_size < 64; shift_size += 8)
        log_record_with_header.push_back((log_length >> shift_size) & 0xFFFF);
    std::copy(log_body.begin(), log_body.end(),
              std::back_inserter(log_record_with_header));
    return log_record_with_header;
}

// Concatenates log bytes sequence with their headers.
std::vector<uint8_t>
ConcatenateLogRecords(const std::vector<LogRecord *> &log_records) {
    std::vector<uint8_t> concatenated_logs;
    for (const LogRecord *log_record : log_records) {
        const std::vector<uint8_t> log_record_with_header =
            LogRecordWithHeader(log_record);
        concatenated_logs.reserve(concatenated_logs.size() +
                                  log_record_with_header.size());
        std::copy(log_record_with_header.begin(), log_record_with_header.end(),
                  std::back_inserter(concatenated_logs));
    }
    return concatenated_logs;
}

// The head of each block is offset of the block (the block is empty from the
// offset). The offset is represented as a 4-byte integer.
const int kOffsetSize = 4;

explicit LogBlock::LogBlock(const int block_size) {
    offset_ = kOffsetSize;
    block_  = disk::Block(block_size);
    block_.WriteInt(0, offset_);
}

ResultE<size_t>
LogBlock::WriteAppend(const std::vector<uint8_t>::iterator &bytes_begin,
                      const std::vector<uint8_t>::iterator &bytes_end) {
    const size_t bytes_size      = bytes_end - bytes_begin;
    const size_t appendable_size = block_.content_.size() - size_;
    if (bytes_size > appendable_size) {
        std::copy(bytes_begin, bytes_begin + appendable_size,
                  content_.begin() + size_);
        size_ += appendable_size;
        return Error(appendable_size);
    }
    std::copy(bytes_begin, bytes_end, content_.begin() + size_);
    size_ += bytes_size;
    return Ok();
}

LogManager::LogManager(const std::string &log_filename,
                       const std::string &log_directory_name,
                       const int block_size)
    : log_filename_(log_filename),
      disk_manager_(disk::DiskManager(
          /*directory_name=*/log_directory_name, /*block_size=*/block_size)) {}

Result LogManager::Init() {
    const auto expect_logfile_size = disk_manager_.Size(log_filename_);
    if (expect_logfile_size.IsError())
        return Error("the log file does not exist.");
    const size_t current_logfile_size = expect_logfile_size.Get();

    if (current_logfile_size == 0) {
        if (disk_manager_.AllocateNewBlocks(disk::BlockID(log_filename_, 1))
                .IsError()) {
            return Error("failed to allocate new blocks in the log file");
        }
        last_block_info_ = LogManager::LogBlockInfo{
            .block_id = disk::BlockID(log_filename_, 0),
            .block    = disk::Block(/**/),
            .offset   = kOffsetSize};
        return Ok();
    }

    disk::BlockID last_block_id(log_filename_, current_logfile_size - 1);
    disk::Block last_block;
    if (disk_manager_.Read(last_block_id, last_block).IsError()) {
        return Error("the last block cannot be read.");
    }

    const auto expect_offset = last_block.ReadInt(0);
    if (expect_offset.IsError()) return Error("offset not in the block");
    last_block.SetSize(expect_offset.Get());

    last_block_info_ = LogManager::LogBlockInfo{
        .block_id = last_block_id,
        .block    = last_block,
        .offset   = expect_offset.Get(),
    };

    return Ok();
}

Result LogManager::WriteLogs(const std::vector<LogRecord *> &log_records) {
    const std::vector<uint8_t> concatenated_logs =
        ConcatenateLogRecords(log_records);
    if (AllocateNewBlocks(concatenated_logs.size()).IsError()) {
        return Error("failed to allocate new log blocks");
    }

    auto log_iterator = concatenated_logs.begin();
    while (log_iterator < concatenated_logs.end()) {
        const size_t space_left =
            disk_manager_.BlockSize() - last_block_info_.block.Size();
    }

    return Ok();
}

Result LogManager::FlushAll() { return disk_manager_.Flush(log_filename_); }

Result LogManager::AllocateNewBlocks(const size_t bytesize_to_allocate) const {
    const size_t current_blocksize = last_block_info_.block_id.BlockIndex() + 1;
    const size_t blocksize_to_allocate =
        (bytesize_to_allocate + disk_manager_.BlockSize() - 1) /
        disk_manager_.BlockSize();
    return disk_manager_.AllocateNewBlocks(disk::BlockID(
        log_filename_, current_blocksize + blocksize_to_allocate));
}

} // namespace dblog