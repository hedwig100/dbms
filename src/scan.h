#ifndef _SCAN_H
#define _SCAN_H

#include "data/data.h"
#include "result.h"
#include <string>
#include <vector>

namespace scan {

using namespace ::result;

// Scan is an interface for reading data from a table or virtual table (like a
// view or join table)
class Scan {
  public:
    // Initialize the scan, ready to read the first row.
    // If there is no row, return false.
    virtual Result Init() = 0;

    // Move to the next row. Returns false if there are no more rows.
    virtual ResultV<bool> Next() = 0;

    // Get the bytes value of a field in the current row.
    virtual ResultV<data::DataItem> Get(const std::string &fieldname) = 0;

    // Closes the scan.
    virtual Result Close() = 0;
};

// UpdateScan is an interface for updating data in a table.
class UpdateScan : public Scan {
  public:
    // Update the value of a field in the current row.
    virtual Result Update(const std::string &fieldname,
                          const data::DataItem &item) = 0;

    // Insert a new row to somewhere in the table. The scan moves to the newly
    // inserted row.
    virtual Result Insert() = 0;

    // Delete the current row and move to the next row.
    virtual Result Delete() = 0;
};

} // namespace scan

#endif