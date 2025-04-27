#include "metadata.h"
#include "data/char.h"
#include "data/int.h"
#include "table_scan.h"

namespace metadata {

constexpr int kMaxTablename = 32;
constexpr int kMaxFieldname = 32;

// The schema of the table metadata tables.
// This corresponds to the following SQL:
// CREATE TABLE tables (
//     table_name CHAR(32),
//     slot_size INT
// );
const std::string kTableTableName = "tables";
const schema::Schema
    kTableSchema({schema::Field("table_name", data::TypeChar(kMaxTablename)),
                  schema::Field("slot_size", data::TypeInt())});
const schema::Layout kTableLayout(kTableSchema);

// The schema of the field metadata tables.
// This corresponds to the following SQL:
// CREATE TABLE fields (
//     table_name CHAR(32),
//     field_name CHAR(32),
//     field_type INT,
//     field_length INT,
//     field_offset INT
// );
const std::string kFieldTableName = "fields";
const schema::Schema kFieldSchema({
    schema::Field("table_name", data::TypeChar(kMaxTablename)),
    schema::Field("field_name", data::TypeChar(kMaxFieldname)),
    schema::Field("field_type", data::TypeInt()),
    schema::Field("field_length", data::TypeInt()),
    schema::Field("field_offset", data::TypeInt()),
});
const schema::Layout kFieldLayout(kFieldSchema);

TableManager::TableManager() {}

Result TableManager::CreateTable(const std::string &table_name,
                                 const schema::Schema &schema,
                                 transaction::Transaction &transaction) const {
    schema::Layout layout(schema);
    FIRST_TRY(UpdateTableMetadata(table_name, layout, transaction));
    TRY(UpdateFieldMetadata(table_name, schema, layout, transaction));
    return Ok();
}

Result
TableManager::UpdateTableMetadata(const std::string &table_name,
                                  const schema::Layout &layout,
                                  transaction::Transaction &transaction) const {
    if (table_name.size() > kMaxTablename) {
        return Error(
            "TableManager::UpdateTableMetadata() table name is too long");
    }

    scan::TableScan table_scan(transaction, kTableTableName, kTableLayout);
    FIRST_TRY(table_scan.Init());
    TRY(table_scan.Insert());
    TRY(table_scan.Update("table_name", data::Char(table_name, kMaxTablename)));
    TRY(table_scan.Update("slot_size", data::Int(layout.Length())));
    TRY(table_scan.Close());
    return Ok();
}

Result TableManager::UpdateFieldMetadata(
    const std::string &table_name, const schema::Schema &schema,
    const schema::Layout &layout, transaction::Transaction &transaction) const {
    scan::TableScan field_scan(transaction, kFieldTableName, kFieldLayout);
    FIRST_TRY(field_scan.Init());
    for (const schema::Field &field : schema.Fields()) {
        if (field.FieldName().size() > kMaxFieldname) {
            return Error(
                "TableManager::UpdateFieldMetadata() field name is too long");
        }

        TRY(field_scan.Insert());
        TRY(field_scan.Update("table_name",
                              data::Char(table_name, kMaxTablename)));
        TRY(field_scan.Update("field_name",
                              data::Char(field.FieldName(), kMaxFieldname)));
        TRY(field_scan.Update("field_type",
                              data::Int(static_cast<int>(field.Type()))));
        TRY(field_scan.Update("field_length", data::Int(field.Length())));
        TRY_VALUE(field_offset, layout.Offset(field.FieldName()));
        TRY(field_scan.Update("field_offset", data::Int(field_offset.Get())));
    }
    TRY(field_scan.Close());
    return Ok();
}

ResultV<schema::Layout>
TableManager::GetLayout(const std::string &table_name,
                        transaction::Transaction &transaction) const {
    int slot_size = 0;
    scan::TableScan table_scan(transaction, kTableTableName, kTableLayout);
    FIRST_TRY(table_scan.Init());
    while (true) {
        TRY_VALUE(name, table_scan.GetChar("table_name"));
        if (name.Get() == table_name) {
            TRY_VALUE(slot_size_result, table_scan.GetInt("slot_size"));
            slot_size = slot_size_result.Get();
            break;
        }

        TRY_VALUE(has_next, table_scan.Next());
        if (!has_next.Get()) break;
    }
    TRY(table_scan.Close());

    if (slot_size == 0) {
        return Error("TableManager::GetLayout() failed to find the table '" +
                     table_name + "'");
    }

    scan::TableScan field_scan(transaction, kFieldTableName, kFieldLayout);
    TRY(field_scan.Init());
    std::unordered_map<std::string, int> field_lengths, offsets;
    while (true) {
        TRY_VALUE(name, field_scan.GetChar("table_name"));
        if (name.Get() == table_name) {
            TRY_VALUE(field_name, field_scan.GetChar("field_name"));
            TRY_VALUE(field_length, field_scan.GetInt("field_length"));
            TRY_VALUE(field_offset, field_scan.GetInt("field_offset"));
            field_lengths[field_name.Get()] = field_length.Get();
            offsets[field_name.Get()]       = field_offset.Get();
        }

        TRY_VALUE(has_next, field_scan.Next());
        if (!has_next.Get()) break;
    }
    TRY(field_scan.Close());

    return Ok(schema::Layout(slot_size, field_lengths, offsets));
}

} // namespace metadata