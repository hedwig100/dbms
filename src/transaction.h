#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "buffer.h"
#include "concurrency.h"
#include "data/data.h"
#include "log.h"
#include "recovery.h"
#include "result.h"

namespace transaction {

class TransactionInterface {
  public:
    // Reads bytes from `position` with `length`.
    virtual ResultV<std::vector<uint8_t>>
    ReadBytes(const disk::DiskPosition &position, const size_t length) = 0;

    // Reads int from `position`.
    virtual ResultV<int> ReadInt(const disk::DiskPosition &position) = 0;

    // Reads string with `length` from `position`.
    virtual ResultV<std::string> ReadString(const disk::DiskPosition &position,
                                            const size_t length) = 0;

    // Writes the data to `position`.
    virtual Result Write(const disk::DiskPosition &position,
                         const data::DataItem &data) = 0;

    // Commits the transaction.
    virtual Result Commit() = 0;

    // Rollbacks the transaction.
    virtual Result Rollback() = 0;

    // Returns the size of the file.
    virtual ResultV<size_t> Size(const std::string &filename) = 0;

    // Allocates new blocks for the file.
    virtual Result AllocateNewBlocks(const disk::BlockID &block_id) = 0;
};

// Transaction manages the data and the log records.
// If one methods fails (returns Error), the transaction rolls back itself.
// Thus, rollback is not user's responsibility even if the method fails.
class Transaction : TransactionInterface {
  public:
    Transaction(disk::DiskManager &disk_manager,
                buffer::BufferManager &buffer_manager,
                dblog::LogManager &log_manager,
                dbconcurrency::LockTable &lock_table);

    ResultV<std::vector<uint8_t>> ReadBytes(const disk::DiskPosition &position,
                                            const size_t length);

    ResultV<int> ReadInt(const disk::DiskPosition &position);

    ResultV<std::string> ReadString(const disk::DiskPosition &position,
                                    const size_t length);

    Result Write(const disk::DiskPosition &position,
                 const data::DataItem &data);

    Result Commit();

    Result Rollback();

    ResultV<size_t> Size(const std::string &filename);

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