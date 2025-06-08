#include "scans_test.h"
#include "data/int.h"
#include "scan.h"
#include "scans.h"
#include <gtest/gtest.h>

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
    EXPECT_EQ(data::ReadInt(result.Get().Item()), 3);
    result = select_scan.Get("field2");
    EXPECT_TRUE(result.IsOk());
    EXPECT_EQ(data::ReadInt(result.Get().Item()), 4);
}

TEST(SelectScan, Update) {
    ScanForTest scan;
    scan::SelectScan select_scan(scan);
    EXPECT_TRUE(select_scan.Init().IsOk());
    EXPECT_TRUE(select_scan.Next().Get());
    auto result = select_scan.Get("field1");
    EXPECT_TRUE(result.IsOk());
    EXPECT_EQ(data::ReadInt(result.Get().Item()), 3);
    EXPECT_TRUE(select_scan.Update("field1", data::Int(10)).IsOk());
    result = select_scan.Get("field1");
    EXPECT_TRUE(result.IsOk());
    EXPECT_EQ(data::ReadInt(result.Get().Item()), 10);
}

TEST(SelectScan, Insert) {
    ScanForTest scan;
    scan::SelectScan select_scan(scan);
    EXPECT_TRUE(select_scan.Init().IsOk());
    EXPECT_TRUE(select_scan.Insert().IsOk());
    EXPECT_TRUE(select_scan.Update("field1", data::Int(20)).IsOk());
    auto result = select_scan.Get("field1");
    EXPECT_TRUE(result.IsOk());
    EXPECT_EQ(data::ReadInt(result.Get().Item()), 20);
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