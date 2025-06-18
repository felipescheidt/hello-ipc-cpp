cmake_minimum_required(VERSION 3.15)

project(
  ${CMAKE_PROJECT_NAME}Tests
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB test_sources "src/hello-ipc/*.cpp")

if(NOT test_sources)
  message(WARNING "No test source files found in test/src/hello-ipc/. No tests will be built.")
endif()

message(STATUS "Adding tests under ${CMAKE_PROJECT_NAME}...") # CMAKE_PROJECT_NAME is hello_ipc-Tests here

foreach(file ${test_sources})
  string(REGEX REPLACE "(.*/)([a-zA-Z0-9_ ]+)(\.cpp)" "\\2" test_name ${file}) 
  add_executable(${test_name}_Test ${file})

  # Add include directory for hello_ipc headers
  target_include_directories(${test_name}_Test PUBLIC
    ${CMAKE_SOURCE_DIR}/include # CMAKE_SOURCE_DIR is the top-level project source dir
  )

  target_compile_features(${test_name}_Test PUBLIC cxx_std_17)

  if(hello_ipc_ENABLE_CODE_COVERAGE)
    target_compile_options(hello_ipc_lib PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
    target_link_options(hello_ipc_lib PUBLIC -fprofile-arcs -ftest-coverage)
  endif()

  target_link_libraries(
    ${test_name}_Test
    PUBLIC
      hello_ipc_lib
      GTest::gtest
      GTest::gmock
      GTest::gtest_main
  )

  add_test(
    NAME ${test_name}
    COMMAND ${test_name}_Test
  )
endforeach()

message(STATUS "Finished adding unit tests for ${CMAKE_PROJECT_NAME}.")