#include "table_scan.h"
#include "data/byte.h"
#include "data/char.h"
#include "data/int.h"
#include "disk.h"
#include "result.h"
#include "schema.h"
#include "transaction.h"

namespace scan {

std::string TableFileName(const std::string &table_name) {
    return table_name + ".table";
}

constexpr uint8_t kUnusedFlag = 0x00;
constexpr uint8_t kUsedFlag   = 0x01;

TableScan::TableScan(transaction::Transaction &transaction,
                     std::string table_name, schema::Layout layout)
    : transaction_(transaction), table_name_(table_name), layout_(layout) {}

Result TableScan::Init() {
    SetBlockNumber(0);
    slot_ = 0;

    ResultV<size_t> size = transaction_.Size(TableFileName(table_name_));
    if (size.IsError()) {
        return size +
               Error("TableScan::Init() failed to get the size of the file");
    }
    if (size.Get() == 0) {
        Result result = CreateFirstBlock();
        if (result.IsError())
            return result +
                   Error("TableScan::Init() failed to create the first block.");
        return Ok();
    }

    ResultV<bool> is_used = IsUsed();
    if (is_used.IsError()) {
        return is_used +
               Error("TableScan::Init() failed to check if the slot is used");
    }
    if (is_used.Get()) { return Ok(); }

    ResultV<bool> next = Next();
    if (next.IsError()) {
        return next + Error("TableScan::Init() failed to get the next slot");
    }

    if (next.Get()) { return Ok(); }
    return Insert();
}

ResultV<bool> TableScan::Next() {
    while (true) {
        ResultV<bool> next = NextSlot();
        if (next.IsError()) {
            return next +
                   Error("TableScan::Next() failed to get the next slot");
        }

        if (!next.Get()) { return Ok(false); }

        ResultV<bool> is_used = IsUsed();
        if (is_used.IsError()) {
            return is_used +
                   Error(
                       "TableScan::Next() failed to check if the slot is used");
        }

        if (is_used.Get()) { return Ok(true); }
    }
}

ResultV<data::DataItemWithType> TableScan::Get(const std::string &fieldname) {
    TRY_VALUE(field_length, layout_.Length(fieldname));
    TRY_VALUE(field_type, layout_.Type(fieldname));
    TRY_VALUE(field_offset, layout_.Offset(fieldname));

    disk::DiskPosition position(/*block_id=*/block_id_,
                                /*offset=*/slot_ * layout_.Length() +
                                    field_offset.Get());
    data::DataItem item;
    FIRST_TRY(transaction_.Read(position, field_length.Get(), item));
    return Ok(
        data::DataItemWithType(item, field_type.Get(), field_length.Get()));
}

ResultV<int> TableScan::GetInt(const std::string &fieldname) {
    TRY_VALUE(field_length, layout_.Length(fieldname));
    TRY_VALUE(field_offset, layout_.Offset(fieldname));

    disk::DiskPosition position(/*block_id=*/block_id_,
                                /*offset=*/slot_ * layout_.Length() +
                                    field_offset.Get());
    data::DataItem item;
    FIRST_TRY(transaction_.Read(position, data::kTypeInt.ValueLength(), item));
    return Ok(data::ReadInt(item));
}

ResultV<std::string> TableScan::GetChar(const std::string &fieldname) {
    TRY_VALUE(field_length, layout_.Length(fieldname));
    TRY_VALUE(field_offset, layout_.Offset(fieldname));

    disk::DiskPosition position(/*block_id=*/block_id_,
                                /*offset=*/slot_ * layout_.Length() +
                                    field_offset.Get());
    data::DataItem item;
    FIRST_TRY(transaction_.Read(position, field_length.Get(), item));
    std::string value = data::ReadChar(item, field_length.Get());
    data::RightTrim(value);
    return Ok(value);
}

Result TableScan::Update(const std::string &fieldname,
                         const data::DataItem &item) {
    TRY_VALUE(field_length, layout_.Length(fieldname));
    TRY_VALUE(field_offset, layout_.Offset(fieldname));

    disk::DiskPosition position(/*block_id=*/block_id_,
                                /*offset=*/slot_ * layout_.Length() +
                                    field_offset.Get());
    FIRST_TRY(transaction_.Write(position, field_length.Get(), item));
    return Ok();
}

Result TableScan::Insert() {
    while (true) {
        ResultV<bool> is_used = IsUsed();
        if (is_used.IsError()) {
            return is_used + Error("TableScan::Insert() failed to check if the "
                                   "slot is used");
        }

        if (!is_used.Get()) { return SetUsed(); }

        ResultV<bool> next = NextSlot();
        if (next.IsError()) {
            return next +
                   Error("TableScan::Insert() failed to get the next slot");
        }

        if (!next.Get()) { break; }
    }

    Result allocate = transaction_.AllocateNewBlocks(block_id_ + 1);
    if (allocate.IsError()) {
        return allocate +
               Error("TableScan::Insert() failed to allocate new blocks");
    }

    SetBlockNumber(block_id_.BlockIndex() + 1);
    slot_ = 0;
    return SetUsed();
}

Result TableScan::Delete() {
    disk::DiskPosition position(/*block_id=*/block_id_,
                                /*offset=*/slot_ * layout_.Length());
    return transaction_.Write(position, data::kTypeByte.ValueLength(),
                              data::Byte(kUnusedFlag));
}

Result TableScan::Close() { return Ok(); }

bool DoesNextSlotExist(int slot, int slot_size, int whole_size) {
    return (slot + 2) * slot_size <= whole_size;
}

Result TableScan::CreateFirstBlock() {
    // Here, `block_id_` must be the first block of the database file.
    Result allocate = transaction_.AllocateNewBlocks(block_id_);
    if (allocate.IsError()) {
        return allocate +
               Error("TableScan::CreateFirstBlock() failed to allocate new "
                     "blocks");
    }
    return Ok();
}

ResultV<bool> TableScan::NextSlot() {
    if (DoesNextSlotExist(slot_, layout_.Length(), transaction_.BlockSize())) {
        slot_++;
        return Ok(true);
    }

    ResultV<size_t> size = transaction_.Size(TableFileName(table_name_));
    if (size.IsError()) {
        return size +
               Error("TableScan::Next() failed to get the size of the file");
    }

    int block_index = block_id_.BlockIndex();
    if (block_index + 1 < size.Get()) {
        SetBlockNumber(block_index + 1);
        slot_ = 0;
        return Ok(true);
    }

    return Ok(false);
}

ResultV<bool> TableScan::IsUsed() {
    disk::DiskPosition position(/*block_id=*/block_id_,
                                /*offset=*/slot_ * layout_.Length());
    data::DataItem item;
    FIRST_TRY(transaction_.Read(position, data::kTypeByte.ValueLength(), item));
    return Ok(data::ReadByte(item) == kUsedFlag);
}

Result TableScan::SetUsed() {
    disk::DiskPosition position(/*block_id=*/block_id_,
                                /*offset=*/slot_ * layout_.Length());
    return transaction_.Write(position, data::kTypeByte.ValueLength(),
                              data::Byte(kUsedFlag));
}

void TableScan::SetBlockNumber(int block_number) {
    block_id_ = disk::BlockID(TableFileName(table_name_), block_number);
}

} // namespace scan
