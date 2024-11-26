#ifndef TRANSACTION_H
#define TRANSACTION_H

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
class Transaction {
  public:
    Transaction(buffer::BufferManager &buffer_manager,
                dblog::LogManager log_manager,
                dbconcurrency::LockTable &lock_table);

    // Reads the data from `position` and writes it to `data`.
    ResultV<std::unique_ptr<data::DataItem>>
    Transaction::Read(const disk::DiskPosition &position,
                      const data::DataType &type);

    // Writes the data to `position`.
    Result Write(const disk::DiskPosition &position,
                 const data::DataItem &data);

    // Commits the transaction.
    Result Commit();

    // Rollbacks the transaction.
    Result Rollback();

    // Returns the size of the file.
    ResultV<int> Size(const std::string &filename) const;

    // Allocates new blocks for the file.
    Result AllocateNewBlocks(const disk::BlockID &block_id) const;

  private:
    dblog::TransactionID transaction_id_;
    buffer::BufferManager &buffer_manager_;
    dbconcurrency::ConcurrentManager concurrent_manager_;
    recovery::RecoveryManager recovery_manager_;
};

} // namespace transaction

#endif // TRANSACTION_H