#include "transaction.h"

namespace transaction {

dblog::TransactionID NextTransactionID() {
    static dblog::TransactionID transaction_id = 0;
    return transaction_id++;
}

Transaction::Transaction(buffer::BufferManager &buffer_manager,
                         dblog::LogManager log_manager,
                         dbconcurrency::LockTable &lock_table)
    : transaction_id_(NextTransactionID()), buffer_manager_(buffer_manager),
      concurrent_manager_(dbconcurrency::ConcurrentManager(lock_table)),
      recovery_manager_(recovery::RecoveryManager(std::move(log_manager))) {}

ResultV<std::unique_ptr<data::DataItem>>
Transaction::Read(const disk::DiskPosition &position,
                  const data::DataType &type) {
    disk::Block block;
    Result result = buffer_manager_.Read(position.BlockID(), block);
    if (result.IsError()) { return result + Error(""); }

    ResultV<std::unique_ptr<data::DataItem>> data_item =
        block.Read(position.Offset(), type);
    if (data_item.IsError()) { return data_item + Error(""); }

    return data_item;
}

Result Transaction::Write(const disk::DiskPosition &position,
                          const data::DataItem &data) {
    disk::Block block;
    Result result = buffer_manager_.Read(position.BlockID(), block);
    if (result.IsError()) { return result + Error(""); }

    Result write_result = block.Write(position.Offset(), data);
    if (write_result.IsError()) { return write_result + Error(""); }

    return buffer_manager_.Write(position.BlockID(), block);
}

Result Transaction::Commit() { return Ok(); }

Result Transaction::Rollback() { return Ok(); }

ResultV<int> Transaction::Size(const std::string &filename) const {
    return buffer_manager_.Size(filename);
}

Result Transaction::AllocateNewBlocks(const disk::BlockID &block_id) const {
    return buffer_manager_.AllocateNewBlocks(block_id);
}

} // namespace transaction