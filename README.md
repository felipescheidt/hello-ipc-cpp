# Hello IPC - cpp project

This is a simple cpp project that uses socket for Inter-Process Communication (IPC) to manage some LEDs state.

## Getting started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites:

The project is meant to run on a Lunix OS machine

- C++ Compiler - needs to support at least the C++17 standard
- CMake v3.15+
- Linux OS
- Google Test (libgtest-dev)

### Installing

First you need to clone the project repository:

```bash
git clone https://github.com/felipescheidt/hello-ipc-cpp.git
```

## Building the project

To build the project, please follow the steps below:

```bash
mkdir build/ && cd build/
cmake ..
cmake --build .
```
After the commands above you will be able to find the binary in build/ dir.

If you need to enable debug symbols:
```bash
mkdir build/ && cd build/
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

Unit tests:
```bash
mkdir build/ && cd build/
cmake -Dhello_ipc_ENABLE_UNIT_TESTING=ON ..
cmake --build .
ctest -V
```

For code coverage, after running tests:
```bash
gcovr -r .. --exclude ../src/main.cpp --exclude _deps
```

## Running the project

Once you have built the project, you'll be able to find the application's binary at build folder.

### Starting the socket server

In one terminal you can start the tcp socket as server:
```bash
cd build/
./hello_ipc --server
```

### Starting the hello_ipc services

Then you can run the services:
```bash
cd build/
./hello_ipc
```