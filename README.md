# Hello IPC - cpp project

This is a simple cpp project that uses socket for Inter-Process Communication (IPC) to manage some LEDs state.

## Getting started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites:

The project is meant to run on a Lunix OS machine

- C++ Compiler - needs to support at least the C++17 standard
- CMake v3.15+
- Linux OS

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