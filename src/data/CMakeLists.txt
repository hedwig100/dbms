enable_testing()
include(GoogleTest)

## char
add_library(char
  char.cc
)
target_include_directories(char
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)

add_executable(char_test
  char_test.cc
)
target_include_directories(char_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src/data
)
target_link_libraries(char_test
  char
  GTest::gtest_main
)
gtest_discover_tests(char_test)

## int
add_library(int
  int.cc
)
target_include_directories(int
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)

add_executable(int_test
  int_test.cc
)
target_include_directories(int_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src/data
)
target_link_libraries(int_test
  int
  GTest::gtest_main
)
gtest_discover_tests(int_test)

## uint32
add_library(uint32
  uint32.cc
)
target_include_directories(uint32
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)

add_executable(uint32_test
  uint32_test.cc
)
target_include_directories(uint32_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src/data
)
target_link_libraries(uint32_test
  uint32
  GTest::gtest_main
)
gtest_discover_tests(uint32_test)