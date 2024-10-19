#ifndef _LOG_RECORD_H
#define _LOG_RECORD_H

#include "data/data.h"
#include "disk.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace dblog {

// Types of log records
enum class LogType {
    // Indicates that a transaction begins.
    kTransactionBegin = 0,

    // Indicates that an item is inserted, updated or deleted in the disk.
    kOperation = 1,

    // Indicates that a transaction ends.
    kTransactionEnd = 2,

    // Indicates that a checkpointing finishes.
    kCheckpointing = 3,
};

// The base class for all types of log records
class LogRecord {
  public:
    // The log type
    virtual LogType Type() const = 0;

    // The log body which is written to the log file with a header
    virtual const std::vector<uint8_t> &LogBody() const = 0;

    // Appends the log body to `bytes`.
    void AppendLogBody(std::vector<uint8_t> &bytes) const {
        const std::vector<uint8_t> &log_body = this->LogBody();
        std::copy(log_body.begin(), log_body.end(), std::back_inserter(bytes));
    }
};

// Read LogRecord from `log_body_bytes`.
ResultV<std::unique_ptr<LogRecord>>
ReadLogRecord(const std::vector<uint8_t> &log_body_bytes);

// TransactionID is represented as an unsigned 32-bit integer.
using TransactionID = uint32_t;

// Log record which indicates that a transaction begins.
class LogTransactionBegin : public LogRecord {
  public:
    explicit LogTransactionBegin(const TransactionID transaction_id);
    inline LogType Type() const { return LogType::kTransactionBegin; }
    inline const std::vector<uint8_t> &LogBody() const { return log_body_; }

  private:
    TransactionID transaction_id_;
    std::vector<uint8_t> log_body_;
};

// Maniplation type to an item (like integer, char) in disk
enum class ManiplationType {
    // Inserts to a disk
    kInsert = 0,

    // Updates a disk
    kUpdate = 1,

    // Deletes a disk
    kDelete = 2,
};

// Log record which indicates that an item is inserted, updated or deleted in
// the disk.
class LogOperation : public LogRecord {
  public:
    // Initialize a LogOperation log. Either of `previous_item` or `new_item`
    // must be non-null pointer.
    LogOperation(TransactionID transaction_id, ManiplationType maniplation_type,
                 const disk::DiskPosition &offset,
                 std::unique_ptr<data::DataItem> previous_item,
                 std::unique_ptr<data::DataItem> new_item);
    inline LogType Type() const { return LogType::kOperation; }
    inline const std::vector<uint8_t> &LogBody() const { return log_body_; }

  private:
    TransactionID transaction_id_;
    ManiplationType maniplation_type_;
    disk::DiskPosition offset_;
    std::unique_ptr<data::DataItem> previous_item_;
    std::unique_ptr<data::DataItem> new_item_;
    std::vector<uint8_t> log_body_;
};

// Types of the end of transctions.
enum class TransactionEndType {
    // Indicates that the transaction successfully commits.
    kCommit = 0,

    // Indicates that the transaction fails and rollbacks.
    kRollback = 1,
};

// Log record which indicates that a transaction ends.
class LogTransactionEnd : public LogRecord {
  public:
    LogTransactionEnd(TransactionID transaction_id,
                      TransactionEndType transaction_end_type);
    inline LogType Type() const { return LogType::kTransactionEnd; }
    const std::vector<uint8_t> &LogBody() const { return log_body_; }

  private:
    TransactionID transaction_id_;
    TransactionEndType transaction_end_type_;
    std::vector<uint8_t> log_body_;
};

// Log record which indicates that a checkpointing finishes.
class LogCheckpointing : public LogRecord {
  public:
    LogCheckpointing();
    inline LogType Type() const { return LogType::kCheckpointing; }
    inline const std::vector<uint8_t> &LogBody() const { return log_body_; }

  private:
    std::vector<uint8_t> log_body_;
};

} // namespace dblog

#endif