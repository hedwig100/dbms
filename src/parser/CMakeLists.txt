enable_testing()
include(GoogleTest)

BISON_TARGET(bison_parser
    bison.y
    ${CMAKE_CURRENT_BINARY_DIR}/bison.cc
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/bison.h
)

FLEX_TARGET(flex_lexer
    flex.l
    ${CMAKE_CURRENT_BINARY_DIR}/flex.cc
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/flex.h
)

ADD_FLEX_BISON_DEPENDENCY(flex_lexer bison_parser)

## sql
add_library(sql
    sql.cc
)
target_include_directories(sql
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

## parser
add_library(parser
    parser.cc
    ${CMAKE_CURRENT_BINARY_DIR}/bison.cc
    ${CMAKE_CURRENT_BINARY_DIR}/flex.cc
)
target_include_directories(parser
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${PROJECT_SOURCE_DIR}/src
)
target_link_libraries(parser
    sql
)

add_executable(parser_test
  parser_test.cc
)
target_include_directories(parser_test
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(parser_test
  parser
  GTest::gtest_main
)
gtest_discover_tests(parser_test)