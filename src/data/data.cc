#include "data/data.h"

namespace data {

size_t DataItem::size() const {
    if (std::holds_alternative<ConstLengthItem>(item_)) {
        return std::get<ConstLengthItem>(item_).size();
    } else {
        return std::get<VariableLengthItem>(item_).size();
    }
}

void DataItem::resize(size_t length) {
    if (std::holds_alternative<ConstLengthItem>(item_)) {
        if (length <= kMaxItemLength) return;
        // Convert to variable length, this case is not common.
        item_ = VariableLengthItem(length);
    } else {
        std::get<VariableLengthItem>(item_).resize(length);
    }
}

uint8_t *DataItem::begin() {
    return std::visit(
        [](auto &container) -> uint8_t * { return container.data(); }, item_);
}

const uint8_t *DataItem::begin() const {
    return std::visit(
        [](const auto &container) -> const uint8_t * {
            return container.data();
        },
        item_);
}

uint8_t *DataItem::end() {
    return std::visit(
        [](auto &container) -> uint8_t * {
            return container.data() + container.size();
        },
        item_);
}

const uint8_t *DataItem::end() const {
    return std::visit(
        [](auto &container) -> const uint8_t * {
            return container.data() + container.size();
        },
        item_);
}

bool DataItem::operator==(const DataItem &other) const {
    if (size() != other.size()) return false;
    return std::equal(begin(), end(), other.begin());
}

bool DataItem::operator!=(const DataItem &other) const {
    return !(*this == other);
}

Result Read(DataItem &item, const std::vector<uint8_t> &bytes,
            const size_t offset, int length) {
    if (offset < 0 || offset + length > bytes.size())
        return Error("data::internal::Read() offset should be fit the size.");
    item.resize(length);
    std::copy(bytes.begin() + offset, bytes.begin() + offset + length,
              item.begin());
    return Ok();
}

Result Write(const DataItem &item, std::vector<uint8_t> &bytes,
             const size_t offset, int length) {
    if (offset < 0 || offset + length > bytes.size())
        return Error("data::internal::Write() offset should be fit the size.");
    if (item.size() < length)
        return Error("data::internal::Write() item size should be larger than "
                     "length.");
    std::copy(item.begin(), item.begin() + length, bytes.begin() + offset);
    return Ok();
}

} // namespace data