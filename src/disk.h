#ifndef DISK_H
#define DISK_H

#include <cstdint>
#include <string>
#include <vector>

#include "result.h"

using namespace ::result;

namespace disk {

// ID of the block. This is is represented as a pair of `filename` and
// `block_index`. The `block_index` represents the 0-indexed index of the
// block in the file with the block size.
class BlockID {
  public:
    inline BlockID() {}
    BlockID(const std::string &filename, const int block_index);

    // Returns the filename.
    const std::string &Filename() const;

    // Returns the block index.
    const int BlockIndex() const;

    BlockID operator+(int block_index_to_advance) const;

    BlockID &operator+=(int block_index_to_advance);

    BlockID operator-(int block_index_to_back) const;

    BlockID &operator-=(int block_index_to_back);

    bool operator==(const BlockID &other_block) const;

    bool operator!=(const BlockID &other_block) const;

  private:
    std::string filename_;
    int block_index_;
};

// Specify position in the disk by BlockID and offset in the block.
class DiskPosition {
  public:
    // `offset` must be non-negative integer smaller than block size.
    DiskPosition(const BlockID &block_id, const int offset)
        : block_id_(block_id), offset_(offset) {}

    // Block id of the position
    inline const disk::BlockID BlockID() const { return block_id_; }

    // Offset of the position in the block
    inline int Offset() const { return offset_; }

    // Move this position with `displacement`. `displacement` can be negative
    // integer.
    DiskPosition Move(const int displacement, const int block_size) const;

  private:
    disk::BlockID block_id_;
    int offset_;
};

// The unit of data exchanged with disk.
class Block {
  public:
    inline Block() {}
    inline explicit Block(const int block_size) {
        content_.assign(block_size, 0);
    }

    // Initialize the block with `block_size` and `content`.
    // If `content` is smaller than `block_size`, the trailing bytes are empty.
    Block(const int block_size, const char *content);

    // Initialize the block with `block_size` and `content`.
    // If `content` is smaller than `block_size`, the trailing bytes are empty.
    Block(const int block_size, const std::vector<uint8_t> &content);

    // Blocksize of this block
    inline size_t BlockSize() const { return content_.size(); }

    // Reads the byte with the `offset`.
    ResultV<uint8_t> ReadByte(const int offset) const;

    // Writes the byte `value` with the `offset`.
    Result WriteByte(const int offset, const uint8_t value);

    // Reads the bytes of `length` with the `offset` to `bytes`.
    Result ReadBytes(const int offset, const size_t length,
                     std::vector<uint8_t> &bytes) const;

    // Writes the bytes `value` with the `offset`. If the `value` is shorter
    // than `length`, only writes the first `length` bytes.
    Result WriteBytes(const int offset, const size_t length,
                      const std::vector<uint8_t> &value);

    // Writes `value`[`value_offset`:] to the block with `offset`. If
    // `value`[`value_offset`:] does not fit the block, writes the bytes to the
    // block and returns the offset of the bytes left with ResultE.
    ResultE<size_t> WriteBytesWithOffset(const size_t offset,
                                         const std::vector<uint8_t> &value,
                                         const size_t value_offset);

    // Reads the int with the `offset`. The value is read as little-endian.
    ResultV<int> ReadInt(const int offset) const;

    // Writes the int `value` with the `offset`. The value is written as
    // little-endian.
    Result WriteInt(const int offset, const int value);

    // Reads the string of length `length` with the `offset`.
    ResultV<std::string> ReadString(const int offset, const int length) const;

    // Writes the string `value` with the `offset`.
    Result WriteString(const int offset, const std::string &value);

    // Returns the content of the block.
    const std::vector<uint8_t> &Content() const;

  private:
    std::vector<uint8_t> content_;
};

// Manages writes and reads to disk.
class DiskManager {
  public:
    // Initiate a disk manager, the directory of `directory_path` should exist
    // when this disk manager is initiated.
    DiskManager(const std::string &directory_path, const int block_size);

    // Returns a directory path which this instance manages.
    inline const std::string &DirectoryPath() const { return directory_path_; }

    // Returns the block size of this manager.
    inline int BlockSize() const { return block_size_; }

    // Reads the bytes of `block_id` into `block`. `block.BlockSize()` and
    // `this.BlockSize()` must be the same to run this function without any
    // unintentional behavior.
    Result Read(const BlockID &block_id, Block &block) const;

    // Writes the bytes `block` to the place of `block_id`. `block.BlockSize()`
    // and `this.BlockSize()` must be the same to run this function without any
    // unintentional behavior.
    Result Write(const BlockID &block_id, const Block &block) const;

    // Flushes the writes of `directory_path`/`filename` to the disk.
    Result Flush(const std::string &filename) const;

    // The number of blocks in the file of `filename`.
    ResultV<size_t> Size(const std::string &filename) const;

    // Allocates new blocks until the id of `block_id` (including the end).
    // If file of `block_id.Filename()` does not exist, this function creates a
    // new file and resize it to the `block_id.BlockIndex()`. If file of
    // `block_id.Filename()` exists, resize it.
    Result AllocateNewBlocks(const BlockID &block_id) const;

    // Read bytes which can lie across multiple blocks. `position` specifies the
    // start position to read the bytes. `block` is the block in which
    // `position` is located. `length` is the length of the bytes to read.
    // The bytes are written to `bytes`.
    Result ReadBytesAcrossBlocks(const DiskPosition &position,
                                 const disk::Block &block, int length,
                                 std::vector<uint8_t> &bytes) const;

    // Read int which can lie across multiple blocks. `position` specifies the
    // start position to read the bytes. `block` is the block in which
    // `position` is located.
    ResultV<int> ReadIntAcrossBlocks(const DiskPosition &position,
                                     const disk::Block &block) const;

    // Read uint32_t which can lie across multiple blocks. `position` specifies
    // the start position to read the bytes. `block` is the block in which
    // `position` is located.
    ResultV<uint32_t> ReadUint32AcrossBlocks(const DiskPosition &position,
                                             const disk::Block &block) const;

    // Read bytes which can lie across multiple blocks.
    // `position`.Move(`offset`) is the start position to read the bytes. The
    // `offset` can be negative value. `block` is the block in which `position`
    // is located. `length` is the length of the bytes to read. The bytes are
    // written to `bytes`.
    Result ReadBytesAcrossBlocksWithOffset(const DiskPosition &position,
                                           const int offset,
                                           const disk::Block &block, int length,
                                           std::vector<uint8_t> &bytes) const;

    // Read int which can lie across multiple blocks.
    // `position`.Move(`offset`) is the start position to read the bytes. The
    // `offset` can be negative value. `block` is the block in which `position`
    // is located.
    ResultV<int> ReadIntAcrossBlocksWithOffset(const DiskPosition &position,
                                               const int offset,
                                               const disk::Block &block) const;

    // Read iint32_t which can lie across multiple blocks.
    // `position`.Move(`offset`) is the start position to read the bytes. The
    // `offset` can be negative value. `block` is the block in which `position`
    // is located.
    ResultV<uint32_t>
    ReadUint32AcrossBlocksWithOffset(const DiskPosition &position,
                                     const int offset,
                                     const disk::Block &block) const;

  private:
    const std::string directory_path_;
    const int block_size_;
};

} // namespace disk

#endif // DISK_H