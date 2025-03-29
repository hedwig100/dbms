enable_testing()
include(GoogleTest)

## buffer
add_library(buffer
  buffer.cc
)
target_link_libraries(buffer
  disk
  log
)
target_include_directories(buffer
  PUBLIC ${PROJECT_SOURCE_DIR}/src
  PUBLIC ${PROJECT_SOURCE_DIR}/src/transaction
)

add_executable(buffer_test
  buffer_test.cc
)
target_include_directories(buffer_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src
  PUBLIC ${PROJECT_SOURCE_DIR}/src/transaction
)
target_link_libraries(buffer_test
  buffer
  GTest::gtest_main
)
gtest_discover_tests(buffer_test)

## checksum
add_library(checksum
  checksum.cc
)
target_include_directories(checksum
  PUBLIC ${PROJECT_SOURCE_DIR}/src
  PUBLIC ${PROJECT_SOURCE_DIR}/src/transaction
)

## concurrency
add_library(concurrency
  concurrency.cc
)
target_link_libraries(concurrency
  disk
)
target_include_directories(concurrency
  PUBLIC ${PROJECT_SOURCE_DIR}/src
  PUBLIC ${PROJECT_SOURCE_DIR}/src/transaction
)

add_executable(concurrency_test
  concurrency_test.cc
)
target_include_directories(concurrency_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src
  PUBLIC ${PROJECT_SOURCE_DIR}/src/transaction
)
target_link_libraries(concurrency_test
  concurrency
  GTest::gtest_main
)
gtest_discover_tests(concurrency_test)

## disk
add_library(disk
  disk.cc
)
target_link_libraries(disk
  byte
  char
  int
)
target_include_directories(disk
  PUBLIC ${PROJECT_SOURCE_DIR}/src
  PUBLIC ${PROJECT_SOURCE_DIR}/src/transaction
)

add_executable(disk_test
  disk_test.cc
)
target_include_directories(disk_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src
  PUBLIC ${PROJECT_SOURCE_DIR}/src/transaction
)
target_link_libraries(disk_test
  disk
  GTest::gtest_main
)
gtest_discover_tests(disk_test)

## log
add_library(log
  log.cc
)
target_link_libraries(log
  checksum
  disk
  log_record
)
target_include_directories(log
  PUBLIC ${PROJECT_SOURCE_DIR}/src
  PUBLIC ${PROJECT_SOURCE_DIR}/src/transaction
)

add_executable(log_test
  log_test.cc
)
target_include_directories(log_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src
  PUBLIC ${PROJECT_SOURCE_DIR}/src/transaction
)
target_link_libraries(log_test
  log
  GTest::gtest_main
)
gtest_discover_tests(log_test)

## log_record
add_library(log_record
  log_record.cc
)
target_link_libraries(log_record
  buffer
  bytes
  char
  checksum
  disk
  int 
  uint32
)
target_include_directories(log_record
  PUBLIC ${PROJECT_SOURCE_DIR}/src
  PUBLIC ${PROJECT_SOURCE_DIR}/src/transaction
)

add_executable(log_record_test
  log_record_test.cc
)
target_include_directories(log_record_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src
  PUBLIC ${PROJECT_SOURCE_DIR}/src/transaction
)
target_link_libraries(log_record_test
  log_record
  GTest::gtest_main
)
gtest_discover_tests(log_record_test)

## recovery
add_library(recovery
  recovery.cc
)
target_link_libraries(recovery
  log
  log_record
)
target_include_directories(recovery
  PUBLIC ${PROJECT_SOURCE_DIR}/src
  PUBLIC ${PROJECT_SOURCE_DIR}/src/transaction
)

add_executable(recovery_test
  recovery_test.cc
)
target_include_directories(recovery_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src
  PUBLIC ${PROJECT_SOURCE_DIR}/src/transaction
)
target_link_libraries(recovery_test
  recovery
  GTest::gtest_main
)
gtest_discover_tests(recovery_test)

## transaction
add_library(transaction
  transaction.cc
)
target_link_libraries(transaction
  buffer
  concurrency
  log 
  recovery
  result
)
target_include_directories(transaction
  PUBLIC ${PROJECT_SOURCE_DIR}/src
  PUBLIC ${PROJECT_SOURCE_DIR}/src/transaction
)

add_executable(transaction_test
  transaction_test.cc
)
target_include_directories(transaction_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src
  PUBLIC ${PROJECT_SOURCE_DIR}/src/transaction
)
target_link_libraries(transaction_test
  transaction
  GTest::gtest_main
)
gtest_discover_tests(transaction_test)
