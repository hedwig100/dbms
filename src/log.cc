#include "log.h"
#include "data/int.h"
#include "data/uint32.h"
#include <iostream>

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

Result LogBlock::ReadLogBlock(disk::DiskManager &disk_manager,
                              const disk::BlockID block_id) {
    Result read_result = disk_manager.Read(block_id, block_);
    if (read_result.IsError())
        return read_result + Error("dblog::internal::LogBlock::ReadLogBlock() "
                                   "faield to read the log block.");

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

// Read bytes which can lie across multiple log blocks. `position` specifies the
// start position to read the bytes. The `position` needs to be located outside
// the offset region of the block (outside the first `kDefaultOffset` bytes of
// the block). `block` is the block in which `position` is located. `length` is
// the length of the bytes to read. The bytes are written to `bytes`.
Result ReadBytesAcrossBlocks(disk::DiskManager &disk_manager,
                             const LogBlock &block,
                             const disk::DiskPosition &position, int length,
                             std::vector<uint8_t> &bytes) {
    if (position.Offset() < kDefaultOffset)
        return Error("dblog::internal::ReadBytesAcrossBlocks() position must "
                     "not be first kDefaultOffset bytes of the block.");

    int length_of_first_block =
        (position.Offset() + length <= disk_manager.BlockSize()
             ? length
             : disk_manager.BlockSize() - position.Offset());

    Result read_result = block.RawBlock().ReadBytes(
        position.Offset(), length_of_first_block, bytes);
    if (read_result.IsError())
        return read_result + Error("dblog::internal::ReadBytesAcrossBlocks() "
                                   "failed to read the first block.");
    length -= length_of_first_block;

    if (length == 0) return Ok();

    const int log_block_size       = disk_manager.BlockSize() - kDefaultOffset;
    disk::BlockID current_block_id = position.BlockID();
    LogBlock current_block         = block;

    while (length > 0) {
        current_block_id += 1;
        Result read_result =
            current_block.ReadLogBlock(disk_manager, current_block_id);
        if (read_result.IsError())
            return read_result + Error("dblog::internal::ReadBytesAcrossBlocks("
                                       ") failed to read the blocks");

        if (length >= log_block_size) {
            std::copy(current_block.RawBlock().Content().begin() +
                          kDefaultOffset,
                      current_block.RawBlock().Content().end(),
                      std::back_inserter(bytes));
            length -= log_block_size;
        } else {
            std::vector<uint8_t> tmp_bytes(length);
            current_block.RawBlock().ReadBytes(kDefaultOffset, length,
                                               tmp_bytes);
            std::copy(tmp_bytes.begin(), tmp_bytes.end(),
                      std::back_inserter(bytes));
            length = 0;
        }
    }

    return Ok();
}

// Read uint32_t which can lie across multiple log blocks. `position` specifies
// the start position to read the bytes. The `position` needs to be located
// outside the offset region of the block (outside the first `kDefaultOffset`
// bytes of the block). `block` is the block in which `position` is located.
ResultV<uint32_t> ReadUint32AcrossBlocks(disk::DiskManager &disk_manager,
                                         const LogBlock &block,
                                         const disk::DiskPosition &position) {
    std::vector<uint8_t> uint32_bytes(data::kUint32Bytesize);
    auto read_result = ReadBytesAcrossBlocks(
        disk_manager, block, position, data::kUint32Bytesize, uint32_bytes);
    if (read_result.IsError()) {
        return read_result +
               Error("dblog::internal::ReadUint32AcrossBlocks() failed to read "
                     "bytes corresponding to uint32_t.");
    }
    return data::ReadUint32(uint32_bytes, 0);
}

// Moves `position` for the amount of `displacement` in the log block, which
// means we ignore the offset regsion (first `kDefaultOffset` bytes) of the log
// block. `displacement` can be negative.
disk::DiskPosition MoveInLogBlock(const disk::DiskPosition &position,
                                  const int displacement,
                                  const int block_size) {
    disk::DiskPosition log_position =
        position.Move(-kDefaultOffset, block_size);
    log_position = log_position.Move(displacement, block_size - kDefaultOffset);
    return log_position.Move(kDefaultOffset, block_size);
}

// Read bytes which can lie across multiple log blocks.
// `position.`.Move(`offset`) specifies the start position to read the bytes.
// The `position` needs to be located outside the offset region of the block.
// `block` is the block in which `position` is located. `length` is the length
// of the bytes to read. The bytes are written to `bytes`.
Result ReadBytesAcrossBlocksWithOffset(disk::DiskManager &disk_manager,
                                       const LogBlock &block,
                                       const disk::DiskPosition &position,
                                       const int offset, int length,
                                       std::vector<uint8_t> &bytes) {
    disk::DiskPosition start_position =
        MoveInLogBlock(position, offset, disk_manager.BlockSize());
    if (start_position.BlockID() == position.BlockID()) {
        return ReadBytesAcrossBlocks(disk_manager, block, start_position,
                                     length, bytes);
    }

    LogBlock start_block;
    auto read_result =
        start_block.ReadLogBlock(disk_manager, start_position.BlockID());
    if (read_result.IsError())
        return read_result +
               Error("dblog::internal::ReadBytesAcrossBlocksWithOffset() "
                     "failed to read the start block.");
    return ReadBytesAcrossBlocks(disk_manager, start_block, start_position,
                                 length, bytes);
}

// Read int which can lie across multiple log blocks.
// `position.`.Move(`offset`) specifies the start position to read the bytes.
// The `position` needs to be located outside the offset region of the block.
// `block` is the block in which `position` is located.
ResultV<int> ReadIntAcrossBlocksWithOffset(disk::DiskManager &disk_manager,
                                           const LogBlock &block,
                                           const disk::DiskPosition &position,
                                           const int offset) {
    std::vector<uint8_t> int_bytes(data::kIntBytesize);
    auto read_result = ReadBytesAcrossBlocksWithOffset(
        disk_manager, block, position, offset, data::kIntBytesize, int_bytes);
    if (read_result.IsError()) {
        return read_result +
               Error("dblog::internal::ReadIntAcrossBlocksWithOffset() failed "
                     "to read bytes corresponding to int.");
    }
    return data::ReadInt(int_bytes, 0);
}

} // namespace internal

