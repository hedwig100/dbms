#include "data/int.h"
#include "macro_test_transaction.h"
#include "transaction.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iterator>
#include <random>
#include <thread>
#include <vector>

#define TRANSACTION_FUNCTION(function_name, content)                           \
    void function_name(disk::DiskManager &disk_manager,                        \
                       buffer::BufferManager &buffer_manager,                  \
                       dblog::LogManager &log_manager,                         \
                       dbconcurrency::LockTable &lock_table,                   \
                       const std::string data_filename) {                      \
        transaction::Transaction transaction(disk_manager, buffer_manager,     \
                                             log_manager, lock_table);         \
        content                                                                \
    }

TRANSACTION_FUNCTION(func0, {
    Result result = transaction.Write(
        disk::DiskPosition(disk::BlockID(data_filename, 0), 0),
        data::kTypeInt.ValueLength(), data::Int(9));
    if (result.IsError()) { std::cerr << result.Error() << std::endl; }

    result = transaction.Commit();
    if (result.IsError()) { std::cerr << result.Error() << std::endl; }
})

TRANSACTION_FUNCTION(func1, {
    Result result = transaction.Write(
        disk::DiskPosition(disk::BlockID(data_filename, 0), 6),
        data::kTypeInt.ValueLength(), data::Int(8));
    if (result.IsError()) { std::cerr << result.Error() << std::endl; }

    result = transaction.Commit();
    if (result.IsError()) { std::cerr << result.Error() << std::endl; }
})

TEST_F(TransactionTest, random) {
    std::thread thread0(func0, std::ref(data_disk_manager),
                        std::ref(buffer_manager), std::ref(log_manager),
                        std::ref(lock_table), data_filename);
    std::thread thread1(func1, std::ref(data_disk_manager),
                        std::ref(buffer_manager), std::ref(log_manager),
                        std::ref(lock_table), data_filename);
    thread0.join();
    thread1.join();

    ResultV<int> result = transaction_for_check.ReadInt(
        disk::DiskPosition(disk::BlockID(data_filename, 0), 0));
    EXPECT_TRUE(result.IsOk()) << result.Error();
    EXPECT_EQ(result.Get(), 9);

    ResultV<int> result1 = transaction_for_check.ReadInt(
        disk::DiskPosition(disk::BlockID(data_filename, 0), 6));
    EXPECT_TRUE(result1.IsOk()) << result1.Error();
    EXPECT_EQ(result1.Get(), 8);
}

int RandomInteger(int lower, int upper) {
    std::random_device seed_gen;
    std::mt19937 engine(seed_gen());
    std::uniform_int_distribution<int> dist(lower, upper);
    return dist(engine);
}

void RandomShuffle(std::vector<int> &index) {
    std::random_device seed_gen;
    std::mt19937 engine(seed_gen());
    std::shuffle(index.begin(), index.end(), engine);
}

// A: Tests if the transactions are atomic.

const std::vector<disk::DiskPosition> kPositionsForAtomicityTest = {
    disk::DiskPosition(disk::BlockID(data_filename, 0), 0),
    disk::DiskPosition(disk::BlockID(data_filename, 0), 4),
    disk::DiskPosition(disk::BlockID(data_filename, 0), 10),
    disk::DiskPosition(disk::BlockID(data_filename, 1), 3),
    disk::DiskPosition(disk::BlockID(data_filename, 2), 3),
    disk::DiskPosition(disk::BlockID(data_filename, 2), 9),
    disk::DiskPosition(disk::BlockID(data_filename, 3), 5),
    disk::DiskPosition(disk::BlockID(data_filename, 4), 8),
};

std::mutex commit_result_mutex;
std::vector<bool> commit_results(4, false);

TRANSACTION_FUNCTION(AtomicityTestFunc1, {
    for (int i : {0, 1}) {
        Result write_result =
            transaction.Write(kPositionsForAtomicityTest[i],
                              data::kTypeInt.ValueLength(), data::Int(1));
        if (write_result.IsError()) {
            std::cerr << "Transaction1 " << write_result.Error() << std::endl;
            return;
        }
    }

    for (int i : {0, 1}) {
        ResultV<int> read_result =
            transaction.ReadInt(kPositionsForAtomicityTest[i]);
        if (read_result.IsError()) {
            std::cerr << "Transaction1 " << read_result.Error() << std::endl;
            return;
        }
        EXPECT_EQ(read_result.Get(), 1);
    }

    ResultV<int> int_result =
        transaction.ReadInt(kPositionsForAtomicityTest[RandomInteger(0, 7)]);
    if (int_result.IsError()) {
        std::cerr << "Transaction1 " << int_result.Error() << std::endl;
        return;
    }

    Result result = transaction.Commit();
    if (result.IsError()) {
        std::cerr << "Transaction1 " << result.Error() << std::endl;
    }

    std::lock_guard<std::mutex> lock(commit_result_mutex);
    commit_results[0] = true;
})

