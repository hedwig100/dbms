#include "disk.h"

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

/** Block */
Block::Block() {}

Block::Block(const int block_size, const char *content) {
    content_.resize(block_size);
    std::copy(content, content + block_size, content_.begin());
}

ResultV<uint8_t> Block::ReadByte(const int offset) const {
    if (offset >= content_.size())
        return Error("offset should be smaller than content size.");
    return Ok(content_[offset]);
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
                                     block_id.BlockIndex() * block_size_);
        return Ok();
    } catch (std::filesystem::filesystem_error) {
        return Error("failed to allocate new blocks");
    }
}

} // namespace disk