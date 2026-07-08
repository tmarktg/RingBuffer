# RingBuffer

A header-only, generic, RAII-managed circular buffer in modern C++, with a Catch2 test suite and a small demo showing wraparound and overwrite behavior.

See [`Project.md`](Project.md) for the design rationale (storage choice, empty-vs-full handling, overwrite policy, rule of five).

## Layout

```
RingBuffer/
├── include/ring_buffer.hpp   # the RingBuffer<T> template (header-only)
├── src/main.cpp              # demo: push past capacity, show overwrite + FIFO drain
├── tests/test_ring_buffer.cpp # Catch2 test suite
└── CMakeLists.txt
```

## Requirements

- CMake >= 3.14
- A C++17 compiler (AppleClang, GCC, or MSVC)
- Internet access on first configure (CMake's `FetchContent` pulls Catch2 v3)

## Build

```sh
cmake -S . -B build
cmake --build build
```

On Windows, this project can also be opened directly in Visual Studio as a CMake project (File > Open > Folder).

## Run the tests

```sh
ctest --test-dir build --output-on-failure
```

## Run the demo

```sh
./build/ring_buffer_demo
```

Pushes 6 ints into a capacity-4 buffer to show the oldest elements being overwritten, then drains it to show correct FIFO ordering.
