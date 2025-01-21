#include "concurrency.h"
#include <chrono>

namespace dbconcurrency {

inline bool IsReadLocked(int lock) { return lock > 0; }

inline bool IsLockedByOne(int lock) { return lock == 1; }

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

Result LockTable::WriteLockWhenOwningReadLock(const disk::BlockID &block_id) {
    std::unique_lock<std::mutex> lock(lock_table_mutex_);
    if (read_write_condition_.wait_for(lock, wait_time_, [this, block_id] {
            return IsLockedByOne(lock_table_[block_id]);
        })) {
        LockWrite(lock_table_[block_id]);
        return Ok();
    }
    return Error("dbconcurrency::LockTable::WriteLockWhenOwningReadLock() "
                 "failed to acquire write lock in time limit.");
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

Result ConcurrentManager::ReadLock(const disk::BlockID &block_id) {
    if (owned_locks_.count(block_id)) return Ok();
    Result lock_result = lock_table_.ReadLock(block_id);
    if (lock_result.IsError())
        return lock_result + Error("dbconcurrency::ConcurrentManager::ReadLock("
                                   ") failed to acquire read lock.");
    owned_locks_[block_id] = ReadOrWrite::kRead;
    return Ok();
}

Result ConcurrentManager::WriteLock(const disk::BlockID &block_id) {
    if (owned_locks_.count(block_id)) {
        if (owned_locks_[block_id] == ReadOrWrite::kWrite) return Ok();
        Result lock_result = lock_table_.WriteLockWhenOwningReadLock(block_id);
        if (lock_result.IsError())
            return lock_result +
                   Error("dbconcurrency::ConcurrentManager::WriteLock() failed "
                         "to acquire write lock when owning read lock.");
    } else {
        Result lock_result = lock_table_.WriteLock(block_id);
        if (lock_result.IsError())
            return lock_result +
                   Error("dbconcurrency::ConcurrentManager::WriteLock() failed "
                         "to acquire write lock.");
    }
    owned_locks_[block_id] = ReadOrWrite::kWrite;
    return Ok();
}

void ConcurrentManager::Release() {
    for (auto &[block_id, _] : owned_locks_) {
        lock_table_.Release(block_id);
    }
    owned_locks_.clear();
}

} // namespace dbconcurrency