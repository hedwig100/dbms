enable_testing()
include(GoogleTest)

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

## disk
add_library(disk
  disk.cc
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
