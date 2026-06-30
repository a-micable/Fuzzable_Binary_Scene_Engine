# Build Guide

## Requirements

- CMake 3.16 or newer.
- A C++17 compiler.

## Default Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Options

- `BSE_BUILD_TESTS=ON` builds unit and integration tests.
- `BSE_BUILD_FUZZERS=OFF` builds libFuzzer entry points when enabled. Fuzzer builds require
  ClusterFuzzLite's `LIB_FUZZING_ENGINE` environment variable or a Clang compiler that supports
  `-fsanitize=fuzzer`.
- `BSE_STANDALONE_FUZZERS=OFF` links the same harnesses with a small local corpus-file driver for
  offline smoke testing when libFuzzer is unavailable.
- `BSE_BUILD_EXAMPLES=ON` builds examples.
- `BSE_BUILD_BENCHMARKS=ON` builds lightweight benchmarks.

## Release Build

```sh
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --parallel
```

## Offline Build

The build uses only the checked-in source tree and the local C++ toolchain. CMake does not download
dependencies, read credentials, or require interactive input.
