#ifndef _CONCURRENCY_H
#define _CONCURRENCY_H

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

    // Release lock of `block_id`.
    void Release(const disk::BlockID &block_id);

  private:
    std::chrono::milliseconds wait_time_;
    std::mutex lock_table_mutex_;
    std::condition_variable read_write_condition_;
    std::map<disk::BlockID, int> lock_table_;
};

} // namespace dbconcurrency

#endif
