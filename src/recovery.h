#ifndef _RECOVERY_H
#define _RECOVERY_H

#include "log.h"
#include "log_record.h"
#include "result.h"

namespace recovery {

using namespace result;

// Recovery manager manages log files by using dblog::LogManager and do commit,
// rollback, and recover operations using log records.
class RecoveryManager {
  public:
    explicit RecoveryManager(dblog::LogManager &&log_manager);

    // Writes the log of log record.
    ResultV<dblog::LogSequenceNumber>
    WriteLog(const dblog::LogRecord &log_record);

    // Commits the transaction whose id is `transaction_id`.
    Result Commit(const dblog::TransactionID transaction_id);

    // Rollbacks the transaction whose id is `transaction_id`.
    Result Rollback(const dblog::TransactionID transaction_id);

    // Recover records from logs.
    Result Recover();

  private:
    dblog::LogManager log_manager_;
};

} // namespace recovery

#endif