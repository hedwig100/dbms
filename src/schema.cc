#include "schema.h"
#include <algorithm>

namespace schema {

// The first byte is used for the used/unused flag.
constexpr int kUsedFlagOffset = 0;
constexpr int kUsedFlagLength = 1;

Layout::Layout(const Schema &schema) {
    int offset = kUsedFlagLength;
    for (const auto &field : schema.Fields()) {
        field_lengths_[field.FieldName()] = field.Length();
        field_types_[field.FieldName()]   = field.Type();
        offsets_[field.FieldName()]       = offset;
        sorted_field_names_.push_back(field.FieldName());
        offset += field.Length();
    }
    length_ = offset;
}

Layout::Layout(int length,
               std::unordered_map<std::string, data::BaseDataType> field_types,
               std::unordered_map<std::string, int> field_lengths,
               std::unordered_map<std::string, int> offsets)
    : length_(length), field_lengths_(field_lengths), field_types_(field_types),
      offsets_(offsets) {
    std::vector<std::pair<int, std::string>> field_offsets;
    for (const auto &pair : offsets_) {
        field_offsets.emplace_back(pair.second, pair.first);
    }
    std::sort(field_offsets.begin(), field_offsets.end());
    for (const auto &pair : field_offsets) {
        sorted_field_names_.push_back(pair.second);
    }
}

} // namespace schema