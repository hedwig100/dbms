#include "buffer.h"

namespace buffer {

/** Buffer */

Buffer::Buffer() : block_id_(disk::BlockID("", 0)), block_(disk::Block()) {}

Buffer::Buffer(const disk::BlockID &block_id, const disk::Block &block)
    : block_id_(block_id), block_(block) {}

const disk::BlockID &Buffer::BlockID() const { return block_id_; }

disk::Block Buffer::Block() const { return block_; }

void Buffer::SetBlock(const disk::Block &block) { block_ = block; }

/** BufferManager */

BufferManager::BufferManager(const int buffer_size,
                             const disk::DiskManager &disk_manager)
    : disk_manager_(disk_manager) {
    buffer_pool_.resize(buffer_size);
}

Result BufferManager::Read(const disk::BlockID &block_id, disk::Block &block) {
    auto buffer_result = FindBufferWithBlockID(block_id);
    if (buffer_result.IsOk()) {
        block = buffer_result.Get()->Block();
        return Ok();
    }
    auto result = disk_manager_.Read(block_id, block);
    if (result.IsError()) { return result; }
    return AddNewBuffer(Buffer(block_id, block));
}

Result BufferManager::Write(const disk::BlockID &block_id,
                            const disk::Block &block) {
    auto buffer_result = FindBufferWithBlockID(block_id);
    if (buffer_result.IsOk()) {
        buffer_result.Get()->SetBlock(block);
        return Ok();
    }
    return AddNewBuffer(Buffer(block_id, block));
}

Result BufferManager::Flush(const disk::BlockID &block_id) {
    auto buffer_result = FindBufferWithBlockID(block_id);
    if (buffer_result.IsOk()) {
        auto write_result =
            disk_manager_.Write(block_id, buffer_result.Get()->Block());
        if (write_result.IsError()) return write_result;
    }
    return disk_manager_.Flush(block_id.Filename());
}

std::vector<Buffer> BufferManager::BufferPool() const { return buffer_pool_; }

ResultV<Buffer *>
BufferManager::FindBufferWithBlockID(const disk::BlockID &block_id) {
    for (auto &buffer : buffer_pool_) {
        if (buffer.BlockID() == block_id) { return Ok(&buffer); }
    }
    return Error("no buffer with the block_id");
}

Result BufferManager::AddNewBuffer(const Buffer &buffer) {
    // (TODO) implement decent evicting logic like LRU.
    Result result = Ok();

    // NOTE: If BlockID().Filename() is empty, it means the buffer is empty.
    if (buffer_pool_[0].BlockID().Filename() != "")
        result = disk_manager_.Write(buffer_pool_[0].BlockID(),
                                     buffer_pool_[0].Block());
    buffer_pool_[0] = buffer;
    return result;
}

} // namespace buffer