enable_testing()
include(GoogleTest)

add_subdirectory(data)

## main
add_executable(main
    main.cc
)

## buffer
add_library(buffer
  buffer.cc
)
target_link_libraries(buffer
  disk
)
target_include_directories(buffer
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)

add_executable(buffer_test
  buffer_test.cc
)
target_include_directories(buffer_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)
target_link_libraries(buffer_test
  buffer
  GTest::gtest_main
)
gtest_discover_tests(buffer_test)

## concurrency
add_library(concurrency
  concurrency.cc
)
target_link_libraries(concurrency
  disk
)
target_include_directories(concurrency
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)

add_executable(concurrency_test
  concurrency_test.cc
)
target_include_directories(concurrency_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src
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
  char
  data_read
  int
)
target_include_directories(disk
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)

add_executable(disk_test
  disk_test.cc
)
target_include_directories(disk_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src
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
  disk
  log_record
)
target_include_directories(log
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)

add_executable(log_test
  log_test.cc
)
target_include_directories(log_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src
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
  char
  data_read
  disk
  int 
  uint32
)
target_include_directories(log_record
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)

add_executable(log_record_test
  log_record_test.cc
)
target_include_directories(log_record_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)
target_link_libraries(log_record_test
  log_record
  GTest::gtest_main
)
gtest_discover_tests(log_record_test)

## macro_test
add_library(macro_test
  INTERFACE macro_test.h
)
target_include_directories(macro_test
  INTERFACE ${PROJECT_SOURCE_DIR}/src
)

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
)

add_executable(recovery_test
  recovery_test.cc
)
target_include_directories(recovery_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)
target_link_libraries(recovery_test
  recovery
  GTest::gtest_main
)
gtest_discover_tests(recovery_test)

## result
add_library(result
  INTERFACE result.h
)
target_include_directories(result
  INTERFACE ${PROJECT_SOURCE_DIR}/src
)

add_executable(result_test
  result_test.cc
)
target_include_directories(result_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)
target_link_libraries(result_test
  result
  GTest::gtest_main
)
gtest_discover_tests(result_test)

## sample_lib
add_library(sample_lib
  sample_lib.cc
)
target_include_directories(sample_lib
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)

add_executable(sample_lib_test
  sample_lib_test.cc
)
target_include_directories(sample_lib_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)
target_link_libraries(sample_lib_test
  sample_lib
  GTest::gtest_main
)
gtest_discover_tests(sample_lib_test)
