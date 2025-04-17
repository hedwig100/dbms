#ifndef _TABLE_SCAN_H
#define _TABLE_SCAN_H

#include "result.h"
#include "scan.h"
#include "schema.h"
#include "transaction/transaction.h"
#include <string>

namespace scan {

std::string TableFileName(const std::string &table_name);

class TableScan : public UpdateScan {
  public:
    TableScan(transaction::Transaction &transaction, std::string table_name,
              schema::Layout layout);

    // Initialize the scan, ready to read the first row.
    Result Init();

    // Move to the next row. Returns false if there are no more rows.
    ResultV<bool> Next();

    // Get the dataitem of a field in the current row.
    ResultV<data::DataItem> Get(const std::string &fieldname);

    // Get the int value of a field in the current row.
    ResultV<int> GetInt(const std::string &fieldname);

    // Get the string value of a field in the current row.
    ResultV<std::string> GetChar(const std::string &fieldname);

    // Update the value of a field in the current row.
    Result Update(const std::string &fieldname, const data::DataItem &item);

    // Insert a new row to somewhere in the table. The scan moves to the newly
    // inserted row.
    Result Insert();

    // Delete the current row and move to the next row.
    Result Delete();

    // Closes the scan.
    Result Close();

  private:
    // When the database file is empty, create the file and its first block.
    Result CreateFirstBlock();

    // Move to the next slot in the table. If there is a slot, returns true,
    // otherwise return false. It does not check if the slot is not empty.
    ResultV<bool> NextSlot();

    // Check if the currnt slot is used.
    ResultV<bool> IsUsed();

    // Set the current slot as used.
    Result SetUsed();

    // Set the block number.
    void SetBlockNumber(int block_number);

    transaction::Transaction &transaction_;
    std::string table_name_;
    schema::Layout layout_;

    disk::BlockID block_id_;
    int slot_;
};

} // namespace scan

#endif