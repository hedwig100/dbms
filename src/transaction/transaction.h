#ifndef _TRANSACTION_TRANSACTION_H
#define _TRANSACTION_TRANSACTION_H

#include "buffer.h"
#include "concurrency.h"
#include "data/data.h"
#include "log.h"
#include "recovery.h"
#include "result.h"

namespace transaction {

// Returns the next transaction id which is unique in the system.
dblog::TransactionID NextTransactionID();

// Transaction manages the data and the log records.
// If one methods fails (returns Error), the transaction rolls back itself.
// Thus, rollback is not user's responsibility even if the method fails.
class Transaction {
  public:
    Transaction(disk::DiskManager &disk_manager,
                buffer::BufferManager &buffer_manager,
                dblog::LogManager &log_manager,
                dbconcurrency::LockTable &lock_table);

    // Blocksize of the disk.
    inline int BlockSize() const { return disk_manager_.BlockSize(); }

    // Writes `item` of `type` to `position`.
    Result Write(const disk::DiskPosition &position, const int length,
                 const data::DataItem &item);

    Result Read(const disk::DiskPosition &position, const int length,
                data::DataItem &item);

    // Reads int from `position`.
    ResultV<int> ReadInt(const disk::DiskPosition &position);

    // Commits the transaction.
    Result Commit();

    // Rollbacks the transaction.
    Result Rollback();

    // Returns the size of the file.
    ResultV<size_t> Size(const std::string &filename);

    // Allocates new blocks for the file.
    Result AllocateNewBlocks(const disk::BlockID &block_id);

  private:
    dblog::TransactionID transaction_id_;
    disk::DiskManager &disk_manager_;
    buffer::BufferManager &buffer_manager_;
    dbconcurrency::ConcurrentManager concurrent_manager_;
    recovery::RecoveryManager recovery_manager_;
};

} // namespace transaction

#endif // TRANSACTION_H