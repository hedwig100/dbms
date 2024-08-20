#ifndef LOG_H
#define LOG_H

#include <cstdint>
#include <result.h>
#include <vector>

#include "disk.h"

namespace dblog {

class LogBlock {
  public:
    inline LogBlock() : offset_(0), block_() {};

    explicit LogBlock(const int block_size);

    inline int Offset() const { return offset_; }
    inline void SetOffset(int offset) { offset_ = offset; }

    // Appends `bytes` to the tail of this block. If the input bytes sequece
    // does not fit to the block, returns an error. The error contains the
    // length of bytes sequence appended to the block.
    ResultE<size_t>
    WriteAppend(const std::vector<uint8_t>::iterator &bytes_begin,
                const std::vector<uint8_t>::iterator &bytes_end);

  private:
    // The size of the content of the block in bytes (`block_size` is the
    // maximum size of the block).
    int offset_;

    disk::Block block_;
};

// Types of log records
enum class LogType {
    kTransactionBegin = 0,
    kOperation        = 1,
    kTransactionEnd   = 2,
    kCheckpointing    = 3,
};

// The base class for all types of log records
class LogRecord {
  public:
    // The log type
    virtual LogType Type() const = 0;

    // The log body
    virtual std::vector<uint8_t> LogBody() const = 0;
};

// Manages logs. More specifically, writes a log to the log file
// and reads logs from the log file.
class LogManager {
  public:
    // Initiate a log manager, the file of `log_directory_name`/`log_filename`
    // should exist when this log manager is initiated. `block_size` is the size
    // of the block of log file. The Init function should be called right after
    // the constrcutor is called.
    LogManager(const std::string &log_filename,
               const std::string &log_directory_name, const int block_size);

    // This function should be called before starting using the instance.
    Result Init();

    // Writes `log_records` to the log file.
    Result WriteLogs(const std::vector<LogRecord *> &log_records);

    // Flushs all log writes to the disk.
    Result FlushAll();

  private:
    Result AllocateNewBlocks(const size_t bytesize_to_allocate) const;

    struct LogBlockInfo {
        disk::BlockID block_id;
        disk::Block block;
        int offset;
    };

    const std::string log_filename_;
    disk::DiskManager disk_manager_;
    LogBlockInfo last_block_info_;
};

} // namespace dblog

#endif // LOG_H