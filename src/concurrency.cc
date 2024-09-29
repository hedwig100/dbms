#include "concurrency.h"
#include <chrono>

namespace dbconcurrency {

inline bool IsReadLocked(int lock) { return lock > 0; }

inline bool IsWriteLocked(int lock) { return lock < 0; }

inline bool IsNotLocked(int lock) { return lock == 0; }

inline void LockRead(int &lock) { lock++; }

inline void UnlockRead(int &lock) { lock--; }

inline void LockWrite(int &lock) { lock = -1; }

inline void UnlockWrite(int &lock) { lock = 0; }

Result LockTable::ReadLock(const disk::BlockID &block_id) {
    std::unique_lock<std::mutex> lock(lock_table_mutex_);
    if (read_write_condition_.wait_for(lock, wait_time_, [this, block_id] {
            int lock = lock_table_[block_id];
            return IsNotLocked(lock) || IsReadLocked(lock);
        })) {
        LockRead(lock_table_[block_id]);
        return Ok();
    }
    return Error("dbconcurrency::LockTable::ReadLock() failed to acquire read "
                 "lock in time limit.");
}

Result LockTable::WriteLock(const disk::BlockID &block_id) {
    std::unique_lock<std::mutex> lock(lock_table_mutex_);
    if (read_write_condition_.wait_for(lock, wait_time_, [this, block_id] {
            return IsNotLocked(lock_table_[block_id]);
        })) {
        LockWrite(lock_table_[block_id]);
        return Ok();
    }
    return Error("dbconcurrency::LockTable::WriteLock() failed to acquire "
                 "write lock in time limit.");
}

void LockTable::Release(const disk::BlockID &block_id) {
    std::unique_lock<std::mutex> lock(lock_table_mutex_);
    if (IsReadLocked(lock_table_[block_id])) {
        UnlockRead(lock_table_[block_id]);
    }
    if (IsWriteLocked(lock_table_[block_id])) {
        UnlockWrite(lock_table_[block_id]);
    }
    read_write_condition_.notify_all();
}

} // namespace dbconcurrency