#ifndef _LOG_RECORD_H
#define _LOG_RECORD_H

#include "data/data.h"
#include "disk.h"
#include <cstdint>
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
    virtual std::vector<uint8_t> LogBody() const = 0;
};

// TransactionID is represented as an unsigned 32-bit integer.
using TransactionID = uint32_t;

// Log record which indicates that a transaction begins.
class LogTransactionBegin : public LogRecord {
  public:
    inline explicit LogTransactionBegin(const TransactionID transaction_id)
        : transaction_id_(transaction_id) {}
    std::vector<uint8_t> LogBody() const;
    inline LogType Type() const { return LogType::kTransactionBegin; }

  private:
    TransactionID transaction_id_;
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
                 const disk::BlockID &offset,
                 const data::DataItem *previous_item,
                 const data::DataItem *new_item);
    inline LogType Type() const { return LogType::kOperation; }
    inline std::vector<uint8_t> LogBody() const { return log_body_; }

  private:
    TransactionID transaction_id_;
    ManiplationType maniplation_type_;
    disk::BlockID offset_;
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
    std::vector<uint8_t> LogBody() const;

  private:
    TransactionID transaction_id_;
    TransactionEndType transaction_end_type_;
};

// Log record which indicates that a checkpointing finishes.
class LogCheckpointing : public LogRecord {
  public:
    inline LogCheckpointing() {}
    inline LogType Type() const { return LogType::kCheckpointing; }
    std::vector<uint8_t> LogBody() const;
};

} // namespace dblog

#endif