TRANSACTION_FUNCTION(AtomicityTestFunc2, {
    for (int i : {2, 3}) {
        Result write_result =
            transaction.Write(kPositionsForAtomicityTest[i],
                              data::kTypeInt.ValueLength(), data::Int(2));
        if (write_result.IsError()) {
            std::cerr << "Transaction2 " << write_result.Error() << std::endl;
            return;
        }
    }

    for (int i : {2, 3}) {
        ResultV<int> read_result =
            transaction.ReadInt(kPositionsForAtomicityTest[i]);
        if (read_result.IsError()) {
            std::cerr << "Transaction2 " << read_result.Error() << std::endl;
            return;
        }
        EXPECT_EQ(read_result.Get(), 2);
    }

    ResultV<int> int_result =
        transaction.ReadInt(kPositionsForAtomicityTest[RandomInteger(0, 7)]);
    if (int_result.IsError()) {
        std::cerr << "Transaction2 " << int_result.Error() << std::endl;
        return;
    }

    Result result = transaction.Commit();
    if (result.IsError()) {
        std::cerr << "Transaction2 " << result.Error() << std::endl;
    }

    std::lock_guard<std::mutex> lock(commit_result_mutex);
    commit_results[1] = true;
})

TRANSACTION_FUNCTION(AtomicityTestFunc3, {
    for (int i : {4, 5}) {
        Result write_result =
            transaction.Write(kPositionsForAtomicityTest[i],
                              data::kTypeInt.ValueLength(), data::Int(3));
        if (write_result.IsError()) {
            std::cerr << "Transaction3 " << write_result.Error() << std::endl;
            return;
        }
    }

    for (int i : {4, 5}) {
        ResultV<int> read_result =
            transaction.ReadInt(kPositionsForAtomicityTest[i]);
        if (read_result.IsError()) {
            std::cerr << "Transaction3 " << read_result.Error() << std::endl;
            return;
        }
        EXPECT_EQ(read_result.Get(), 3);
    }

    ResultV<int> int_result =
        transaction.ReadInt(kPositionsForAtomicityTest[RandomInteger(0, 7)]);
    if (int_result.IsError()) {
        std::cerr << "Transaction3 " << int_result.Error() << std::endl;
        return;
    }

    Result result = transaction.Commit();
    if (result.IsError()) {
        std::cerr << "Transaction3 " << result.Error() << std::endl;
    }

    std::lock_guard<std::mutex> lock(commit_result_mutex);
    commit_results[2] = true;
})

TRANSACTION_FUNCTION(AtomicityTestFunc4, {
    for (int i : {6, 7}) {
        Result write_result =
            transaction.Write(kPositionsForAtomicityTest[i],
                              data::kTypeInt.ValueLength(), data::Int(4));
        if (write_result.IsError()) {
            std::cerr << "Transaction4 " << write_result.Error() << std::endl;
            return;
        }
    }

    for (int i : {6, 7}) {
        ResultV<int> read_result =
            transaction.ReadInt(kPositionsForAtomicityTest[i]);
        if (read_result.IsError()) {
            std::cerr << "Transaction4 " << read_result.Error() << std::endl;
            return;
        }
        EXPECT_EQ(read_result.Get(), 4);
    }

    ResultV<int> int_result =
        transaction.ReadInt(kPositionsForAtomicityTest[RandomInteger(0, 7)]);
    if (int_result.IsError()) {
        std::cerr << "Transaction4 " << int_result.Error() << std::endl;
        return;
    }

    Result result = transaction.Commit();
    if (result.IsError()) {
        std::cerr << "Transaction4 " << result.Error() << std::endl;
    }

    std::lock_guard<std::mutex> lock(commit_result_mutex);
    commit_results[3] = true;
})

TEST_F(TransactionTest, AtomicityTest) {
    // The test consists of four transactions. Each transaction writes two
    // integers to the disk. The first transaction writes 1, the second writes
    // 2, the third writes 3, and the fourth writes 4. The test checks if the
    // transactions are atomic, i.e., if the transactions are committed or
    // rollbacked. If the transaction is committed, the test checks if the
    // written data is correct. If the transaction is rollbacked, the test
    // checks if the written data is not written.
    std::thread thread1(AtomicityTestFunc1, std::ref(data_disk_manager),
                        std::ref(buffer_manager), std::ref(log_manager),
                        std::ref(lock_table), data_filename);
    std::thread thread2(AtomicityTestFunc2, std::ref(data_disk_manager),
                        std::ref(buffer_manager), std::ref(log_manager),
                        std::ref(lock_table), data_filename);
    std::thread thread3(AtomicityTestFunc3, std::ref(data_disk_manager),
                        std::ref(buffer_manager), std::ref(log_manager),
                        std::ref(lock_table), data_filename);
    std::thread thread4(AtomicityTestFunc4, std::ref(data_disk_manager),
                        std::ref(buffer_manager), std::ref(log_manager),
                        std::ref(lock_table), data_filename);

    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();

    for (int i = 0; i < 4; i++) {
        if (commit_results[i]) {
            for (int j = 0; j < 2; j++) {
                ResultV<int> result = transaction_for_check.ReadInt(
                    kPositionsForAtomicityTest[i * 2 + j]);
                ASSERT_TRUE(result.IsOk()) << result.Error();
                EXPECT_EQ(result.Get(), i + 1);
            }
        } else {
            for (int j = 0; j < 2; j++) {
                ResultV<int> result = transaction_for_check.ReadInt(
                    kPositionsForAtomicityTest[i * 2 + j]);
                ASSERT_TRUE(result.IsOk()) << result.Error();
                EXPECT_EQ(result.Get(), 0);
            }
        }
    }
}

