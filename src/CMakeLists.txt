enable_testing()
include(GoogleTest)

add_subdirectory(data)
add_subdirectory(execute)
add_subdirectory(parser)
add_subdirectory(transaction)

## main
add_executable(main
    main.cc
)

## macro_test
add_library(macro_test
  INTERFACE macro_test.h
)
target_include_directories(macro_test
  INTERFACE ${PROJECT_SOURCE_DIR}/src
)

## metadata
add_library(metadata
  metadata.cc
)
target_link_libraries(metadata
  schema
  table_scan
  transaction
)
target_include_directories(metadata
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)

add_executable(metadata_test
  metadata_test.cc
)
target_include_directories(metadata_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)
target_link_libraries(metadata_test
  char 
  int
  metadata
  GTest::gtest_main
)
gtest_discover_tests(metadata_test)

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

## scans
add_library(scans
  scans.cc
)
target_link_libraries(scans
  table_scan
)
target_include_directories(scans
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)

add_executable(scans_test
  scans_test.cc
)
target_include_directories(scans_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)
target_link_libraries(scans_test
  GTest::gtest_main
  int
  scans
)
gtest_discover_tests(scans_test)


## schema
add_library(schema
  schema.cc
)
target_include_directories(schema
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)

add_executable(schema_test
  schema_test.cc
)
target_include_directories(schema_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)
target_link_libraries(schema_test
  schema
  GTest::gtest_main
  GTest::gmock_main
)
gtest_discover_tests(schema_test)

## table_scan
add_library(table_scan
  table_scan.cc
)
target_link_libraries(table_scan
  byte 
  disk
  schema
  transaction
)
target_include_directories(table_scan
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)

add_executable(table_scan_test
  table_scan_test.cc
)
target_include_directories(table_scan_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)
target_link_libraries(table_scan_test
  int
  table_scan
  GTest::gtest_main
)
gtest_discover_tests(table_scan_test)
