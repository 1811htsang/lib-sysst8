# lib-sysst8

Lib-sysst8 (lib68) is a lightweight C library for managing system state machine and fault conditions in embedded systems. It provides a structured approach to handle errors, log messages, and perform actions based on the severity of the fault.

## Features

- Fault Condition Reporting (FCR) system with severity levels and actions
- Finite State Machine (FSM) support with function pointers for state handling
- Configurable message types and user-defined signals
- Support for different platforms (e.g., Linux, embedded systems) with blank stubs for specific platform functions
- Easy integration into existing C projects

## Getting Started

### Prerequisites

- CMake 3.16 or higher
- A C compiler (e.g., GCC, Clang)
- Optional: Platform-specific dependencies for embedded systems

### Building the Library

- Clone the repository:

```bash
git clone https://github.com/1811htsang/lib-sysst8.git lib-sysst8
```

- Navigate to the project directory:

```bash
cd lib-sysst8
```

- Build the library using CMake:

```bash
cmake -S . -B build
make -C build
```

## Integration

- Clone the repository with folder named `lib-sysst8` into your project directory.
- Include these header files in your build system:

```c
#include "sysst8_conf.h"
#include "sysst8_fcr.h"
#include "sysst8_fsm.h"
```

- Add the source files to your build system:

```c
src/sysst8_fcr.c
src/sysst8_fsm.c
```

- Link against the compiled library if you built it as a shared or static library.
- When including in your project, ensure that the include paths are set correctly to find the `lib-sysst8/inc` directory.
