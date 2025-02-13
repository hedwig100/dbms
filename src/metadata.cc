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

TableManager::TableManager(transaction::Transaction &transaction) {
    CreateTable(kTableTableName, kTableSchema, transaction);
    CreateTable(kFieldTableName, kFieldSchema, transaction);
}

Result TableManager::CreateTable(const std::string &table_name,
                                 const schema::Schema &schema,
                                 transaction::Transaction &transaction) {
    // TODO: Create file corresponding to the table.
    schema::Layout layout(schema);
    Result result = UpdateTableMetadata(table_name, layout, transaction);
    if (result.IsError()) {
        return result + Error("TableManager::CreateTable() Failed to update "
                              "table metadata");
    }

    result = UpdateFieldMetadata(table_name, schema, layout, transaction);
    if (result.IsError()) {
        return result + Error("TableManager::CreateTable() Failed to update "
                              "field metadata");
    }
    return Ok();
}

Result
TableManager::UpdateTableMetadata(const std::string &table_name,
                                  const schema::Layout &layout,
                                  transaction::Transaction &transaction) {
    // TODO: Error handling with Insert, Update, Close.
    scan::TableScan table_scan(transaction, kTableTableName, kTableLayout);
    ResultV<bool> result = table_scan.Init();
    if (result.IsError()) {
        return result + Error("TableManager::CreateTable() Failed to "
                              "initialize table scan");
    }
    table_scan.Insert();
    table_scan.Update("table_name", data::Char(table_name, kMaxTablename));
    table_scan.Update("slot_size", data::Int(layout.Length()));
    table_scan.Close();
    return Ok();
}

Result TableManager::UpdateFieldMetadata(
    const std::string &table_name, const schema::Schema &schema,
    const schema::Layout &layout, transaction::Transaction &transaction) {
    // TODO: Error handling with Insert, Update, Close.
    scan::TableScan field_scan(transaction, kFieldTableName, kFieldLayout);
    ResultV<bool> result = field_scan.Init();
    if (result.IsError()) {
        return result + Error("TableManager::CreateTable() Failed to "
                              "initialize field scan");
    }
    for (const schema::Field &field : schema.Fields()) {
        field_scan.Insert();
        field_scan.Update("table_name", data::Char(table_name, kMaxTablename));
        field_scan.Update("field_name",
                          data::Char(field.FieldName(), kMaxFieldname));
        field_scan.Update("field_type",
                          data::Int(static_cast<int>(field.Type())));
        field_scan.Update("field_length", data::Int(field.Length()));
        field_scan.Update("field_offset",
                          data::Int(layout.Offset(field.FieldName())));
    }
    field_scan.Close();
    return Ok();
}

ResultV<schema::Layout>
TableManager::GetLayout(const std::string &table_name,
                        transaction::Transaction &transaction) {}

} // namespace metadata