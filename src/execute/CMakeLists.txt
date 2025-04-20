enable_testing()
include(GoogleTest)

## execute
add_library(execute
    execute.cc
)
target_link_libraries(execute
    parser
    transaction
)
target_include_directories(execute
    PUBLIC ${PROJECT_SOURCE_DIR}/src
    PRIVATE ${CMAKE_BINARY_DIR}/src/parser ## for bison
)

add_executable(execute_test
  execute_test.cc
)
target_include_directories(execute_test
    PUBLIC ${PROJECT_SOURCE_DIR}/src
)
target_link_libraries(execute_test
  execute
  GTest::gtest_main
)
gtest_discover_tests(execute_test)

## sql
add_library(sql
    sql.cc
)
target_link_libraries(sql
  metadata
  scans
  table_scan
  transaction
)
target_include_directories(sql
    PUBLIC ${PROJECT_SOURCE_DIR}/src
)

add_executable(sql_test
  sql_test.cc
)
target_include_directories(sql_test
    PUBLIC ${PROJECT_SOURCE_DIR}/src
)
target_link_libraries(sql_test
  sql
  table_scan
  GTest::gtest_main
)
gtest_discover_tests(sql_test)