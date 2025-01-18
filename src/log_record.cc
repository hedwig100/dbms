#include "log_record.h"
#include "data/char.h"
#include "data/data.h"
#include "data/data_read.h"
#include "data/int.h"
#include "data/uint32.h"

namespace dblog {

constexpr size_t kLogTransactionBeginByteSize = 5;
constexpr size_t kLogTransactionEndByteSize   = 5;

constexpr uint8_t kLogTransactionBeginMask = 0b00000000;
constexpr uint8_t kLogOperationMask        = 0b01000000;
constexpr uint8_t kLogTransactionEndMask   = 0b10000000;
constexpr uint8_t kLogCheckpointingMask    = 0b11000000;

constexpr uint8_t kUpdateFlag = 0b00010000;

constexpr uint8_t kCommitFlag   = 0b00000000;
constexpr uint8_t kRollbackFlag = 0b00100000;

inline bool IsTransactionBegin(uint8_t log_header) {
    return (log_header & 0b11000000) == kLogTransactionBeginMask;
}

inline bool IsOperation(uint8_t log_header) {
    return (log_header & 0b11000000) == kLogOperationMask;
}

inline bool IsTransactionEnd(uint8_t log_header) {
    return (log_header & 0b11000000) == kLogTransactionEndMask;
}

inline bool IsCheckpointing(uint8_t log_header) {
    return (log_header & 0b11000000) == kLogCheckpointingMask;
}

inline bool IsCommit(uint8_t log_header) {
    return (log_header & 0b00110000) == kCommitFlag;
}

inline bool IsRollback(uint8_t log_header) {
    return (log_header & 0b00110000) == kRollbackFlag;
}

ResultV<std::unique_ptr<LogRecord>>
ReadLogTransactionBegin(const std::vector<uint8_t> &log_body_bytes) {
    ResultV<uint32_t> transaction_id_result =
        data::ReadUint32(log_body_bytes, 1);
    if (transaction_id_result.IsError()) {
        return transaction_id_result + Error("dblog::ReadLogTransactionBegin() "
                                             "failed to read transaction id.");
    }
    return ResultV<std::unique_ptr<LogRecord>>(std::move(
        std::make_unique<LogTransactionBegin>(transaction_id_result.Get())));
}

ResultV<std::unique_ptr<LogRecord>> ReadLogOperationUpdate(
    TransactionID transaction_id, const disk::DiskPosition &offset,
    const std::vector<uint8_t> &log_body_bytes, int bytes_offset) {
    ResultV<std::unique_ptr<data::DataItem>> prevdata_result =
        data::ReadTypeData(log_body_bytes, bytes_offset);
    if (prevdata_result.IsError())
        return prevdata_result + Error("dblog::ReadLogOperationUpdate() failed "
                                       "to read previous data.");
    bytes_offset += prevdata_result.Get()->Type().TypeParameterValueLength();

    ResultV<std::unique_ptr<data::DataItem>> newdata_result = data::ReadData(
        prevdata_result.Get()->Type(), log_body_bytes, bytes_offset);
    if (newdata_result.IsError())
        return newdata_result + Error("dblog::ReadLogOperationUpdate() failed "
                                      "to read the new data.");

    return ResultV<std::unique_ptr<LogRecord>>(std::move(
        std::make_unique<LogOperation>(transaction_id, offset,
                                       *prevdata_result.MoveValue().get(),
                                       *newdata_result.MoveValue().get())));
}

ResultV<std::unique_ptr<LogRecord>>
ReadLogOperation(const std::vector<uint8_t> &log_body_bytes) {
    int offset = 1;

    ResultV<uint32_t> transaction_id_result =
        data::ReadUint32(log_body_bytes, offset);
    if (transaction_id_result.IsError()) {
        return transaction_id_result +
               Error(
                   "dblog::ReadLogOperation() failed to read transaction id.");
    }
    offset += data::kUint32Bytesize;

    ResultV<uint32_t> filename_length_result =
        data::ReadUint32(log_body_bytes, offset);
    if (filename_length_result.IsError()) {
        return filename_length_result +
               Error("dblog::ReadLogOperation() failed to read filename.");
    }
    offset += data::kUint32Bytesize;
    const uint32_t filename_length = filename_length_result.Get();

    ResultV<std::string> filename_result =
        data::ReadString(log_body_bytes, offset,
                         /*length=*/filename_length);
    if (filename_result.IsError()) {
        return filename_result +
               Error("dblog::ReadLogOperation() failed to read filename.");
    }
    offset += filename_length;

    ResultV<int> blockindex_result = data::ReadInt(log_body_bytes, offset);
    if (blockindex_result.IsError()) {
        return blockindex_result +
               Error("dblog::ReadLogOperation() failed to read block_index.");
    }
    offset += data::kIntBytesize;

    ResultV<int> offset_result = data::ReadInt(log_body_bytes, offset);
    if (offset_result.IsError()) {
        return offset_result + Error("dblog::ReadLogOperation() failed to read "
                                     "offset of the block.");
    }
    offset += data::kIntBytesize;

    return ReadLogOperationUpdate(
        transaction_id_result.Get(),
        disk::DiskPosition(
            disk::BlockID(filename_result.Get(), blockindex_result.Get()),
            offset_result.Get()),
        log_body_bytes, offset);
}

ResultV<std::unique_ptr<LogRecord>>
ReadLogTransactionEnd(const std::vector<uint8_t> &log_body_bytes) {
    ResultV<uint32_t> transaction_id_result =
        data::ReadUint32(log_body_bytes, 1);
    if (transaction_id_result.IsError()) {
        return transaction_id_result + Error("dblog::ReadLogTransactionEnd() "
                                             "failed to read transaction_id.");
    }

    TransactionEndType transaction_end_type;
    if (IsCommit(log_body_bytes[0]))
        transaction_end_type = TransactionEndType::kCommit;
    else if (IsRollback(log_body_bytes[0]))
        transaction_end_type = TransactionEndType::kRollback;
    else
        return Error("dblog::ReadLogTransactionEnd() transaction type should "
                     "be either Commit or Rollback.");

    return ResultV<std::unique_ptr<LogRecord>>(
        std::move(std::make_unique<LogTransactionEnd>(
            transaction_id_result.Get(), transaction_end_type)));
}

ResultV<std::unique_ptr<LogRecord>>
ReadLogCheckpointing(const std::vector<uint8_t> &log_body_bytes) {
    return ResultV<std::unique_ptr<LogRecord>>(
        std::move(std::make_unique<LogCheckpointing>()));
}

ResultV<std::unique_ptr<LogRecord>>
ReadLogRecord(const std::vector<uint8_t> &log_body_bytes) {
    if (IsTransactionBegin(log_body_bytes[0]))
        return ReadLogTransactionBegin(log_body_bytes);
    else if (IsOperation(log_body_bytes[0]))
        return ReadLogOperation(log_body_bytes);
    else if (IsTransactionEnd(log_body_bytes[0]))
        return ReadLogTransactionEnd(log_body_bytes);
    else if (IsCheckpointing(log_body_bytes[0]))
        return ReadLogCheckpointing(log_body_bytes);
    return Error("fail");
}

// Returns mask bits of a transaction-end log.
uint8_t LogTransactionEndMask(TransactionEndType transaction_end_type) {
    switch (transaction_end_type) {
    case TransactionEndType::kCommit:
        return kLogTransactionEndMask | kCommitFlag;
    case TransactionEndType::kRollback:
        return kLogTransactionEndMask | kRollbackFlag;
    }
    return 0;
}

LogTransactionBegin::LogTransactionBegin(const TransactionID transaction_id)
    : transaction_id_(transaction_id) {
    log_body_.resize(kLogTransactionBeginByteSize);
    log_body_[0] = kLogTransactionBeginMask;
    data::WriteUint32(log_body_, 1, transaction_id_);
}

// This is an super roughly estimated average size of log operation.
constexpr size_t kEstimatedAverageLogSize = 24;

LogOperation::LogOperation(TransactionID transaction_id,
                           const disk::DiskPosition &offset,
                           const data::DataItem &previous_item,
                           const data::DataItem &new_item)
    : transaction_id_(transaction_id), offset_(offset) {
    log_body_.reserve(kEstimatedAverageLogSize);

    log_body_.push_back(kLogOperationMask);
    data::WriteUint32NoFail(log_body_, log_body_.size(), transaction_id);
    data::WriteUint32NoFail(log_body_, log_body_.size(),
                            offset.BlockID().Filename().size());
    data::WriteStringNoFail(log_body_, log_body_.size(),
                            offset.BlockID().Filename());
    data::WriteIntNoFail(log_body_, log_body_.size(),
                         offset.BlockID().BlockIndex());
    data::WriteIntNoFail(log_body_, log_body_.size(), offset.Offset());

    previous_item.Type().WriteTypeParameter(log_body_, log_body_.size());
    previous_item_offset_in_log_body_ = log_body_.size();
    previous_item.WriteNoFail(log_body_, log_body_.size());
    new_item_offset_in_log_body_ = log_body_.size();
    new_item.WriteNoFail(log_body_, log_body_.size());
}

Result LogOperation::UnDo(disk::DiskManager &disk_manager) const {
    disk::Block block;
    Result result = disk_manager.Read(offset_.BlockID(), block);
    if (result.IsError())
        return result +
               Error("dblog::LogOperation::Undo() failed read data block.");

    ResultE<size_t> size_result = block.WriteBytesWithOffset(
        offset_.Offset(), log_body_, previous_item_offset_in_log_body_);
    if (size_result.IsError())
        return Error("dblog::LogOperation::Undo() failed to write previous "
                     "item to the block.");

    result = disk_manager.Write(offset_.BlockID(), block);
    if (result.IsError())
        return result + Error("dblog::LogOperation::Undo() failed to "
                              "write block to disk.");
    return Ok();
}

Result LogOperation::ReDo(disk::DiskManager &disk_manager) const {
    disk::Block block;
    Result result = disk_manager.Read(offset_.BlockID(), block);
    if (result.IsError())
        return result +
               Error("dblog::LogOperation::Redo() failed read data block.");

    ResultE<size_t> size_result = block.WriteBytesWithOffset(
        offset_.Offset(), log_body_, new_item_offset_in_log_body_);
    if (size_result.IsError())
        return Error("dblog::LogOperation::Redo() failed to write new "
                     "item to the block.");

    result = disk_manager.Write(offset_.BlockID(), block);
    if (result.IsError())
        return result + Error("dblog::LogOperation::Redo() failed to "
                              "write block to disk.");
    return Ok();
}

LogTransactionEnd::LogTransactionEnd(TransactionID transaction_id,
                                     TransactionEndType transaction_end_type)
    : transaction_id_(transaction_id),
      transaction_end_type_(transaction_end_type) {
    log_body_.resize(kLogTransactionEndByteSize);
    log_body_[0] = LogTransactionEndMask(transaction_end_type_);
    data::WriteUint32(log_body_, 1, transaction_id_);
}

LogCheckpointing::LogCheckpointing() { log_body_ = {kLogCheckpointingMask}; }

} // namespace dblog