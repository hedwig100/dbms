#include "data/char.h"
#include "data/int.h"
#include "macro_test_transaction.h"
#include "metadata.h"
#include <gtest/gtest.h>

class MetadataManagerTest : public TransactionTest {};

TEST_F(MetadataManagerTest, CreateTableSuccess) {
    metadata::TableManager manager;
    schema::Schema schema({schema::Field("field0", data::TypeInt()),
                           schema::Field("field1", data::TypeChar(10))});
    transaction::Transaction transaction(data_disk_manager, buffer_manager,
                                         log_manager, lock_table);

    auto res = manager.CreateTable("table0", schema, transaction);

    EXPECT_TRUE(res.IsOk()) << res.Error();
}

TEST_F(MetadataManagerTest, GetLayoutSuucess) {
    metadata::TableManager manager;
    schema::Schema schema({schema::Field("field0", data::TypeInt()),
                           schema::Field("field1", data::TypeChar(10))});
    transaction::Transaction transaction(data_disk_manager, buffer_manager,
                                         log_manager, lock_table);
    auto res = manager.CreateTable("table0", schema, transaction);
    ASSERT_TRUE(res.IsOk()) << res.Error();

    auto layout_res = manager.GetLayout("table0", transaction);

    EXPECT_TRUE(layout_res.IsOk()) << layout_res.Error();
    auto layout = layout_res.Get();
    EXPECT_EQ(layout.Length(), 15);
    EXPECT_EQ(layout.Length("field0"), 4);
    EXPECT_EQ(layout.Length("field1"), 10);
    EXPECT_EQ(layout.Offset("field0"), 1);
    EXPECT_EQ(layout.Offset("field1"), 5);
}