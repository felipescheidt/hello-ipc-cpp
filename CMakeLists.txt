cmake_minimum_required(VERSION 3.15)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

project(
  "hello_ipc"
  VERSION 0.0.1
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create a library for the hello_ipc code
add_library(hello_ipc_lib
  SHARED
  src/hello-ipc/Service.cpp
  src/hello-ipc/LedManager.cpp
  src/hello-ipc/UpdateLed.cpp
  src/hello-ipc/Logger.cpp
  src/hello-ipc/QueryLed.cpp
)

# Ensure the library and anything linking to it can find the headers
target_include_directories(hello_ipc_lib 
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include/hello-ipc
)

# Your main executable
add_executable(hello_ipc
  src/main.cpp
)

# Find the threads library
find_package(Threads REQUIRED)

# Link the main executable against the hello_ipc library
target_link_libraries(hello_ipc PRIVATE hello_ipc_lib)

#enable c++ compiler warnings
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(hello_ipc PRIVATE -Wall -Wextra -pedantic)
  target_compile_options(hello_ipc_lib PRIVATE -Wall -Wextra -pedantic) # Also apply to library
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  target_compile_options(hello_ipc PRIVATE /W4)
  target_compile_options(hello_ipc_lib PRIVATE /W4)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
  target_compile_options(hello_ipc PRIVATE -Wall -Wextra -pedantic)
  target_compile_options(hello_ipc_lib PRIVATE -Wall -Wextra -pedantic)
else()
  message(WARNING "Unknown compiler, no warnings enabled")
endif()

# Compiling with debug symbols
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Compiling with debug symbols")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
else()
  message(STATUS "Compiling without debug symbols")
  #
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

#
# Unit testing setup
#

# --- Google Test Integration (MUST BE IN ROOT CMAKE) ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For projects with C++11 or newer, set this to avoid a CMake warning
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if(${PROJECT_NAME}_ENABLE_UNIT_TESTING)
  enable_testing()
  message(STATUS "Build unit tests for the project. Tests should always be found in the test folder\n")
  add_subdirectory(test)
endif()

#
# Custom target for code coverage
#

if(${PROJECT_NAME}_ENABLE_CODE_COVERAGE)
  find_program(GCOVR_EXECUTABLE gcovr)

  if(NOT GCOVR_EXECUTABLE)
    message(FATAL_ERROR "gcovr not found! Please install it to use the coverage target.")
  endif()

  add_custom_target(coverage
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    COMMAND ${GCOVR_EXECUTABLE}
            -r ${CMAKE_SOURCE_DIR}
            --exclude ${CMAKE_SOURCE_DIR}/src/main.cpp
            --exclude _deps
            --exclude test
    COMMAND ${GCOVR_EXECUTABLE}
            -r ${CMAKE_SOURCE_DIR}
            --exclude ${CMAKE_SOURCE_DIR}/src/main.cpp
            --exclude _deps
            --exclude test
            --html --html-details -o coverage.html
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running tests and generating code coverage report (console + HTML)"
  )
endif()