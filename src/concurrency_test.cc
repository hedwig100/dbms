#include "concurrency.h"
#include "disk.h"
#include <chrono>
#include <gtest/gtest.h>
#include <iostream>
#include <thread>

dbconcurrency::LockTable lock_table(/*wait_time_sec=*/2);
std::map<disk::BlockID, int> shared;
const disk::BlockID block0("a.txt", 0), block1("a.txt", 2), block2("a.txt", 4),
    block3("a.txt", 7);

void WriteLockCorrectlyLockOtherThread0() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Result result = lock_table.WriteLock(block0);
    EXPECT_TRUE(result.IsOk());
    shared[block0] = 5;

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    lock_table.Release(block0);
}

void WriteLockCorrectlyLockOtherThread1() {
    Result result = lock_table.ReadLock(block0);
    EXPECT_TRUE(result.IsOk());
    EXPECT_EQ(shared[block0], 0);
    lock_table.Release(block0);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    result = lock_table.ReadLock(block0);
    EXPECT_TRUE(result.IsOk());
    EXPECT_EQ(shared[block0], 5);
    lock_table.Release(block0);
}

TEST(ConcurrencyLockTable, WriteLockCorrectlyLockOtherThread) {
    std::thread thread0(WriteLockCorrectlyLockOtherThread0);
    std::thread thread1(WriteLockCorrectlyLockOtherThread1);
    thread0.join();
    thread1.join();
}

void ReadLockCorrectlyLockWriteThread0() {
    Result result = lock_table.WriteLock(block1);
    EXPECT_TRUE(result.IsOk());
    shared[block1] = 8;
    lock_table.Release(block1);

    result = lock_table.ReadLock(block1);
    EXPECT_TRUE(result.IsOk());

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    lock_table.Release(block1);
}

void ReadLockCorrectlyLockWriteThread1() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Result result = lock_table.ReadLock(block1);
    EXPECT_TRUE(result.IsOk());
    EXPECT_EQ(shared[block1], 8);
    lock_table.Release(block1);

    result = lock_table.WriteLock(block1);
    EXPECT_TRUE(result.IsOk());
    EXPECT_EQ(shared[block1], 8);
    shared[block1] = 10;
    lock_table.Release(block1);

    result = lock_table.ReadLock(block1);
    EXPECT_TRUE(result.IsOk());
    EXPECT_EQ(shared[block1], 10);
    lock_table.Release(block1);
}

TEST(ConcurrencyLockTable, ReadLockCorrectlyLockWriteThread) {
    std::thread thread0(ReadLockCorrectlyLockWriteThread0);
    std::thread thread1(ReadLockCorrectlyLockWriteThread1);
    thread0.join();
    thread1.join();
    EXPECT_EQ(shared[block1], 10);
}

void DetectDeadLock0() {
    Result result = lock_table.WriteLock(block2);
    EXPECT_TRUE(result.IsOk());

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    result = lock_table.ReadLock(block3);
    EXPECT_TRUE(result.IsOk());
    lock_table.Release(block2);
    lock_table.Release(block3);
}

void DetectDeadLock1() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Result result = lock_table.WriteLock(block3);
    EXPECT_TRUE(result.IsOk());

    result = lock_table.ReadLock(block2);
    EXPECT_TRUE(result.IsError()); // Dead locked!
    lock_table.Release(block3);
}

TEST(ConcurrencyLockTable, DetectDeadLock) {
    std::thread thread0(DetectDeadLock0);
    std::thread thread1(DetectDeadLock1);
    thread0.join();
    thread1.join();
}