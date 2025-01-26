#include "schema.h"

namespace schema {

constexpr int kUsedFlagOffset = 0;
constexpr int kUsedFlagLength = 1;

Layout::Layout(const Schema &schema) : schema(schema) {
    int offset = kUsedFlagLength;
    for (const auto &field : schema.Fields()) {
        offsets[field.FieldName()] = offset;
        offset += field.Length();
    }
    length = offset;
}

} // namespace schema