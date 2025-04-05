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

} // namespace data