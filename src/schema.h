#ifndef _SCHEMA_H_
#define _SCHEMA_H_

#include "data/data.h"
#include "result.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace schema {

using namespace ::result;

class Field {
  public:
    Field(const std::string &fieldname, const data::DataType &datatype)
        : fieldname(fieldname), datatype(datatype.BaseType()),
          length(datatype.ValueLength()) {}

    const std::string &FieldName() const { return fieldname; }

    data::BaseDataType Type() const { return datatype; }

    int Length() const { return length; }

  private:
    std::string fieldname;
    data::BaseDataType datatype;
    int length;
};

class Schema {
  public:
    explicit Schema(const std::vector<Field> &fields) : fields(fields) {}

    // Add a field to the schema.
    void AddField(const Field &field) { fields.push_back(field); }

    // Returns all fields in the schema.
    const std::vector<Field> &Fields() const { return fields; }

  private:
    std::vector<Field> fields;
};

class Layout {
  public:
    explicit Layout(const Schema &schema);
    explicit Layout(int length,
                    std::unordered_map<std::string, int> field_lengths,
                    std::unordered_map<std::string, int> offsets);

    // Returns the offset of the field. If the field does not exist, raise an
    // exception.
    int Offset(const std::string &fieldname) const {
        return offsets_.at(fieldname);
    }

    // Returns the length of the field. If the field does not exist, raise an
    // exception.
    int Length(const std::string &fieldname) const {
        return field_lengths_.at(fieldname);
    }

    // Returns the length of the record (schema).
    int Length() const { return length_; }

    // Returns all field names in the schema.
    const std::vector<std::string> &FieldNames() const {
        return sorted_field_names_;
    }

    // Returns true if the field exists in the schema.
    bool HasField(const std::string &fieldname) const {
        return field_lengths_.find(fieldname) != field_lengths_.end();
    }

  private:
    int length_;
    std::unordered_map<std::string, int> field_lengths_;
    std::unordered_map<std::string, int> offsets_;
    std::vector<std::string> sorted_field_names_;
};

} // namespace schema

#endif // _SCHEMA_H_