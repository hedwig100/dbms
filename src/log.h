#ifndef _LOG_H
#define _LOG_H

#include "disk.h"
#include "log_record.h"
#include "result.h"
#include <string>

namespace dblog {

using namespace ::result;

using LogSequenceNumber = uint32_t;

namespace internal {

// LogBLock is a kind of block. The main feature is that it has the offset at
// the first 4-bytes of the block. To append new log records, the offset is
// necessary.
class LogBlock {
  public:
    inline LogBlock() {};

    // Initiate this log block with `block_size` including the offset region.
    // `block_size` must be larger than 4 as the offset cannot in the block.
    // The offset is initiated with 0.
    explicit LogBlock(const int block_size);

    // Returns the offset of the log block (the part from the offset is empty).
    inline int Offset() const { return offset_; }

    // Appends the `bytes`[`bytes_offset`:] to the block.
    // `bytes`[`bytes_offset`:] is the part of `bytes` after `bytes_offset`. If
    // the `bytes[`bytes_offset`:]` does not fit to this block, it returns
    // length of the `bytes` fit in this block.
    ResultE<size_t> Append(const std::vector<uint8_t> &bytes,
                           size_t bytes_offset);

    // Read a log block from disk.
    Result ReadLogBlock(const disk::DiskManager &disk_manager,
                        const disk::BlockID block_id);

    // Returns a raw content of the block including offset. This method is
    // mainly for writing a block.
    inline const disk::Block &RawBlock() const { return block_; }

  private:
    // Updates the `offset_` and writes the offset to the block.
    void UpdateOffset(int new_offset);

    disk::Block block_;
    int offset_;
};

} // namespace internal

// Compute the checksum of the `bytes`.
uint32_t ComputeChecksum(const std::vector<uint8_t> &bytes);

// Compute the log record with the header. The header has
// length of log record in bytes and hash of the log record (to achieve
// atomic write of logs). NOTE: log length must be smaller than 2^32-1.
std::vector<uint8_t> LogRecordWithHeader(const LogRecord *log_record);

// LogManager manages a log file. This class has a current (most recent) log
// block of the log file, and append log records to the log file.
class LogManager {
  public:
    // Initiate a log manager, the file of `log_directory_path`/`log_filename`
    // should exist when this log manager is initiated. `block_size` is the size
    // of the block of log file, which must be larger than 4. The Init function
    // should be called right after the constrcutor is called.
    LogManager(const std::string &log_filename,
               const std::string &log_directory_path, const size_t block_size);

    // This function should be called before starting using the instance.
    Result Init();

    // Writes `log_record` to the log file with the header. The header has
    // length of log record in bytes and hash of the log record (to achieve
    // atomic write of logs). NOTE: log length must be smaller than 2^32-1.
    ResultV<LogSequenceNumber> WriteLog(const LogRecord *log_record);

    // Flushes log records until logs with log sequence number of
    // `number_to_flush` (including the end).
    Result Flush(LogSequenceNumber number_to_flush);

    // Flushes all log records.
    Result Flush();

  private:
    // Writes the current block to disk and allocates a new block and sets the
    // block to the `current_block`.
    Result MoveToNextBlock();

    // Writes the current block to disk.
    inline Result WriteCurrentBlock() const {
        return disk_manager_.Write(current_block_id_,
                                   current_block_.RawBlock());
    }

    // Allocates the next block of the current block.
    Result AllocateNextBlock();

    const std::string log_filename_;
    disk::DiskManager disk_manager_;
    LogSequenceNumber current_number_   = 0;
    LogSequenceNumber next_save_number_ = 0;
    disk::BlockID current_block_id_;
    internal::LogBlock current_block_;
};

} // namespace dblog

#endif // _LOG_H