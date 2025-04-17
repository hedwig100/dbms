#ifndef _SCANS_H
#define _SCANS_H

#include "result.h"
#include "scan.h"
#include "table_scan.h"

namespace scan {

using namespace ::result;

class SelectScan : public UpdateScan {
  public:
    SelectScan(UpdateScan &scan);

    // Initialize the scan, ready to read the first row.
    // If there is no row, return false.
    Result Init();

    // Move to the next row. Returns false if there are no more rows.
    ResultV<bool> Next();

    // Get the bytes value of a field in the current row.
    ResultV<data::DataItem> Get(const std::string &fieldname);

    Result Update(const std::string &fieldname, const data::DataItem &item);

    // Insert a new row to somewhere in the table. The scan moves to the newly
    // inserted row.
    Result Insert();

    // Delete the current row and move to the next row.
    Result Delete();

    // Closes the scan.
    Result Close();

  private:
    UpdateScan &scan_;
};

} // namespace scan

#endif // _SCANS_H