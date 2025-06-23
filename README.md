# Hello IPC - cpp project

This project is a C++ implementation of an inter-process communication (IPC) system for controlling and querying LED states.

It uses UNIX domain sockets for communication between a server (which manages LED states) and multiple clients (which can update or query those states).

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

For test and code coverage:
```bash
mkdir build/ && cd build/
cmake -DCMAKE_BUILD_TYPE=Debug -Dhello_ipc_ENABLE_UNIT_TESTING=ON -Dhello_ipc_ENABLE_CODE_COVERAGE=ON ..
cmake --build . && cmake --build . --target coverage
```

## Running the project

Once you have built the project, you'll be able to find the application's binary at build folder.

### Starting the LedManager service

In terminal 1 you need to start the LedManager service (socket: server mode):
```bash
cd build/
./hello_ipc --led-manager
```

### Starting the UpdateLed service:

In terminal 2 you can start the UpdateLed service (socket: client mode):
```bash
cd build/
./hello_ipc --update-led
```
Or
```bash
cd build/
./hello_ipc --update-led --led1 --led2 --led3
```

### Starting the QueryLed service (optional):

In terminal 3 you can start the QueryLed service (socket: client mode):
```bash
cd build/
./hello_ipc --query-led
```
