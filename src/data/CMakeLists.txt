enable_testing()
include(GoogleTest)

## byte
add_library(byte
  byte.cc
)
target_include_directories(byte
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)
target_link_libraries(byte
  data
)

add_executable(byte_test
  byte_test.cc
)
target_include_directories(byte_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src/data
)
target_link_libraries(byte_test
  byte
  GTest::gtest_main
)
gtest_discover_tests(byte_test)


## bytes
add_library(bytes
  bytes.cc
)
target_include_directories(bytes
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)

add_executable(bytes_test
  bytes_test.cc
)
target_include_directories(bytes_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src/data
)
target_link_libraries(bytes_test
  bytes
  GTest::gtest_main
)
gtest_discover_tests(bytes_test)

## char
add_library(char
  char.cc
)
target_include_directories(char
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)
target_link_libraries(char
  data
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

## data
add_library(data
  data.cc
)
target_include_directories(data
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)

add_executable(data_test
  data_test.cc
)
target_include_directories(data_test
  PUBLIC ${PROJECT_SOURCE_DIR}/src/data
)
target_link_libraries(data_test
  data
  GTest::gtest_main
)
gtest_discover_tests(data_test)

## int
add_library(int
  int.cc
)
target_include_directories(int
  PUBLIC ${PROJECT_SOURCE_DIR}/src
)
target_link_libraries(int
  data
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