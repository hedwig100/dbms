#include "scans.h"

namespace scan {

SelectScan::SelectScan(UpdateScan &scan) : scan_(scan) {}

Result SelectScan::Init() { return scan_.Init(); }

ResultV<bool> SelectScan::Next() { return scan_.Next(); }

ResultV<data::DataItem> SelectScan::Get(const std::string &fieldname) {
    return scan_.Get(fieldname);
}

Result SelectScan::Update(const std::string &fieldname,
                          const data::DataItem &item) {
    return scan_.Update(fieldname, item);
}

Result SelectScan::Insert() { return scan_.Insert(); }

Result SelectScan::Delete() { return scan_.Delete(); }

Result SelectScan::Close() { return scan_.Close(); }

} // namespace scan