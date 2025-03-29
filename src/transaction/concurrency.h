#ifndef _TRANSACTION_CONCURRENCY_H
#define _TRANSACTION_CONCURRENCY_H

#include "disk.h"
#include "result.h"
#include <chrono>
#include <condition_variable>
#include <map>
#include <mutex>
#include <vector>

namespace dbconcurrency {

// LockTable manages the read-write lock of blocks.
// All these class methods are thread-safe.
class LockTable {
  public:
    inline explicit LockTable(double wait_time_sec)
        : wait_time_(int(wait_time_sec * 1000)) {}

    // Try to get the read lock of the block. If `wait_time_sec_` passed,
    // returns the failed reesult.
    Result ReadLock(const disk::BlockID &block_id);

    // Try to get the write lock of the block. If `wait_time_sec_` passed,
    // returns the failed reesult.
    Result WriteLock(const disk::BlockID &block_id);

    // Try to get the write lock of the block when the transaction owns read
    // lock of the block. If `wait_time_sec_` passed, returns the failed
    // reesult.
    Result WriteLockWhenOwningReadLock(const disk::BlockID &block_id);

    // Release lock of `block_id`.
    void Release(const disk::BlockID &block_id);

  private:
    std::chrono::milliseconds wait_time_;
    std::mutex lock_table_mutex_;
    std::condition_variable read_write_condition_;
    std::map<disk::BlockID, int> lock_table_;
};

// Manages locked block of one transaction.
class ConcurrentManager {
  public:
    inline ConcurrentManager(LockTable &lock_table) : lock_table_(lock_table) {}

    // Try to acquire read lock of the block.
    Result ReadLock(const disk::BlockID &block_id);

    // Try to acquire write lock of the block.
    Result WriteLock(const disk::BlockID &block_id);

    // Unlock all the locks the manager owns.
    void Release();

  private:
    enum class ReadOrWrite {
        kRead  = 0,
        kWrite = 1,
    };
    LockTable &lock_table_;
    std::map<disk::BlockID, ReadOrWrite> owned_locks_;
};

} // namespace dbconcurrency

#endif