constexpr int kChecksumBytesize = data::kUint32Bytesize;

constexpr int kLogLengthBytesize = data::kIntBytesize;

// Length of the log header in bytes. The header consists of "checksum (4bytes)
// + length of the log body (4bytes)".
constexpr size_t kLogHeaderLength = kChecksumBytesize + kLogLengthBytesize;

constexpr size_t kApproximateLogRecordLength =
    kLogHeaderLength + 40 + kLogLengthBytesize;

// Read the previous log of the log which starts from `log_start`. The `block`
// is the block that `log_start` is located.
ResultV<LogIterator> ReadPreviousLog(disk::DiskManager &disk_manager,
                                     const disk::DiskPosition &log_start,
                                     const internal::LogBlock &block) {

    ResultV<int> log_body_length_result =
        internal::ReadIntAcrossBlocksWithOffset(disk_manager, block, log_start,
                                                -kLogLengthBytesize);
    if (log_body_length_result.IsError()) {
        return log_body_length_result +
               Error("dblog::ReadPreviousLog() failed to read log body length "
                     "in tail.");
    }

    disk::DiskPosition previous_log_start = internal::MoveInLogBlock(
        log_start,
        -(kLogHeaderLength + log_body_length_result.Get() + kLogLengthBytesize),
        disk_manager.BlockSize());

    if (previous_log_start.BlockID() == log_start.BlockID()) {
        return Ok(LogIterator(disk_manager, previous_log_start,
                              log_body_length_result.Get(), block));
    }
    return Ok(LogIterator(disk_manager, previous_log_start,
                          log_body_length_result.Get()));
}

LogIterator::LogIterator(disk::DiskManager &disk_manager,
                         const disk::DiskPosition &log_start,
                         int log_body_length)
    : disk_manager_(disk_manager), log_start_(log_start),
      log_body_length_(log_body_length) {}

LogIterator::LogIterator(disk::DiskManager &disk_manager,
                         const disk::DiskPosition &log_start,
                         int log_body_length, const internal::LogBlock &block)
    : disk_manager_(disk_manager), log_start_(log_start),
      log_body_length_(log_body_length),
      log_start_block_(new internal::LogBlock(block)) {}

LogIterator::LogIterator(const LogIterator &other)
    : disk_manager_(other.disk_manager_), log_start_(other.log_start_),
      log_body_length_(other.log_body_length_) {
    if (other.log_start_block_) {
        log_start_block_ = std::unique_ptr<internal::LogBlock>(
            new internal::LogBlock(*other.log_start_block_));
    }
}

