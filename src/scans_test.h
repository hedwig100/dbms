#ifndef SCANS_TEST_H
#define SCANS_TEST_H

#include "data/int.h"
#include "result.h"
#include "scan.h"
#include <map>

using namespace result;

class ScanForTest : public scan::UpdateScan {
  public:
    ScanForTest() {}

    // Initialize the scan, ready to read the first row.
    Result Init() override { return Ok(); }

    // Move to the next row. Returns false if there are no more rows.
    ResultV<bool> Next() override {
        if (current_row_ >= records_.size()) { return Ok(false); }
        current_row_++;
        while (current_row_ < records_.size() &&
               records_[current_row_].is_empty) {
            current_row_++;
        }
        return Ok(current_row_ < records_.size());
    }

    // Get the bytes value of a field in the current row.
    ResultV<data::DataItem> Get(const std::string &fieldname) override {
        if (records_[current_row_].is_empty) { return Error("Row is empty"); }
        auto it = records_[current_row_].values.find(fieldname);
        if (it == records_[current_row_].values.end()) {
            return Error("Field not found");
        }
        return Ok(data::DataItem(it->second));
    }

    Result Update(const std::string &fieldname,
                  const data::DataItem &item) override {
        if (records_[current_row_].is_empty) { return Error("Row is empty"); }
        records_[current_row_].values[fieldname] = item;
        return Ok();
    }

    Result Insert() override {
        if (current_row_ >= records_.size()) {
            records_.emplace_back(Record{false, {}});
        }
        if (records_[current_row_].is_empty) {
            records_[current_row_].is_empty = false;
        } else {
            records_.emplace_back(Record{false, {}});
            current_row_ = records_.size() - 1;
        }
        return Ok();
    }

    Result Delete() override {
        if (current_row_ >= records_.size()) { return Error("Row not found"); }
        records_[current_row_].is_empty = true;
        current_row_++;
        while (current_row_ < records_.size() &&
               records_[current_row_].is_empty) {
            current_row_++;
        }
        return Ok();
    }

    Result Close() override { return Ok(); }

  private:
    struct Record {
        bool is_empty;
        std::map<std::string, data::DataItem> values;
    };
    int current_row_             = 0;
    std::vector<Record> records_ = {
        {false, {{"field1", data::Int(1)}, {"field2", data::Int(2)}}},
        {false, {{"field1", data::Int(3)}, {"field2", data::Int(4)}}},
        {true, {}},
        {false, {{"field1", data::Int(5)}, {"field2", data::Int(6)}}},
    };
};

#endif // SCANS_TEST_H