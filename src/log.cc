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
// the offset region of the block. `block` is the block in which `position` is
// located. `length` is the length of the bytes to read. The bytes are written
// to `bytes`.
Result ReadBytesAcrossBlocks(const disk::DiskManager &disk_manager,
                             const LogBlock &block,
                             const disk::DiskPosition &position, int length,
                             std::vector<uint8_t> &bytes) {
    if (position.Offset() < kDefaultOffset) return Error("fail");

    const int log_block_size = disk_manager.BlockSize() - kDefaultOffset;

    int length_of_first_block = (position.Offset() + length <= log_block_size
                                     ? length
                                     : log_block_size - position.Offset());
    Result read_result        = block.RawBlock().ReadBytes(
        position.Offset(), length_of_first_block, bytes);
    if (read_result.IsError()) return read_result + Error("fail");
    length -= length_of_first_block;

    if (length == 0) return Ok();

    disk::BlockID current_block_id = position.BlockID();
    LogBlock current_block         = block;

    while (length > 0) {
        current_block_id += 1;
        Result read_result =
            current_block.ReadLogBlock(disk_manager, current_block_id);
        if (read_result.IsError()) return read_result + Error("fail");

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

ResultV<uint32_t> ReadUint32AcrossBlocks(const disk::DiskManager &disk_manager,
                                         const LogBlock &block,
                                         const disk::DiskPosition &position) {
    std::vector<uint8_t> uint32_bytes(data::kUint32Bytesize);
    auto read_result = ReadBytesAcrossBlocks(
        disk_manager, block, position, data::kUint32Bytesize, uint32_bytes);
    if (read_result.IsError()) { return read_result + Error("fail"); }
    return data::ReadUint32(uint32_bytes, 0);
}

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
Result ReadBytesAcrossBlocksWithOffset(const disk::DiskManager &disk_manager,
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
    if (read_result.IsError()) return read_result + Error("fail");
    return ReadBytesAcrossBlocks(disk_manager, start_block, start_position,
                                 length, bytes);
}

ResultV<int> ReadIntAcrossBlocksWithOffset(
    const disk::DiskManager &disk_manager, const LogBlock &block,
    const disk::DiskPosition &position, const int offset) {
    std::vector<uint8_t> int_bytes(data::kIntBytesize);
    auto read_result = ReadBytesAcrossBlocksWithOffset(
        disk_manager, block, position, offset, data::kIntBytesize, int_bytes);
    if (read_result.IsError()) { return read_result + Error("fail"); }
    return data::ReadInt(int_bytes, 0);
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
    data::WriteUint32NoFail(log_body_with_header, log_body_with_header.size(),
                            log_body.size());
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

ResultV<LogSequenceNumber> LogManager::WriteLog(const LogRecord *log_record) {
    std::vector<uint8_t> log_body_with_header = LogRecordWithHeader(log_record);

    const disk::BlockID rollback_block_id   = current_block_id_;
    const internal::LogBlock rollback_block = current_block_;

    ResultE<size_t> append_result =
        current_block_.Append(log_body_with_header, /*bytes_offset=*/0);
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