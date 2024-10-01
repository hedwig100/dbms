#include "log_record.h"
#include "data/char.h"
#include "data/data.h"
#include "data/int.h"
#include "data/uint32.h"

namespace dblog {

constexpr size_t kLogTransactionBeginByteSize = 5;
constexpr size_t kLogTransactionEndByteSize   = 5;

constexpr uint8_t kLogTransactionBeginMask = 0b00000000;
constexpr uint8_t kLogOperationMask        = 0b01000000;
constexpr uint8_t kLogTransactionEndMask   = 0b10000000;
constexpr uint8_t kLogCheckpointingMask    = 0b11000000;

constexpr uint8_t kInsertFlag = 0b00000000;
constexpr uint8_t kUpdateFlag = 0b00010000;
constexpr uint8_t kDeleteFlag = 0b00100000;

constexpr uint8_t kIntFlag  = 0b00000000;
constexpr uint8_t kCharFlag = 0b00000001;

constexpr uint8_t kCommitFlag   = 0b00000000;
constexpr uint8_t kRollbackFlag = 0b00100000;

// Returns mask bits of a operation log.
uint8_t LogOperationMask(ManiplationType maniplation_type,
                         data::DataType data_type) {
    uint8_t mask = kLogOperationMask;

    switch (maniplation_type) {
    case ManiplationType::kInsert:
        mask |= kInsertFlag;
        break;
    case ManiplationType::kUpdate:
        mask |= kUpdateFlag;
        break;
    case ManiplationType::kDelete:
        mask |= kDeleteFlag;
        break;
    }

    switch (data_type) {
    case data::DataType::kInt:
        mask |= kIntFlag;
        break;
    case data::DataType::kChar:
        mask |= kCharFlag;
        break;
    }

    return mask;
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
                           ManiplationType maniplation_type,
                           const disk::BlockID &offset,
                           const data::DataItem *previous_item,
                           const data::DataItem *current_item)
    : transaction_id_(transaction_id), maniplation_type_(maniplation_type),
      offset_(offset) {
    const data::DataItem *nonnull_item =
        previous_item != nullptr ? previous_item : current_item;

    log_body_.reserve(kEstimatedAverageLogSize);

    log_body_.push_back(
        LogOperationMask(maniplation_type, nonnull_item->Type()));
    data::WriteUint32NoFail(log_body_, log_body_.size(), transaction_id);
    data::WriteUint32NoFail(log_body_, log_body_.size(),
                            offset.Filename().size());
    data::WriteStringNoFail(log_body_, log_body_.size(), offset.Filename());
    data::WriteIntNoFail(log_body_, log_body_.size(), offset.BlockIndex());
    nonnull_item->WriteTypeParameter(log_body_, log_body_.size());

    if (previous_item != nullptr)
        previous_item->Write(log_body_, log_body_.size());
    if (current_item != nullptr)
        current_item->Write(log_body_, log_body_.size());
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