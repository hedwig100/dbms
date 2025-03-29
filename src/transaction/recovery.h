#ifndef _TRANSACTION_RECOVERY_H
#define _TRANSACTION_RECOVERY_H

#include "buffer.h"
#include "log.h"
#include "log_record.h"
#include "result.h"
#include <set>

namespace recovery {

using namespace result;

// Recovery manager manages log files by using dblog::LogManager and do commit,
// rollback, and recover operations using log records.
class RecoveryManager {
  public:
    explicit RecoveryManager(dblog::LogManager &log_manager);

    // Writes the log of log record. Even after this function terminates, it
    // does not mean the log is flushed to the disk. `Commit` method should be
    // called when the log needs to be flushed.
    ResultV<dblog::LogSequenceNumber>
    WriteLog(const dblog::LogRecord &log_record);

    // Commits the transaction whose id is `transaction_id`.
    Result Commit(const dblog::TransactionID transaction_id);

    // Rollbacks the transaction whose id is `transaction_id`.
    Result Rollback(const dblog::TransactionID transaction_id,
                    buffer::BufferManager &buffer_manager);

    // Recover records from logs.
    Result Recover(buffer::BufferManager &buffer_manager) const;

  private:
    Result UnDoStage(dblog::LogIterator &log_iter,
                     std::set<dblog::TransactionID> &committed,
                     std::set<dblog::TransactionID> &rollbacked,
                     buffer::BufferManager &buffer_manager) const;
    Result ReDoStage(dblog::LogIterator &log_iter,
                     std::set<dblog::TransactionID> &committed,
                     std::set<dblog::TransactionID> &rollbacked,
                     buffer::BufferManager &buffer_manager) const;
    dblog::LogManager &log_manager_;
};

} // namespace recovery

#endif