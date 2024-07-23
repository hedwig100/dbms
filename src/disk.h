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
    Block(const int block_size, const char *content);

    // Reads the byte with the `offset`.
    ResultV<uint8_t> ReadByte(const int offset) const;

    // Returns the content of the block.
    const std::vector<uint8_t> &Content() const;

  private:
    std::vector<uint8_t> content_;
};

// Manages writes and reads to disk.
class DiskManager {
  public:
    DiskManager(const std::string &directory_path, const int block_size);

    // Reads the bytes of `block_id` into `block`.
    Result Read(const BlockID &block_id, Block &block) const;

    // Writes the bytes `block` to the place of `block_id`.
    Result Write(const BlockID &block_id, const Block &block) const;

    // Flushes the writes of `directory_path`/`filename` to the disk.
    Result Flush(const std::string &filename) const;

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