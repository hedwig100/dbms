#include "recovery.h"

namespace recovery {

RecoveryManager::RecoveryManager(dblog::LogManager &&log_manger)
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

Result RecoveryManager::Rollback(const dblog::TransactionID transaction_id,
                                 const disk::DiskManager &data_disk_manager) {
    ResultV<dblog::LogIterator> log_iter_result = log_manager_.LastLog();
    if (log_iter_result.IsError()) {
        return log_iter_result + Error("dblog::RecoveryManager::Rollback() "
                                       "failed to read the last log.");
    }
    dblog::LogIterator log_iter = log_iter_result.MoveValue();

    while (true) {
        ResultV<std::vector<uint8_t>> log_body_result = log_iter.LogBody();
        if (log_body_result.IsError()) {
            return log_body_result +
                   Error("dblog::RecoveryManager::Rollback() "
                         "failed to read the log body bytes.");
        }
        ResultV<std::unique_ptr<dblog::LogRecord>> log_record_result =
            dblog::ReadLogRecord(log_body_result.Get());
        if (log_record_result.IsError()) {
            return log_record_result +
                   Error("dblog::RecoveryManager::Rollback("
                         ") failed to read the log record.");
        }
        std::unique_ptr<dblog::LogRecord> log_record =
            log_record_result.MoveValue();

        if (log_record->Type() != dblog::LogType::kCheckpointing &&
            log_record->GetTransactionID() == transaction_id) {
            if (log_record->Type() == dblog::LogType::kTransactionBegin) {
                break;
            }
            Result undo_result = log_record->UnDo(data_disk_manager);
            if (undo_result.IsError())
                return undo_result + Error("dblog::RecoveryManager::Rollback() "
                                           "failed to do undo operation.");
        }

        if (!log_iter.HasPrevious()) break;
        Result previous_result = log_iter.Previous();
        if (previous_result.IsError()) {
            return previous_result + Error("dblog::RecoveryManager::Rollback() "
                                           "failed to read the previous log.");
        }
    }

    ResultV<dblog::LogSequenceNumber> rollback_write_result =
        this->WriteLog(dblog::LogTransactionEnd(
            transaction_id, dblog::TransactionEndType::kRollback));
    if (rollback_write_result.IsError())
        return rollback_write_result +
               Error("recovery::RecoveryManager::Rollback()"
                     " failed to write a rollback record.");
    return Ok();
}

Result RecoveryManager::Recover() {
    // TODO: implement recovery algorithm.
    return Ok();
}

} // namespace recovery