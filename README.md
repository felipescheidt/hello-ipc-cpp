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

Or, if you need to enable debug symbols:
```bash
mkdir build/ && cd build/
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

Running unit tests:
```bash
mkdir build/ && cd build/
cmake -Dhello_ipc_ENABLE_UNIT_TESTING=ON ..
cmake --build .
ctest -V
```

For code coverage, after ctest running:
```bash
gcovr -r .. --exclude main.cpp
```

After the commands above you will be able to find the binary in build/ dir.