#include "disk.h"

#include "data/char.h"
#include "data/int.h"
#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <vector>

namespace disk {

BlockID::BlockID(const std::string &filename, const int block_index)
    : filename_(filename), block_index_(block_index) {}

const std::string &BlockID::Filename() const { return filename_; }

const int BlockID::BlockIndex() const { return block_index_; }

BlockID BlockID::operator+(int block_index_to_advance) const {
    return BlockID(filename_, block_index_ + block_index_to_advance);
}

BlockID &BlockID::operator+=(int block_index_to_advance) {
    block_index_ += block_index_to_advance;
    return *this;
}

BlockID BlockID::operator-(int block_index_to_back) const {
    return BlockID(filename_, block_index_ - block_index_to_back);
}

BlockID &BlockID::operator-=(int block_index_to_back) {
    block_index_ -= block_index_to_back;
    return *this;
}

bool BlockID::operator==(const BlockID &other_block) const {
    return (filename_ == other_block.filename_) &&
           (block_index_ == other_block.block_index_);
}

bool BlockID::operator!=(const BlockID &other_block) const {
    return !(*this == other_block);
}

DiskPosition DiskPosition::Move(const int displacement,
                                const int block_size) const {
    int block_index_displacement = (offset_ + displacement) / block_size;
    int new_offset               = (offset_ + displacement) % block_size;
    if (new_offset < 0) {
        new_offset += block_size;
        block_index_displacement -= 1;
    }
    return DiskPosition(block_id_ + block_index_displacement, new_offset);
}

/** Block */
Block::Block(const int block_size, const char *content) {
    content_.resize(block_size);
    const int copiable_size = std::min(block_size, int(strlen(content)));
    std::copy(content, content + copiable_size, content_.begin());
}

Block::Block(const int block_size, const std::vector<uint8_t> &content) {
    content_.resize(block_size);
    const int copiable_size = std::min(block_size, int(content.size()));
    std::copy(content.begin(), content.begin() + copiable_size,
              content_.begin());
}

ResultV<uint8_t> Block::ReadByte(const int offset) const {
    if (offset < 0 || offset >= content_.size())
        return Error("disk::Block::ReadByte() offset should be fit the size.");
    return Ok(content_[offset]);
}

Result Block::WriteByte(const int offset, const uint8_t value) {
    if (offset < 0 || offset >= content_.size())
        return Error("disk::Block::WriteByte() offset should be fit the size.");
    content_[offset] = value;
    return Ok();
}

Result Block::ReadBytes(const int offset, const size_t length,
                        std::vector<uint8_t> &bytes) const {
    if (offset < 0 || offset + length > content_.size())
        return Error("disk::Block::ReadBytes() offset should be fit the size.");
    bytes.resize(length);
    std::copy(content_.begin() + offset, content_.begin() + offset + length,
              bytes.begin());
    return Ok();
}

Result Block::WriteBytes(const int offset, const size_t length,
                         const std::vector<uint8_t> &value) {
    if (offset < 0 || offset + length > content_.size())
        return Error(
            "disk::Block::WriteBytes() offset should be fit the size.");
    if (value.size() < length)
        return Error("disk::Block::WriteBytes() value should be longer "
                     "than length.");
    std::copy(value.begin(), value.begin() + length, content_.begin() + offset);
    return Ok();
}

ResultE<size_t> Block::WriteBytesWithOffset(const size_t offset,
                                            const std::vector<uint8_t> &value,
                                            const size_t value_offset) {
    if (offset + value.size() - value_offset <= content_.size()) {
        std::copy(value.begin() + value_offset, value.end(),
                  content_.begin() + offset);
        return Ok();
    }
    const size_t length = content_.size() - offset;
    std::copy(value.begin() + value_offset,
              value.begin() + value_offset + length, content_.begin() + offset);
    return Error(value_offset + length);
}

ResultV<int> Block::ReadInt(const int offset) const {
    return data::ReadInt(content_, offset);
}

Result Block::WriteInt(const int offset, const int value) {
    return data::WriteInt(content_, offset, value);
}

ResultV<std::string> Block::ReadString(const int offset,
                                       const int length) const {
    return data::ReadString(content_, offset, length);
}

Result Block::WriteString(const int offset, const std::string &value) {
    return data::WriteString(content_, offset, value.size(), value);
}

const std::vector<uint8_t> &Block::Content() const { return content_; }

/** DiskManager */

DiskManager::DiskManager(const std::string &directory_path,
                         const int block_size)
    : directory_path_(directory_path), block_size_(block_size) {}

Result DiskManager::Read(const BlockID &block_id, Block &block) const {
    std::ifstream file(directory_path_ + block_id.Filename(), std::ios::binary);
    if (!file.is_open())
        return Error("disk::DiskManager::Read() failed to open a file.");

    file.seekg(block_id.BlockIndex() * block_size_, std::ios::beg);
    if (file.fail()) {
        file.close();
        return Error("disk::DiskManager::Read() failed to seek a file.");
    }

    std::vector<uint8_t> block_content(block_size_);
    file.read((char *)&block_content[0], block_size_);
    if (file.fail()) {
        file.close();
        return Error("disk::DiskManager::Read() failed to read a file.");
    }

    block = Block(block_size_, block_content);
    file.close();
    return Ok();
}

Result DiskManager::Write(const BlockID &block_id, const Block &block) const {
    std::ofstream file(directory_path_ + block_id.Filename(), std::ios::binary);
    if (!file.is_open())
        return Error("disk::DiskManager::Write() failed to open a file.");

    file.seekp(block_id.BlockIndex() * block_size_, std::ios::beg);
    if (file.fail()) {
        file.close();
        return Error("disk::DiskManager::Write() failed to seek a file.");
    }

    const auto &content_vector = block.Content();
    file.write((char *)&content_vector[0], block_size_);
    if (file.fail()) {
        file.close();
        return Error("disk::DiskManager::Write() failed to write to a file.");
    }

    file.close();
    return Ok();
}

Result DiskManager::Flush(const std::string &filename) const {
    int fd = open((directory_path_ + filename).c_str(), O_FSYNC);
    if (fd < 0)
        return Error("disk::DiskManager::Flush() failed to open a file.");

    if (fsync(fd) < 0) {
        close(fd);
        return Error("disk::DiskManager::Flush() failed to fsync.");
    }

    close(fd);
    return Ok();
}

ResultV<size_t> DiskManager::Size(const std::string &filename) const {
    try {
        std::uintmax_t filesize =
            std::filesystem::file_size(directory_path_ + filename);
        return Ok((filesize / block_size_));
    } catch (std::filesystem::filesystem_error) {
        return Error("disk::DiskManager::Size() failed to compute file size.");
    }
}

Result DiskManager::AllocateNewBlocks(const BlockID &block_id) const {
    const auto filepath = directory_path_ + block_id.Filename();
    if (!std::filesystem::exists(filepath)) {
        if (!std::filesystem::exists(directory_path_)) {
            std::filesystem::create_directories(directory_path_);
        }

        // Uses std::ofstream to create the file.
        std::ofstream file(filepath);
        if (!file.is_open())
            return Error("disk::DiskManager::AllocatedNewBlocks() failed to "
                         "create a new file.");
        file.close();
    }

    try {
        std::filesystem::resize_file(filepath,
                                     (block_id.BlockIndex() + 1) * block_size_);
        return Ok();
    } catch (std::filesystem::filesystem_error) {
        return Error("disk::DiskManager::AllocatedNewBlocks() failed to "
                     "allocate new blocks.");
    }
}

// Moves the `block` to the next block of `block_id`.
Result MoveToNextBlock(disk::BlockID &block_id, disk::Block &block,
                       const disk::DiskManager &disk_manager) {
    disk::BlockID next_block_id(block_id.Filename(), block_id.BlockIndex() + 1);
    Result read_result = disk_manager.Read(next_block_id, block);
    if (read_result.IsError())
        return read_result +
               Error("disk:MoveToNextBlock() failed to read the next block.");
    block_id = next_block_id;
    return Ok();
}

Result ReadBytesAcrossBlocks(disk::BlockID &block_id, int &offset,
                             disk::Block &block, int length,
                             std::vector<uint8_t> &read_bytes,
                             const disk::DiskManager &disk_manager) {
    if (offset < 0 || offset >= disk_manager.BlockSize())
        return Error(
            "disk::ReadBytesAcrossBlocks() offset should be in the block.");

    int length_of_first_block = (offset + length <= disk_manager.BlockSize()
                                     ? length
                                     : disk_manager.BlockSize() - offset);
    Result read_result =
        block.ReadBytes(offset, length_of_first_block, read_bytes);
    if (read_result.IsError())
        return read_result + Error("disk::ReadBytesAcrossBlocks() failed to "
                                   "read the first block.");
    length -= length_of_first_block;

    // We expect the length is not large enouth to lie across block in many
    // cases. Therefore, retrun here to avoid unnecessary allocation in heap
    // memory.
    if (length == 0) return Ok();

    const disk::BlockID rollback_block_id = block_id;
    const disk::Block rollback_block      = block;

    while (length > 0) {
        Result move_result = MoveToNextBlock(block_id, block, disk_manager);
        if (move_result.IsError()) {
            block_id = rollback_block_id;
            block    = rollback_block;
            return move_result + Error("disk::ReadBytesAcrossBlocks() failed "
                                       "to move to the next block.");
        }

        if (length >= disk_manager.BlockSize()) {
            std::copy(block.Content().begin(), block.Content().end(),
                      std::back_inserter(read_bytes));
            length -= disk_manager.BlockSize();
        } else {
            std::vector<uint8_t> tmp_bytes(length);
            block.ReadBytes(0, length, tmp_bytes);
            std::copy(tmp_bytes.begin(), tmp_bytes.end(),
                      std::back_inserter(read_bytes));
            length = 0;
        }
    }

    return Ok();
}

} // namespace disk