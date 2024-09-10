#include "log.h"
#include "data/uint32.h"

namespace dblog {

namespace internal {

constexpr int kDefaultOffset = 4;

LogBlock::LogBlock(const int block_size) : block_(block_size) {
    UpdateOffset(kDefaultOffset);
}

ResultE<size_t> LogBlock::Append(const std::vector<uint8_t> &bytes,
                                 size_t bytes_offset) {
    ResultE<size_t> append_result =
        block_.WriteBytesWithOffset(offset_, bytes, bytes_offset);
    if (append_result.IsOk()) {
        UpdateOffset(offset_ + bytes.size() - bytes_offset);
        return Ok();
    } else {
        UpdateOffset(block_.BlockSize());
        return append_result;
    }
}

constexpr int kOffsetPositionInLogBlock = 0;

Result LogBlock::ReadLogBlock(const disk::DiskManager &disk_manager,
                              const disk::BlockID block_id) {
    Result read_result = disk_manager.Read(block_id, block_);
    if (read_result.IsError()) return read_result;

    // NOTE: We don't care the error case as block_ is assured to be large
    // enough to store the offset, more specifically size of the block_ is
    // larger than 4.
    offset_ = block_.ReadInt(kOffsetPositionInLogBlock).Get();
    return Ok();
}

void LogBlock::UpdateOffset(int new_offset) {
    offset_ = new_offset;

    // NOTE: We don't care the error case as block_ is assured to be large
    // enough to store the offset, more specifically size of the block_ is
    // larger than 4.
    block_.WriteInt(/*offset=*/kOffsetPositionInLogBlock, /*value=*/offset_);
}

} // namespace internal

// TODO: compute check sum
uint32_t ComputeChecksum(const std::vector<uint8_t> &bytes) { return 0; }

// Length of the log header in bytes. The header consists of "checksum (4bytes)
// + length of the log body (4bytes)".
constexpr size_t kLogHeaderLength = 8;

std::vector<uint8_t> LogRecordWithHeader(const LogRecord *log_record) {
    std::vector<uint8_t> log_body_with_header(kLogHeaderLength);
    const std::vector<uint8_t> &log_body = log_record->LogBody();
    data::WriteUint32NoFail(log_body_with_header, 0, ComputeChecksum(log_body));
    data::WriteUint32NoFail(log_body_with_header, 4, log_body.size());
    log_record->AppendLogBody(log_body_with_header);
    return log_body_with_header;
}

LogManager::LogManager(const std::string &log_filename,
                       const std::string &log_directory_path,
                       const size_t block_size)
    : log_filename_(log_filename),
      disk_manager_(disk::DiskManager(
          /*directory_path=*/log_directory_path, /*block_size=*/block_size)) {}

Result LogManager::Init() {
    if (disk_manager_.BlockSize() < internal::kDefaultOffset) {
        return Error("log blocksize must be larger than 4.");
    }

    const auto expect_logfile_size = disk_manager_.Size(log_filename_);
    if (expect_logfile_size.IsError())
        return Error("the log file does not exist.");
    const size_t current_logfile_size = expect_logfile_size.Get();

    if (current_logfile_size == 0) {
        current_block_id_ = disk::BlockID(log_filename_, 0);
        if (disk_manager_.AllocateNewBlocks(current_block_id_).IsError()) {
            return Error("failed to allocate new blocks in the log file");
        }
        current_block_ = internal::LogBlock(disk_manager_.BlockSize());
        return Ok();
    }

    current_block_id_ = disk::BlockID(log_filename_, current_logfile_size - 1);
    if (current_block_.ReadLogBlock(disk_manager_, current_block_id_)
            .IsError()) {
        return Error("the last block cannot be read.");
    }
    return Ok();
}

ResultV<LogSequenceNumber> LogManager::WriteLog(const LogRecord *log_record) {
    std::vector<uint8_t> log_body_with_header = LogRecordWithHeader(log_record);

    const disk::BlockID rollback_block_id   = current_block_id_;
    const internal::LogBlock rollback_block = current_block_;

    ResultE<size_t> append_result =
        current_block_.Append(log_body_with_header, /*bytes_offset=*/0);
    while (append_result.IsError()) {
        size_t next_offset = append_result.Error();
        if (MoveToNextBlock().IsError()) {
            current_block_id_ = rollback_block_id;
            current_block_    = rollback_block;
            return Error(
                "failed to save the current block or allocate a next block");
        }
        append_result =
            current_block_.Append(log_body_with_header, next_offset);
    }

    return Ok(current_number_++);
}

Result LogManager::Flush(LogSequenceNumber number_to_flush) {
    if (number_to_flush < next_save_number_) return Ok();
    return Flush();
}

Result LogManager::Flush() {
    if (WriteCurrentBlock().IsError()) {
        return Error("failed to write the current block");
    }
    next_save_number_ = current_number_;
    return disk_manager_.Flush(log_filename_);
}

Result LogManager::MoveToNextBlock() {
    if (WriteCurrentBlock().IsError()) {
        return Error("failed to write current block");
    }
    return AllocateNextBlock();
}

Result LogManager::AllocateNextBlock() {
    current_block_id_ =
        disk::BlockID(log_filename_, current_block_id_.BlockIndex() + 1);
    Result allocate_result = disk_manager_.AllocateNewBlocks(current_block_id_);
    if (allocate_result.IsError()) {
        current_block_id_ =
            disk::BlockID(log_filename_, current_block_id_.BlockIndex() - 1);
        return Error("failed to allocate new block");
    }
    current_block_ = internal::LogBlock(disk_manager_.BlockSize());
    return Ok();
}

} // namespace dblog