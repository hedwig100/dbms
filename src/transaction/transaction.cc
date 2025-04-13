#include "transaction.h"
#include "data/byte.h"
#include "data/char.h"
#include "data/int.h"

namespace transaction {

dblog::TransactionID NextTransactionID() {
    static dblog::TransactionID transaction_id = 0;
    return transaction_id++;
}

Transaction::Transaction(disk::DiskManager &disk_manager,
                         buffer::BufferManager &buffer_manager,
                         dblog::LogManager &log_manager,
                         dbconcurrency::LockTable &lock_table)
    : transaction_id_(NextTransactionID()), disk_manager_(disk_manager),
      buffer_manager_(buffer_manager),
      concurrent_manager_(dbconcurrency::ConcurrentManager(lock_table)),
      recovery_manager_(recovery::RecoveryManager(log_manager)) {}

#define ROLLBACK(result_name)                                                  \
    {                                                                          \
        Result rollback_result = Rollback();                                   \
        if (rollback_result.IsError()) {                                       \
            return result_name + rollback_result;                              \
        }                                                                      \
    }

Result Transaction::Write(const disk::DiskPosition &position, const int length,
                          const data::DataItem &item) {
    Result lock_result = concurrent_manager_.WriteLock(position.BlockID());
    if (lock_result.IsError()) {
        ROLLBACK(lock_result);
        return lock_result + Error("transaction::Transaction::"
                                   "Write() failed to "
                                   "lock the block.");
    }

    disk::Block block;
    Result result = buffer_manager_.Read(position.BlockID(), block);
    if (result.IsError()) {
        ROLLBACK(result);
        return result + Error("transaction::Transaction::"
                              "Write() failed to read "
                              "the block.");
    }

    std::vector<uint8_t> previous_item_bytes;
    Result previous_data =
        block.ReadBytes(position.Offset(), length, previous_item_bytes);
    if (previous_data.IsError()) {
        ROLLBACK(previous_data);
        return previous_data + Error("transaction::Transaction::"
                                     "Write() failed "
                                     "to read the previous data.");
    }

    ResultV<dblog::LogSequenceNumber> lsn_result =
        recovery_manager_.WriteLog(dblog::LogOperation(
            transaction_id_, position, length, previous_item_bytes, item));
    if (lsn_result.IsError()) {
        ROLLBACK(lsn_result);
        return lsn_result + Error("transaction::Transaction::"
                                  "Write() failed to "
                                  "write a log record.");
    }

    Result write_result = block.Write(position.Offset(), length, item);
    if (write_result.IsError()) {
        ROLLBACK(write_result);
        return write_result + Error("transaction::Transaction::"
                                    "Write() failed "
                                    "to write the data.");
    }

    write_result =
        buffer_manager_.Write(position.BlockID(), block, lsn_result.Get());
    if (write_result.IsError()) {
        ROLLBACK(write_result);
        return write_result + Error("transaction::Transaction::"
                                    "Write() failed "
                                    "to write the block.");
    }

    return Ok();
}

Result Transaction::Read(const disk::DiskPosition &position, const int length,
                         data::DataItem &item) {
    Result lock_result = concurrent_manager_.ReadLock(position.BlockID());
    if (lock_result.IsError()) {
        ROLLBACK(lock_result);
        return lock_result + Error("transaction::Transaction::ReadByte() "
                                   "failed to lock the block.");
    }

    disk::Block block;
    Result read_result = buffer_manager_.Read(position.BlockID(), block);
    if (read_result.IsError()) {
        ROLLBACK(read_result);
        return read_result + Error("transaction::Transaction::ReadByte() "
                                   "failed to read the block.");
    }

    read_result = block.Read(position.Offset(), length, item);
    if (read_result.IsError()) {
        ROLLBACK(read_result);
        return read_result + Error("transaction::Transaction::ReadByte() "
                                   "failed to read the byte.");
    }

    return Ok();
}

ResultV<int> Transaction::ReadInt(const disk::DiskPosition &position) {
    data::DataItem item;
    FIRST_TRY(Read(position, data::kTypeInt.ValueLength(), item));
    return Ok(data::ReadInt(item));
}

Result Transaction::Commit() {
    Result commit_result = recovery_manager_.Commit(transaction_id_);
    if (commit_result.IsError()) {
        ROLLBACK(commit_result);
        return commit_result +
               Error("transaction::Transaction::Commit() failed to "
                     "commit the transaction.");
    }

    concurrent_manager_.Release();
    return Ok();
}

Result Transaction::Rollback() {
    Result rollback_result =
        recovery_manager_.Rollback(transaction_id_, buffer_manager_);

    // Rollback method must release locks whenever it is called.
    concurrent_manager_.Release();

    if (rollback_result.IsError()) {
        return rollback_result + Error("transaction::Transaction::Rollback() "
                                       "failed to rollback the transaction.");
    }
    return Ok();
}

ResultV<size_t> Transaction::Size(const std::string &filename) {
    disk::BlockID eof_block_id = disk::EndOfFileBlockID(filename);
    Result lock_result         = concurrent_manager_.ReadLock(eof_block_id);
    if (lock_result.IsError()) {
        ROLLBACK(lock_result);
        return lock_result + Error("transaction::Transaction::"
                                   "Size() failed to "
                                   "lock the end of file block.");
    }

    ResultV<size_t> size_result = disk_manager_.Size(filename);
    if (size_result.IsError()) {
        ROLLBACK(size_result);
        return size_result +
               Error("transaction::Transaction::"
                     "Size() failed to get the size of the file.");
    }

    return size_result;
}

Result Transaction::AllocateNewBlocks(const disk::BlockID &block_id) {
    disk::BlockID eof_block_id = disk::EndOfFileBlockID(block_id.Filename());
    Result lock_result         = concurrent_manager_.WriteLock(eof_block_id);
    if (lock_result.IsError()) {
        ROLLBACK(lock_result);
        return lock_result +
               Error("transaction::Transaction::AllocateNewBlocks() "
                     "failed to lock the end of file block.");
    }

    Result allocate_result = disk_manager_.AllocateNewBlocks(block_id);
    if (allocate_result.IsError()) {
        ROLLBACK(allocate_result);
        return allocate_result +
               Error("transaction::Transaction::AllocateNewBlocks() "
                     "failed to allocate new blocks.");
    }

    return Ok();
}

} // namespace transaction