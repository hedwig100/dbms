#ifndef _TRANSACTION_LOG_H
#define _TRANSACTION_LOG_H

#include "checksum.h"
#include "disk.h"
#include "result.h"
#include <memory>
#include <shared_mutex>
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
    Result ReadLogBlock(disk::DiskManager &disk_manager,
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

// The error that implies the complete log record is not written to the disk.
const ResultV<std::vector<uint8_t>> kCompleteLogNotWrittenToDisk =
    Error("dblog::LogIterator::LogBody() the complete log body is not "
          "written to the disk.");

// Has one log record as a byte sequence. This class can advance itself in the
// log file and go to the previous log record if exists.
//
// Log Format (detailed format is in the documents):
// | log header (8bytes) | log body | log length (4bytes) |
//
// `log_body_length` is length of log body, not length of the whole log record.
// `log_start` is the position that log header starts.
class LogIterator {
  public:
    LogIterator(disk::DiskManager &disk_manager,
                const disk::DiskPosition &log_start, int log_body_length);
    LogIterator(disk::DiskManager &disk_manager,
                const disk::DiskPosition &log_start, int log_body_length,
                const internal::LogBlock &block);

    LogIterator(const LogIterator &other);

    LogIterator &operator=(const LogIterator &other);

    // Returns the log body without headers like checksum.
    ResultV<std::vector<uint8_t>> LogBody();

    // Returns true if the log iterator has the next log iterator.
    ResultV<bool> HasNext();

    // Move to the next log record if exists. If the next log record does not
    // exist, it returns the failure.
    Result Next();

    // Returns true if the log iterator has the previous log iterator.
    bool HasPrevious();

    // Move to the previous log record if exists. If the previous log record
    // does not exist, it returns the failure.
    Result Previous();

  private:
    // Returns the block at which `log_start_` is located.
    ResultV<internal::LogBlock> LogStartBlock();

    disk::DiskManager &disk_manager_;
    disk::DiskPosition log_start_;
    int log_body_length_;
    std::unique_ptr<internal::LogBlock> log_start_block_;
};

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

    inline disk::DiskManager &DiskManager() { return disk_manager_; }

    // Writes bytes to log file.
    ResultV<LogSequenceNumber>
    WriteLog(const std::vector<uint8_t> &log_record_bytes);

    // Returns the most recent log iterator.
    ResultV<LogIterator> LastLog();

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
    inline Result WriteCurrentBlock() {
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
    std::shared_mutex mutex_;
};

} // namespace dblog

#endif // _LOG_H