LogIterator &LogIterator::operator=(const LogIterator &other) {
    log_start_       = other.log_start_;
    log_body_length_ = other.log_body_length_;
    if (other.log_start_block_) {
        log_start_block_ = std::unique_ptr<internal::LogBlock>(
            new internal::LogBlock(*other.log_start_block_));
    }
    return *this;
}

ResultV<std::vector<uint8_t>> LogIterator::LogBody() {
    auto block_result = LogStartBlock();
    if (block_result.IsError()) {
        return block_result + Error("dblog::LogIterator::LogBody() failed to "
                                    "read log start block.");
    }

    auto checksum_result = internal::ReadUint32AcrossBlocks(
        disk_manager_, block_result.Get(), log_start_);
    if (checksum_result.IsError())
        return checksum_result +
               Error("dblog::LogIterator::LogBody() failed to read checksum.");

    std::vector<uint8_t> log_body;
    auto read_result = ReadBytesAcrossBlocksWithOffset(
        disk_manager_, block_result.Get(), log_start_, kLogHeaderLength,
        log_body_length_, log_body);
    if (read_result.IsError()) {
        return read_result +
               Error("dblog::LogIterator::LogBody() failed to read log body.");
    }

    if (ComputeChecksum(log_body) != checksum_result.Get())
        return kCompleteLogNotWrittenToDisk;

    return Ok(log_body);
}

ResultV<bool> LogIterator::HasNext() {
    const int log_record_length =
        kLogHeaderLength + log_body_length_ + kLogLengthBytesize;
    disk::DiskPosition next_log_start = internal::MoveInLogBlock(
        log_start_, log_record_length, disk_manager_.BlockSize());

    if (next_log_start.BlockID() == log_start_.BlockID()) {
        auto block_result = LogStartBlock();
        if (block_result.IsError()) {
            return block_result +
                   Error("dblog::LogIterator::HasNext() failed to "
                         "read log start block.");
        }
        return Ok(next_log_start.Offset() < block_result.Get().Offset());
    }

    internal::LogBlock next_log_block;
    Result read_result =
        next_log_block.ReadLogBlock(disk_manager_, next_log_start.BlockID());
    if (read_result.IsError()) {
        return read_result + Error("dblog::LogIterator::HasNext() failed to "
                                   "read next log start block.");
    }
    return Ok(next_log_start.Offset() < next_log_block.Offset());
}

Result LogIterator::Next() {
    auto block_result = LogStartBlock();
    if (block_result.IsError()) {
        return block_result + Error("dblog::LogIterator::Next() failed to "
                                    "read log start block.");
    }

    const int log_record_length =
        kLogHeaderLength + log_body_length_ + kLogLengthBytesize;
    ResultV<int> next_log_body_length_result = ReadIntAcrossBlocksWithOffset(
        disk_manager_, block_result.Get(), log_start_,
        log_record_length + kChecksumBytesize);
    if (next_log_body_length_result.IsError()) {
        return next_log_body_length_result +
               Error("dblog::LogIterator::Next() failed to read the next log "
                     "length.");
    }

    disk::DiskPosition next_log_start = internal::MoveInLogBlock(
        log_start_, log_record_length, disk_manager_.BlockSize());

    if (next_log_start.BlockID() != log_start_.BlockID()) {
        this->log_start_block_.reset();
    }
    this->log_start_       = next_log_start;
    this->log_body_length_ = next_log_body_length_result.Get();

    return Ok();
}

bool LogIterator::HasPrevious() {
    if (log_start_.BlockID().BlockIndex() == 0 &&
        log_start_.Offset() == internal::kDefaultOffset)
        return false;
    return true;
}

Result LogIterator::Previous() {
    auto block_result = LogStartBlock();
    if (block_result.IsError()) {
        return block_result + Error("dblog::LogIterator::Previsou() failed to "
                                    "read log start block.");
    }

    auto previous_log_result =
        ReadPreviousLog(disk_manager_, log_start_, block_result.Get());
    if (previous_log_result.IsError())
        return previous_log_result + Error("dblog::LogIterator::Previous() "
                                           "failed to read previous log.");

    if (previous_log_result.Get().log_start_.BlockID() !=
        log_start_.BlockID()) {
        this->log_start_block_.reset();
    }
    this->log_start_       = previous_log_result.Get().log_start_;
    this->log_body_length_ = previous_log_result.Get().log_body_length_;
    return Ok();
}

