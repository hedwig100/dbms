#ifndef _RECOVERY_H
#define _RECOVERY_H

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
    // `data_disk_manager` is a disk manager responsible for the data of
    // `transaction_id`.
    Result Rollback(const dblog::TransactionID transaction_id,
                    disk::DiskManager &data_disk_manager);

    // Recover records from logs.
    Result Recover(disk::DiskManager &data_disk_manager) const;

  private:
    Result UnDoStage(dblog::LogIterator &log_iter,
                     std::set<dblog::TransactionID> &committed,
                     std::set<dblog::TransactionID> &rollbacked,
                     disk::DiskManager &data_disk_manager) const;
    Result ReDoStage(dblog::LogIterator &log_iter,
                     std::set<dblog::TransactionID> &committed,
                     std::set<dblog::TransactionID> &rollbacked,
                     disk::DiskManager &data_disk_manager) const;
    dblog::LogManager &log_manager_;
};

} // namespace recovery

#endif