// C: Tests if the transactions are consistent.

// I: Tests if the transactions are isolated.

const std::vector<disk::DiskPosition> kPositionsForIsolationTest = {
    disk::DiskPosition(disk::BlockID(data_filename, 0), 0),
    disk::DiskPosition(disk::BlockID(data_filename, 0), 7),
    disk::DiskPosition(disk::BlockID(data_filename, 2), 3),
    disk::DiskPosition(disk::BlockID(data_filename, 4), 8),
};

TRANSACTION_FUNCTION(IsolationTestFunc1, {
    std::vector<int> index(kPositionsForIsolationTest.size());
    std::iota(index.begin(), index.end(), 0);
    RandomShuffle(index);

    for (const int i : index) {
        Result write_result =
            transaction.Write(kPositionsForIsolationTest[i],
                              data::kTypeInt.ValueLength(), data::Int(1));
        if (write_result.IsError()) {
            std::cerr << "Transaction1 " << write_result.Error() << std::endl;
            return;
        }
    }

    Result result = transaction.Commit();
    if (result.IsError()) {
        std::cerr << "Transaction1 " << result.Error() << std::endl;
    }
})

TRANSACTION_FUNCTION(IsolationTestFunc2, {
    std::vector<int> index(kPositionsForIsolationTest.size());
    std::iota(index.begin(), index.end(), 0);
    RandomShuffle(index);

    for (const int i : index) {
        Result write_result =
            transaction.Write(kPositionsForIsolationTest[i],
                              data::kTypeInt.ValueLength(), data::Int(2));
        if (write_result.IsError()) {
            std::cerr << "Transaction2 " << write_result.Error() << std::endl;
            return;
        }
    }

    Result result = transaction.Commit();
    if (result.IsError()) {
        std::cerr << "Transaction2 " << result.Error() << std::endl;
    }
})

TRANSACTION_FUNCTION(IsolationTestFunc3, {
    std::vector<int> index(kPositionsForIsolationTest.size());
    std::iota(index.begin(), index.end(), 0);
    RandomShuffle(index);

    for (const int i : index) {
        Result write_result =
            transaction.Write(kPositionsForIsolationTest[i],
                              data::kTypeInt.ValueLength(), data::Int(3));
        if (write_result.IsError()) {
            std::cerr << "Transaction3 " << write_result.Error() << std::endl;
            return;
        }
    }

    Result result = transaction.Commit();
    if (result.IsError()) {
        std::cerr << "Transaction3 " << result.Error() << std::endl;
    }
})

TRANSACTION_FUNCTION(IsolationTestFunc4, {
    std::vector<int> index(kPositionsForIsolationTest.size());
    std::iota(index.begin(), index.end(), 0);
    RandomShuffle(index);

    for (const int i : index) {
        Result write_result =
            transaction.Write(kPositionsForIsolationTest[i],
                              data::kTypeInt.ValueLength(), data::Int(4));
        if (write_result.IsError()) {
            std::cerr << "Transaction4 " << write_result.Error() << std::endl;
            return;
        }
    }

    Result result = transaction.Commit();
    if (result.IsError()) {
        std::cerr << "Transaction4 " << result.Error() << std::endl;
    }
})

TEST_F(TransactionTest, IsolationTest) {
    // The test consists of four transactions. Each transaction writes four
    // integers to the disk to the same positions for all transactions.
    // Each transaction writes different values from other transactions to
    // the same positions. The test checks if the transactions are isolated,
    // i.e., if the transactions do not affect each other. The test checks if
    // all written data is the same.
    std::thread thread1(IsolationTestFunc1, std::ref(data_disk_manager),
                        std::ref(buffer_manager), std::ref(log_manager),
                        std::ref(lock_table), data_filename);
    std::thread thread2(IsolationTestFunc2, std::ref(data_disk_manager),
                        std::ref(buffer_manager), std::ref(log_manager),
                        std::ref(lock_table), data_filename);
    std::thread thread3(IsolationTestFunc3, std::ref(data_disk_manager),
                        std::ref(buffer_manager), std::ref(log_manager),
                        std::ref(lock_table), data_filename);
    std::thread thread4(IsolationTestFunc4, std::ref(data_disk_manager),
                        std::ref(buffer_manager), std::ref(log_manager),
                        std::ref(lock_table), data_filename);

    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();

    int expected = -1;
    for (const disk::DiskPosition &position : kPositionsForIsolationTest) {
        ResultV<int> result = transaction_for_check.ReadInt(position);
        ASSERT_TRUE(result.IsOk()) << result.Error();

        // Checks if all the values are the same.
        if (expected == -1) {
            expected = result.Get();
        } else {
            EXPECT_EQ(result.Get(), expected);
        }
    }
}

// D: Tests if the transactions are durable.