#ifndef _METADATA_H
#define _METADATA_H

#include "result.h"
#include "schema.h"
#include "transaction.h"
#include <string>

namespace metadata {

using namespace ::result;

class TableManager {
  public:
    TableManager(transaction::Transaction &transaction);

    Result CreateTable(const std::string &table_name,
                       const schema::Schema &schema,
                       transaction::Transaction &transaction);

    ResultV<schema::Layout> GetLayout(const std::string &table_name,
                                      transaction::Transaction &transaction);

  private:
    Result UpdateTableMetadata(const std::string &table_name,
                               const schema::Layout &layout,
                               transaction::Transaction &transaction);

    Result UpdateFieldMetadata(const std::string &table_name,
                               const schema::Schema &schema,
                               const schema::Layout &layout,
                               transaction::Transaction &transaction);
};

} // namespace metadata

#endif