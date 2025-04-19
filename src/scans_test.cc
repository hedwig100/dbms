#include "data/int.h"
#include "scan.h"
#include "scans.h"
#include <gtest/gtest.h>

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

TEST(SelectScan, Init) {
    ScanForTest scan;
    scan::SelectScan select_scan(scan);
    EXPECT_TRUE(select_scan.Init().IsOk());
}

TEST(SelectScan, Next) {
    ScanForTest scan;
    scan::SelectScan select_scan(scan);
    EXPECT_TRUE(select_scan.Init().IsOk());
    EXPECT_TRUE(select_scan.Next().Get());
    EXPECT_TRUE(select_scan.Next().Get());
    EXPECT_FALSE(select_scan.Next().Get());
}

TEST(SelectScan, Get) {
    ScanForTest scan;
    scan::SelectScan select_scan(scan);
    EXPECT_TRUE(select_scan.Init().IsOk());
    EXPECT_TRUE(select_scan.Next().Get());
    auto result = select_scan.Get("field1");
    EXPECT_TRUE(result.IsOk());
    EXPECT_EQ(data::ReadInt(result.Get()), 3);
    result = select_scan.Get("field2");
    EXPECT_TRUE(result.IsOk());
    EXPECT_EQ(data::ReadInt(result.Get()), 4);
}

TEST(SelectScan, Update) {
    ScanForTest scan;
    scan::SelectScan select_scan(scan);
    EXPECT_TRUE(select_scan.Init().IsOk());
    EXPECT_TRUE(select_scan.Next().Get());
    auto result = select_scan.Get("field1");
    EXPECT_TRUE(result.IsOk());
    EXPECT_EQ(data::ReadInt(result.Get()), 3);
    EXPECT_TRUE(select_scan.Update("field1", data::Int(10)).IsOk());
    result = select_scan.Get("field1");
    EXPECT_TRUE(result.IsOk());
    EXPECT_EQ(data::ReadInt(result.Get()), 10);
}

TEST(SelectScan, Insert) {
    ScanForTest scan;
    scan::SelectScan select_scan(scan);
    EXPECT_TRUE(select_scan.Init().IsOk());
    EXPECT_TRUE(select_scan.Insert().IsOk());
    EXPECT_TRUE(select_scan.Update("field1", data::Int(20)).IsOk());
    auto result = select_scan.Get("field1");
    EXPECT_TRUE(result.IsOk());
    EXPECT_EQ(data::ReadInt(result.Get()), 20);
}

TEST(SelectScan, Delete) {
    ScanForTest scan;
    scan::SelectScan select_scan(scan);
    EXPECT_TRUE(select_scan.Init().IsOk());
    EXPECT_TRUE(select_scan.Next().Get());
    EXPECT_TRUE(select_scan.Delete().IsOk());
    EXPECT_FALSE(select_scan.Next().Get());
}

TEST(SelectScan, Close) {
    ScanForTest scan;
    scan::SelectScan select_scan(scan);
    EXPECT_TRUE(select_scan.Init().IsOk());
    EXPECT_TRUE(select_scan.Close().IsOk());
}