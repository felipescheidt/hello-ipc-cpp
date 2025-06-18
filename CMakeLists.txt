cmake_minimum_required(VERSION 3.15)

project(
  "hello_ipc"
  VERSION 0.0.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create a library for the hello_ipc code
add_library(hello_ipc_lib
  STATIC
  src/hello-ipc/Service.cpp
  src/hello-ipc/LedManager.cpp
  src/hello-ipc/UpdateLed.cpp
  src/hello-ipc/Logger.cpp
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

option(${PROJECT_NAME}_ENABLE_UNIT_TESTING "Enable unit tests" ON) # Unit tests enabled by default

if(${PROJECT_NAME}_ENABLE_UNIT_TESTING)
  enable_testing()
  message(STATUS "Build unit tests for the project. Tests should always be found in the test folder\n")
  add_subdirectory(test)
endif()