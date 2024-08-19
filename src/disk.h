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
    inline BlockID() : filename_(""), block_index_(0) {};
    BlockID(const std::string &filename, const int block_index);

    // Returns the filename.
    const std::string &Filename() const;

    // Returns the block index.
    const int BlockIndex() const;

    bool operator==(const BlockID &other_block) const;

  private:
    std::string filename_;
    int block_index_;
};

// The unit of data exchanged with disk.
class Block {
  public:
    Block();

    // Make an empty block with size of `block_size`.
    explicit Block(const int block_size);

    // Initialize a block with the `content`.
    Block(const int block_size, const char *content);

    inline int Size() const { return size_; }
    inline void SetSize(int size) { size_ = size; }

    // Reads the byte with the `offset`.
    ResultV<uint8_t> ReadByte(const int offset) const;

    // Reads int a 4-byte integer) with the `offset`
    ResultV<int> ReadInt(const int offset) const;

    // Returns the content of the block.
    const std::vector<uint8_t> &Content() const;

    // Appends `bytes` to the tail of this block. If the input bytes sequece
    // does not fit to the block, returns an error. The error contains the
    // length of bytes sequence appended to the block.
    ResultE<size_t>
    WriteAppend(const std::vector<uint8_t>::iterator &bytes_begin,
                const std::vector<uint8_t>::iterator &bytes_end);

  private:
    // The size of the content of the block in bytes (`block_size` is the
    // maximum size of the block).
    int size_;

    std::vector<uint8_t> content_;
};

// Manages writes and reads to disk.
class DiskManager {
  public:
    // Initiate a disk manager, the directory of `directory_path` should exist
    // when this disk manager is initiated.
    DiskManager(const std::string &directory_path, const int block_size);

    inline const std::string &DirectoryPath() const { return directory_path_; }

    inline int BlockSize() const { return block_size_; }

    // Reads the bytes of `block_id` into `block`.
    Result Read(const BlockID &block_id, Block &block) const;

    // Writes the bytes `block` to the place of `block_id`.
    Result Write(const BlockID &block_id, const Block &block) const;

    // Flushes the writes of `directory_path`/`filename` to the disk.
    Result Flush(const std::string &filename) const;

    // The number of blocks in the file of `filename`.
    ResultV<size_t> Size(const std::string &filename) const;

    // Allocates new blocks until the id of `block_id`.
    // If file of `block_id.Filename()` does not exist, this function creates a
    // new file and resize it to the `block_id.BlockIndex()`. If file of
    // `block_id.Filename()` exists, resize it.
    Result AllocateNewBlocks(const BlockID &block_id) const;

  private:
    const std::string directory_path_;
    const int block_size_;
};

} // namespace disk

#endif // DISK_H