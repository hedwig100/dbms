enable_testing()
include(GoogleTest)

## sql
add_library(sql
    sql.cc
)
target_include_directories(sql
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(sql_test
  sql_test.cc
)
target_include_directories(sql_test
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(sql_test
  sql
  GTest::gtest_main
)
gtest_discover_tests(sql_test)