ResultV<internal::LogBlock> LogIterator::LogStartBlock() {
    if (log_start_block_) { return Ok(*log_start_block_.get()); }

    log_start_block_ = std::make_unique<internal::LogBlock>();
    auto read_result =
        log_start_block_->ReadLogBlock(disk_manager_, log_start_.BlockID());
    if (read_result.IsError()) {
        return read_result + Error("dblog::LogIterator::LogStartBlock() failed "
                                   "to read log start block.");
    }
    return Ok(*log_start_block_.get());
}

LogManager::LogManager(const std::string &log_filename,
                       const std::string &log_directory_path,
                       const size_t block_size)
    : log_filename_(log_filename),
      disk_manager_(disk::DiskManager(
          /*directory_path=*/log_directory_path, /*block_size=*/block_size)) {}

Result LogManager::Init() {
    std::lock_guard<std::shared_mutex> lock(mutex_);
    if (disk_manager_.BlockSize() < internal::kDefaultOffset) {
        return Error(
            "dblog::LogManager::Init() log blocksize must be larger than 4.");
    }

    const auto expect_logfile_size = disk_manager_.Size(log_filename_);
    if (expect_logfile_size.IsError())
        return expect_logfile_size +
               Error("dblog::LogManager::Init() the log file does not exist.");
    const size_t current_logfile_size = expect_logfile_size.Get();

    if (current_logfile_size == 0) {
        current_block_id_ = disk::BlockID(log_filename_, 0);
        if (disk_manager_.AllocateNewBlocks(current_block_id_).IsError()) {
            return Error("dblog::LogManager::Init() failed to allocate new "
                         "blocks in the log file.");
        }
        current_block_ = internal::LogBlock(disk_manager_.BlockSize());
        return Ok();
    }

    current_block_id_ = disk::BlockID(log_filename_, current_logfile_size - 1);
    if (current_block_.ReadLogBlock(disk_manager_, current_block_id_)
            .IsError()) {
        return Error(
            "dblog::LogManager::Init() the last block cannot be read.");
    }
    return Ok();
}

ResultV<LogIterator> LogManager::LastLog() {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return ReadPreviousLog(
        disk_manager_,
        disk::DiskPosition(current_block_id_, current_block_.Offset()),
        current_block_);
}

ResultV<LogSequenceNumber>
LogManager::WriteLog(const std::vector<uint8_t> &log_record_bytes) {
    std::lock_guard<std::shared_mutex> lock(mutex_);

    const disk::BlockID rollback_block_id   = current_block_id_;
    const internal::LogBlock rollback_block = current_block_;

    ResultE<size_t> append_result =
        current_block_.Append(log_record_bytes, /*bytes_offset=*/0);
    while (append_result.IsError()) {
        size_t next_offset = append_result.Error();
        Result move_result = MoveToNextBlock();
        if (move_result.IsError()) {
            current_block_id_ = rollback_block_id;
            current_block_    = rollback_block;
            return move_result +
                   Error("dblog::LogManager::WriteLog() failed to save the "
                         "current block or allocate a next block.");
        }
        append_result = current_block_.Append(log_record_bytes, next_offset);
    }

    return Ok(current_number_++);
}

Result LogManager::Flush(LogSequenceNumber number_to_flush) {
    if (number_to_flush < next_save_number_) return Ok();
    return Flush();
}

Result LogManager::Flush() {
    std::lock_guard<std::shared_mutex> lock(mutex_);

    Result write_result = WriteCurrentBlock();
    if (write_result.IsError()) {
        return write_result + Error("dblog::LogManager::Flush() failed to "
                                    "write the current block.");
    }
    next_save_number_ = current_number_;
    return disk_manager_.Flush(log_filename_);
}

Result LogManager::MoveToNextBlock() {
    Result write_result = WriteCurrentBlock();
    if (write_result.IsError()) {
        return write_result + Error("dblog::LogManager::MoveToNextBlock() "
                                    "failed to write current block.");
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
        return allocate_result + Error("dblog::LogManager::AllocateNextBlock() "
                                       "failed to allocate new block.");
    }
    current_block_ = internal::LogBlock(disk_manager_.BlockSize());
    return Ok();
}

} // namespace dblog