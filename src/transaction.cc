#include "transaction.h"
#include "data/char.h"
#include "data/data_read.h"
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

// This macro is used to define functions (methods of Transaction) that read
// data from the block. In the function, it locks the block, reads the block.
// `read_block` is the block of code that reads the data from the block, and
// returns the data.
#define READ_FUNCTION(function_name, function_signature, read_block)           \
    function_signature {                                                       \
        Result lock_result = concurrent_manager_.ReadLock(position.BlockID()); \
        if (lock_result.IsError()) {                                           \
            ROLLBACK(lock_result);                                             \
            return lock_result + Error("transaction::Transaction:"             \
                                       ":" #function_name "() failed to "      \
                                       "lock the block.");                     \
        }                                                                      \
                                                                               \
        disk::Block block;                                                     \
        Result read_result = buffer_manager_.Read(position.BlockID(), block);  \
        if (read_result.IsError()) {                                           \
            ROLLBACK(read_result);                                             \
            return read_result + Error("transaction::Transaction:"             \
                                       ":" #function_name "() failed to read " \
                                       "the block.");                          \
        }                                                                      \
                                                                               \
        read_block                                                             \
    }

READ_FUNCTION(ReadBytes,
              ResultV<std::vector<uint8_t>> Transaction::ReadBytes(
                  const disk::DiskPosition &position, const size_t length),
              {
                  std::vector<uint8_t> bytes;
                  Result byte_result =
                      block.ReadBytes(position.Offset(), length, bytes);
                  if (byte_result.IsError()) {
                      ROLLBACK(byte_result);
                      return byte_result + Error("transaction::Transaction::"
                                                 "ReadBytes() failed to "
                                                 "read the bytes.");
                  }
                  return Ok(bytes);
              })

READ_FUNCTION(
    ReadInt,
    ResultV<int> Transaction::ReadInt(const disk::DiskPosition &position), {
        ResultV<int> int_result = block.ReadInt(position.Offset());
        if (int_result.IsError()) {
            ROLLBACK(int_result);
            return int_result + Error("transaction::Transaction::"
                                      "ReadInt() "
                                      "failed to read the int.");
        }
        return int_result;
    })

READ_FUNCTION(ReadString,
              ResultV<std::string> Transaction::ReadString(
                  const disk::DiskPosition &position, const size_t length),
              {
                  ResultV<std::string> char_result =
                      block.ReadString(position.Offset(), length);
                  if (char_result.IsError()) {
                      ROLLBACK(char_result);
                      return char_result + Error("transaction::Transaction::"
                                                 "ReadString() "
                                                 "failed to read the string.");
                  }
                  return char_result;
              })

Result Transaction::Write(const disk::DiskPosition &position,
                          const data::DataItem &data) {
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

    ResultV<std::unique_ptr<data::DataItem>> previous_data =
        block.Read(position.Offset(), data.Type());
    if (previous_data.IsError()) {
        ROLLBACK(previous_data);
        return previous_data + Error("transaction::Transaction::"
                                     "Write() failed "
                                     "to read the previous data.");
    }

    ResultV<dblog::LogSequenceNumber> lsn_result =
        recovery_manager_.WriteLog(dblog::LogOperation(
            transaction_id_, position, *previous_data.MoveValue().get(), data));
    if (lsn_result.IsError()) {
        ROLLBACK(lsn_result);
        return lsn_result + Error("transaction::Transaction::"
                                  "Write() failed to "
                                  "write a log record.");
    }

    Result write_result = block.Write(position.Offset(), data);
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