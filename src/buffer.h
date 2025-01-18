#ifndef BUFFER_H
#define BUFFER_H

#include "disk.h"
#include "log.h"
#include "result.h"
#include <vector>

using namespace ::result;

namespace buffer {

// Buffer is a class for managing a block and related metadata for the block
// like how often the block is accessed and if the block is pinned.
class Buffer {
  public:
    Buffer();
    Buffer(const disk::BlockID &block_id, const disk::Block &block);

    // Returns block_id of the owned block.
    const disk::BlockID &BlockID() const;

    // Returns the owned block.
    const disk::Block &Block() const;

    // Latest log sequence number of modifications of the block.
    dblog::LogSequenceNumber LatestLogSequenceNumber() const {
        return latest_lsn_;
    }

    // Returns true if the block is dirty (modified).
    bool IsDirty() const { return block_id_.Filename() != ""; }

    // Set the block with a log sequence number.
    void SetBlock(const disk::Block &block, const dblog::LogSequenceNumber lsn);

  private:
    dblog::LogSequenceNumber latest_lsn_;
    disk::BlockID block_id_;
    disk::Block block_;
};

// BufferManager manages the buffer pool and reads and writes blocks to the
// buffer pool. Eviction policy should be implemented in the derived class.
class BufferManager {
  public:
    explicit BufferManager(disk::DiskManager &disk_manager,
                           dblog::LogManager &log_manager);

    // Reads the block of `block_id` from the buffer pool. The block with
    // `block_id` is cached in `buffer_pool_`.
    Result Read(const disk::BlockID &block_id, disk::Block &block);

    // Writes the block of `block_id` to the buffer pool. The block with
    // `block_id` is cached in `buffer_pool_`. When execution of this method
    // succeeds, it does not mean the block is written to disk. You have to call
    // `BufferManager::Flush()` to make sure that the block is written to disk.
    // NOTE: Does't raise an error even if there is no block with
    // `block_id`. Thus, you should make sure that the block with `block_id`
    // exists before you call this function.
    Result Write(const disk::BlockID &block_id, const disk::Block &block,
                 const dblog::LogSequenceNumber lsn);

    // Flush the block of `block_id` to disk.
    Result Flush(const disk::BlockID &block_id);

  private:
    // Find the buffer with the `block_id` and return a pointer to the
    // buffer, if there is no buffer with the `block_id`, returns ErrorValue.
    // The returned pointer is a mutable reference to the buffer in
    // `buffer_pool_` (NOTE: shared_ptr doesn't work in this case; you cannot
    // mutate the buffer in `buffer_pool_` with shared_ptr.)
    ResultV<Buffer *> FindBufferWithBlockID(const disk::BlockID &block_id);

    // Flush the buffer to the disk. This method flushes the log file first and
    // then writes the block to the disk, to make sure that the corresponding
    // log is written to disk.
    Result FlushBuffer(const Buffer &buffer);

    // Add new buffer to the buffer pool. When the buffer poll is full, select a
    // evicted buffer and swap the content.
    Result AddNewBuffer(const Buffer &buffer);

    // Selects a buffer to evict in the buffer pool. This method should be
    // implemented in the derived class.
    virtual ResultV<int> SelectEvictBufferID() = 0;

  protected:
    disk::DiskManager &disk_manager_;
    dblog::LogManager &log_manager_;
    std::vector<Buffer> buffer_pool_;
};

// SimpleBufferManager is a simple implementation of BufferManager.
// This class does not implement any eviction policy (just flush the first
// block).
class SimpleBufferManager : public BufferManager {
  public:
    SimpleBufferManager(const int buffer_size, disk::DiskManager &disk_manager,
                        dblog::LogManager &log_manager);

    const std::vector<Buffer> &BufferPool() const;

  private:
    ResultV<int> SelectEvictBufferID();
};

} // namespace buffer

#endif // BUFFER_H