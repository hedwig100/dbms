#ifndef BUFFER_H
#define BUFFER_H

#include "disk.h"
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
    disk::Block Block() const;

    // Set the owned block.
    void SetBlock(const disk::Block &block);

  private:
    disk::BlockID block_id_;
    disk::Block block_;
};

class BufferManager {
  public:
    BufferManager(const int buffer_size, const disk::DiskManager &disk_manager);

    // Reads the block of `block_id` from the buffer pool. The block with
    // `block_id` is cached in `buffer_pool_`.
    Result Read(const disk::BlockID &block_id, disk::Block &block);

    // Writes the block of `block_id` to the buffer pool. The block with
    // `block_id` is cached in `buffer_pool_`. When execution of this method
    // succeeds, it does not mean the block is written to disk. You have to call
    // `::Flush()` to make sure that the block is written to disk.
    // NOTE: Does't raise an error even if there is no block with `block_id`.
    // Thus, you should make sure that the block with `block_id` exists before
    // you call this function.
    Result Write(const disk::BlockID &block_id, const disk::Block &block);

    // Flush the block of `block_id` to disk.
    Result Flush(const disk::BlockID &block_id);

    // Returns the content of buffer pool.
    std::vector<Buffer> BufferPool() const;

    // TODO: implement the follwing method when necesary.
    // void Pin(const disk::BlockID &block_id) const;

  private:
    // Find the buffer with the `block_id` and return a pointer to the
    // buffer, if there is no buffer with the `block_id`, returns ErrorValue.
    // The returned pointer is a mutable reference to the buffer in
    // `buffer_pool_` (NOTE: shared_ptr doesn't work in this case; you cannot
    // mutate the buffer in `buffer_pool_` with shared_ptr.)
    ResultV<Buffer *> FindBufferWithBlockID(const disk::BlockID &block_id);

    // Add new buffer to the buffer pool. When the buffer poll is full, select a
    // evicted buffer and swap the content.
    Result AddNewBuffer(const Buffer &buffer);

    disk::DiskManager disk_manager_;
    std::vector<Buffer> buffer_pool_;
};

} // namespace buffer

#endif // BUFFER_H