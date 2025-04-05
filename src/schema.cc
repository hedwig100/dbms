#include "schema.h"

namespace schema {

// The first byte is used for the used/unused flag.
constexpr int kUsedFlagOffset = 0;
constexpr int kUsedFlagLength = 1;

Layout::Layout(const Schema &schema) {
    int offset = kUsedFlagLength;
    for (const auto &field : schema.Fields()) {
        field_lengths_[field.FieldName()] = field.Length();
        offsets_[field.FieldName()]       = offset;
        offset += field.Length();
    }
    length_ = offset;
}

} // namespace schema