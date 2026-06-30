# Binary Scene Engine

Binary Scene Engine is a small C++17 library and CLI for working with a compact binary scene
format. It is meant for tools that need to create, inspect, validate, and round-trip simple scene
assets without pulling in a large asset pipeline.

The current implementation includes:

- A CMake-based C++17 library.
- A custom little-endian binary scene format with versioned headers.
- Scene graph, mesh, material, texture, animation, skeleton, camera, and light data models.
- Parser, serializer, validator, diagnostics, compression, resource cache, CLI, tests, fuzzers, an
  example, and a lightweight benchmark.

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## CLI

```sh
./build/bsectl create-minimal minimal.bsen
./build/bsectl create-sample sample.bsen
./build/bsectl inspect minimal.bsen
./build/bsectl validate minimal.bsen
```
