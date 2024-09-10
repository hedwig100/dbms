#include "disk.h"
#include <iostream>
#include <string>

void Print(const disk::Block &block) {
    auto content = block.Content();
    std::cout << "BlockContent: '";
    for (const uint8_t &c : content) {
        std::cout << c << ' ';
    }
    std::cout << "'\n";
}

int main() {
    const std::string filename = "filename.txt";
    const size_t block_size    = 32;
    disk::DiskManager manager(/*directory_path=*/"directory/",
                              /*block_size=*/block_size);
    if (manager.AllocateNewBlocks(disk::BlockID(filename, 64)).IsError()) {
        std::cout << "Failed to allocate blocks\n";
        return 1;
    }

    disk::BlockID block_id(filename, 1);
    disk::Block block(block_size, "abcdefghijklmnopqrstuvwxyz");
    block.WriteByte(/*offset=*/6, '\0');
    if (manager.Write(block_id, block).IsError()) {
        std::cout << "Failed to write a block\n";
        return 1;
    }
    Print(block);

    if (manager.Read(block_id, block).IsError()) {
        std::cout << "Failed to read a block\n";
        return 1;
    }
    Print(block);
}