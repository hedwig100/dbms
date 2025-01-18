#include "recovery.h"

namespace recovery {

RecoveryManager::RecoveryManager(dblog::LogManager &log_manger)
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
                                 disk::DiskManager &data_disk_manager) {
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

Result RecoveryManager::Recover(disk::DiskManager &data_disk_manager) const {
    ResultV<dblog::LogIterator> log_iter_result = log_manager_.LastLog();
    if (log_iter_result.IsError()) {
        return log_iter_result + Error("recovery::RecoveryManager::Recover() "
                                       "failed to read the last log.");
    }
    dblog::LogIterator log_iter = log_iter_result.MoveValue();
    std::set<dblog::TransactionID> committed, rollbacked;

    Result undo_result =
        UnDoStage(log_iter, committed, rollbacked, data_disk_manager);
    if (undo_result.IsError()) {
        return undo_result +
               Error("recovery::RecoveryManager::Recover() failed to undo.");
    }

    Result redo_result =
        ReDoStage(log_iter, committed, rollbacked, data_disk_manager);
    if (redo_result.IsError()) {
        return redo_result +
               Error("recovery::RecoveryManager::Recover() failed to redo.");
    }

    return Ok();
}

Result RecoveryManager::UnDoStage(dblog::LogIterator &log_iter,
                                  std::set<dblog::TransactionID> &committed,
                                  std::set<dblog::TransactionID> &rollbacked,
                                  disk::DiskManager &data_disk_manager) const {

    auto already_committed_or_rollbacked =
        [&committed, &rollbacked](dblog::TransactionID transaction_id) -> bool {
        return committed.count(transaction_id) ||
               rollbacked.count(transaction_id);
    };

    while (true) {
        ResultV<std::vector<uint8_t>> log_body_result = log_iter.LogBody();
        if (log_body_result.IsError()) {
            return log_body_result +
                   Error("recovery::RecoveryManager::UnDoStage() "
                         "failed to read the log body bytes.");
        }
        ResultV<std::unique_ptr<dblog::LogRecord>> log_record_result =
            dblog::ReadLogRecord(log_body_result.Get());
        if (log_record_result.IsError()) {
            return log_record_result +
                   Error("recovery::RecoveryManager::UnDoStage("
                         ") failed to read the log record.");
        }
        std::unique_ptr<dblog::LogRecord> log_record =
            log_record_result.MoveValue();

        if (log_record->Type() == dblog::LogType::kTransactionEnd) {
            switch (log_record->GetTransactionEndType()) {
            case dblog::TransactionEndType::kCommit:
                committed.insert(log_record->GetTransactionID());
                break;
            case dblog::TransactionEndType::kRollback:
                rollbacked.insert(log_record->GetTransactionID());
                break;
            }
        } else if (log_record->Type() == dblog::LogType::kOperation &&
                   !already_committed_or_rollbacked(
                       log_record->GetTransactionID())) {
            Result undo_result = log_record->UnDo(data_disk_manager);
            if (undo_result.IsError()) {
                return undo_result + Error("recovery::RecoveryManager::"
                                           "UnDoStage() failed to undo.");
            }
        }

        if (!log_iter.HasPrevious()) break;
        Result previous_result = log_iter.Previous();
        if (previous_result.IsError()) {
            return previous_result +
                   Error("recovery::RecoveryManager::UnDoStage() "
                         "failed to read the previous log.");
        }
    }

    return Ok();
}

Result RecoveryManager::ReDoStage(dblog::LogIterator &log_iter,
                                  std::set<dblog::TransactionID> &committed,
                                  std::set<dblog::TransactionID> &rollbacked,
                                  disk::DiskManager &data_disk_manager) const {
    while (true) {
        ResultV<std::vector<uint8_t>> log_body_result = log_iter.LogBody();
        if (log_body_result.IsError()) {
            return log_body_result +
                   Error("recovery::RecoveryManager::ReDoStage() "
                         "failed to read the log body bytes.");
        }
        ResultV<std::unique_ptr<dblog::LogRecord>> log_record_result =
            dblog::ReadLogRecord(log_body_result.Get());
        if (log_record_result.IsError()) {
            return log_record_result +
                   Error("recovery::RecoveryManager::ReDoStage("
                         ") failed to read the log record.");
        }
        std::unique_ptr<dblog::LogRecord> log_record =
            log_record_result.MoveValue();

        if (log_record->Type() == dblog::LogType::kOperation &&
            committed.count(log_record->GetTransactionID())) {
            Result redo_result = log_record->ReDo(data_disk_manager);
            if (redo_result.IsError()) {
                return redo_result +
                       Error("recovery::RecoveryManager::"
                             "ReDoStage() failed to redo a record.");
            }
        }

        ResultV<bool> has_next_result = log_iter.HasNext();
        if (has_next_result.IsError()) {
            return has_next_result + Error("recovery::RecoveryManager::"
                                           "ReDoStage() failed to check if the "
                                           "next record exists.");
        }
        if (!has_next_result.Get()) break;
        Result next_result = log_iter.Next();
        if (next_result.IsError()) {
            return next_result + Error("recovery::RecoveryManager::ReDoStage() "
                                       "failed to read the next log.");
        }
    }

    return Ok();
}
} // namespace recovery