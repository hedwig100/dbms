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

    // Returns the offset of the field. If the field does not exist, raise an
    // exception.
    int Offset(const std::string &fieldname) const {
        return offsets.at(fieldname);
    }

    // Returns the length of the field. If the field does not exist, raise an
    // exception.
    int Length(const std::string &fieldname) const {
        return field_lengths.at(fieldname);
    }

    // Returns the length of the record (schema).
    int Length() const { return length; }

  private:
    int length;
    std::unordered_map<std::string, int> field_lengths;
    std::unordered_map<std::string, int> offsets;
};

} // namespace schema

#endif // _SCHEMA_H_