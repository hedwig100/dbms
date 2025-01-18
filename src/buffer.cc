#include "buffer.h"

namespace buffer {

Buffer::Buffer() : block_id_(disk::BlockID("", 0)), block_(disk::Block()) {}

Buffer::Buffer(const disk::BlockID &block_id, const disk::Block &block)
    : block_id_(block_id), block_(block) {}

const disk::BlockID &Buffer::BlockID() const { return block_id_; }

const disk::Block &Buffer::Block() const { return block_; }

void Buffer::SetBlock(const disk::Block &block,
                      const dblog::LogSequenceNumber lsn) {
    block_ = block;
    if (lsn > latest_lsn_) latest_lsn_ = lsn;
}

BufferManager::BufferManager(disk::DiskManager &disk_manager,
                             dblog::LogManager &log_manager)
    : disk_manager_(disk_manager), log_manager_(log_manager) {}

Result BufferManager::Read(const disk::BlockID &block_id, disk::Block &block) {
    auto buffer_result = FindBufferWithBlockID(block_id);
    if (buffer_result.IsOk()) {
        block = buffer_result.Get()->Block();
        return Ok();
    }
    auto result = disk_manager_.Read(block_id, block);
    if (result.IsError()) {
        return result + Error("buffer::BufferManager::Read() fail to read.");
    }
    return AddNewBuffer(Buffer(block_id, block));
}

Result BufferManager::Write(const disk::BlockID &block_id,
                            const disk::Block &block,
                            const dblog::LogSequenceNumber lsn) {
    auto buffer_result = FindBufferWithBlockID(block_id);
    if (buffer_result.IsOk()) {
        buffer_result.Get()->SetBlock(block, lsn);
        return Ok();
    }
    return AddNewBuffer(Buffer(block_id, block));
}

Result BufferManager::Flush(const disk::BlockID &block_id) {
    auto buffer_result = FindBufferWithBlockID(block_id);
    if (buffer_result.IsOk()) { return FlushBuffer(*buffer_result.Get()); }
    return disk_manager_.Flush(block_id.Filename());
}

ResultV<Buffer *>
BufferManager::FindBufferWithBlockID(const disk::BlockID &block_id) {
    for (auto &buffer : buffer_pool_) {
        if (buffer.BlockID() == block_id) { return Ok(&buffer); }
    }
    return Error("buffer::BufferManager::FindBufferWithBlockID() no buffer "
                 "with the block_id.");
}

Result BufferManager::FlushBuffer(const Buffer &buffer) {
    // To make sure that the corresponding log is written to disk,
    // flush the log file first and then write the block to disk.
    auto log_result = log_manager_.Flush(buffer.LatestLogSequenceNumber());
    if (log_result.IsError())
        return log_result +
               Error("buffer::BufferManager::Flush() failed to flush log.");

    auto write_result = disk_manager_.Write(buffer.BlockID(), buffer.Block());
    if (write_result.IsError())
        return write_result +
               Error("buffer::BufferManager::Flush() failed to write.");

    return Ok();
}

Result BufferManager::AddNewBuffer(const Buffer &buffer) {
    ResultV<int> evicted_buffer_id = SelectEvictBufferID();
    if (evicted_buffer_id.IsError()) {
        return evicted_buffer_id +
               Error("buffer::BufferManager::AddNewBuffer() "
                     "failed to select evict buffer.");
    }

    const Buffer &evicted_buffer = buffer_pool_[evicted_buffer_id.Get()];
    if (evicted_buffer.IsDirty()) {
        Result flush_result = FlushBuffer(evicted_buffer);
        if (flush_result.IsError()) {
            return flush_result + Error("buffer::BufferManager::AddNewBuffer() "
                                        "failed to flush buffer.");
        }
    }

    buffer_pool_[evicted_buffer_id.Get()] = buffer;
    return Ok();
}

SimpleBufferManager::SimpleBufferManager(const int buffer_size,
                                         disk::DiskManager &disk_manager,
                                         dblog::LogManager &log_manager)
    : BufferManager(disk_manager, log_manager) {
    buffer_pool_.resize(buffer_size);
}

const std::vector<Buffer> &SimpleBufferManager::BufferPool() const {
    return buffer_pool_;
}

ResultV<int> SimpleBufferManager::SelectEvictBufferID() { return Ok(0); }

} // namespace buffer