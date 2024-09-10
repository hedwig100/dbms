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

bool BlockID::operator==(const BlockID &other_block) const {
    return (filename_ == other_block.filename_) &&
           (block_index_ == other_block.block_index_);
}

/** Block */
Block::Block(const int block_size, const char *content) {
    content_.resize(block_size);
    const int copiable_size = std::min(block_size, int(strlen(content)));
    std::copy(content, content + copiable_size, content_.begin());
}

ResultV<uint8_t> Block::ReadByte(const int offset) const {
    if (offset < 0 || offset >= content_.size())
        return Error("offset should be fit the size");
    return Ok(content_[offset]);
}

Result Block::WriteByte(const int offset, const uint8_t value) {
    if (offset < 0 || offset >= content_.size())
        return Error("offset should be fit the size");
    content_[offset] = value;
    return Ok();
}

Result Block::ReadBytes(const int offset, const size_t length,
                        std::vector<uint8_t> &bytes) const {
    if (offset < 0 || offset + length > content_.size())
        return Error("offset should be fit the size");
    bytes.resize(length);
    std::copy(content_.begin() + offset, content_.begin() + offset + length,
              bytes.begin());
    return Ok();
}

Result Block::WriteBytes(const int offset, const size_t length,
                         const std::vector<uint8_t> &value) {
    if (offset < 0 || offset + length > content_.size())
        return Error("offset should be fit the size");
    if (value.size() < length)
        return Error("value should be longer than length");
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
    if (!file.is_open()) return Error("failed to open a file");

    file.seekg(block_id.BlockIndex() * block_size_, std::ios::beg);
    if (file.fail()) {
        file.close();
        return Error("failed to seek a file");
    }

    char block_content[block_size_];
    file.read(block_content, block_size_);
    if (file.fail()) {
        file.close();
        return Error("failed to read a file");
    }

    block = Block(block_size_, block_content);
    file.close();
    return Ok();
}

Result DiskManager::Write(const BlockID &block_id, const Block &block) const {
    std::ofstream file(directory_path_ + block_id.Filename(), std::ios::binary);
    if (!file.is_open()) return Error("failed to open a file");

    file.seekp(block_id.BlockIndex() * block_size_, std::ios::beg);
    if (file.fail()) {
        file.close();
        return Error("failed to seek a file");
    }

    const auto &content_vector = block.Content();
    char content_char[block_size_];
    std::copy(content_vector.begin(), content_vector.end(), content_char);
    file.write(content_char, block_size_);
    if (file.fail()) {
        file.close();
        return Error("failed to write to a file");
    }

    file.close();
    return Ok();
}

Result DiskManager::Flush(const std::string &filename) const {
    int fd = open((directory_path_ + filename).c_str(), O_FSYNC);
    if (fd < 0) return Error("failed to open a file");

    if (fsync(fd) < 0) {
        close(fd);
        return Error("failed to fsync");
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
        return Error("failed to compute file size");
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
        if (!file.is_open()) return Error("failed to create a new file");
        file.close();
    }

    try {
        std::filesystem::resize_file(filepath,
                                     (block_id.BlockIndex() + 1) * block_size_);
        return Ok();
    } catch (std::filesystem::filesystem_error) {
        return Error("failed to allocate new blocks");
    }
}

} // namespace disk