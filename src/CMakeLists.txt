enable_testing()
include(GoogleTest)

## main
add_executable(main
    main.cc
)

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

