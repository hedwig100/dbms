#ifndef _METADATA_H
#define _METADATA_H

#include "result.h"
#include "schema.h"
#include "transaction.h"
#include <string>

namespace metadata {

using namespace ::result;

// Responsible for managing the metadata of tables.
class TableManager {
  public:
    TableManager();

    // Creates a new table with the given name and schema.
    Result CreateTable(const std::string &table_name,
                       const schema::Schema &schema,
                       transaction::Transaction &transaction);

    // Retrieves the layout of the table with the given name.
    ResultV<schema::Layout> GetLayout(const std::string &table_name,
                                      transaction::Transaction &transaction);

  private:
    // Updates the metadata of the table with the given name.
    Result UpdateTableMetadata(const std::string &table_name,
                               const schema::Layout &layout,
                               transaction::Transaction &transaction);

    // Updates the metadata of the fields in the table with the given name.
    Result UpdateFieldMetadata(const std::string &table_name,
                               const schema::Schema &schema,
                               const schema::Layout &layout,
                               transaction::Transaction &transaction);
};

} // namespace metadata

#endif