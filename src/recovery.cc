#include "recovery.h"

namespace recovery {

explicit RecoveryManager::RecoveryManager(dblog::LogManager &&log_manger)
    : log_manager_(log_manger) {}

ResultV<dblog::LogSequenceNumber>
RecoveryManager::WriteLog(const dblog::LogRecord &log_record) {
    ResultV<dblog::LogSequenceNumber> write_result =
        log_manager_.WriteLog(dblog::LogRecordWithHeader(log_record));
    if (write_result.IsError())
        return write_result + Error("recovery::RecoveryManager::WriteLog() "
                                    "failed to write log.");
    return write_result;
}

Result RecoveryManager::Commit(const dblog::TransactionID transaction_id) {
    ResultV<dblog::LogSequenceNumber> commit_write_result =
        this->WriteLog(dblog::LogTransactionEnd(
            transaction_id, dblog::TransactionEndType::kCommit));
    if (commit_write_result.IsError())
        return commit_write_result + Error("recovery::RecoveryManager::Commit()"
                                           " failed to write commit record.");

    Result flush_result = log_manager_.Flush();
    if (flush_result.IsError())
        return flush_result + Error("recovery::RecoveryManager::Commit() "
                                    "failed to flush logs.");
    return Ok();
}

Result RecoveryManager::Rollback(const dblog::TransactionID transaction_id) {
    ResultV<dblog::LogIterator> log_iter = log_manager_.LastLog();
    if (log_iter.IsError())
        return log_iter + Error("recovery::RecoveryManager::Rollback() failed "
                                "to read the last log record.");
    // TODO: implement log iteration
    return Ok();
}

Result RecoveryManager::Recover() {
    // TODO: implement recovery algorithm.
    return Ok();
}

} // namespace recovery