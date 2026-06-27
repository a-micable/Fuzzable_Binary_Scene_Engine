# Binary Scene Engine

Binary Scene Engine is a private C++17 library for parsing, validating, manipulating, and exporting a custom binary 3D scene format. It includes a staged parser, scene graph, meshes, materials, texture metadata, animation channels, skeleton data types, cameras, lights, serialization, compression, validation, resource caching, logging, examples, tests, fuzz targets, and ClusterFuzzLite configuration.

## Build

```bash
cmake -S . -B build -DBSE_BUILD_TESTS=ON -DBSE_BUILD_EXAMPLES=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Format

Files start with a `BSEN` header, version, section table offsets, object/string table offsets, file size, checksum, and flags. Sections are independently bounded and may be compressed with the project RLE codec. Scene objects reference each other by stable 32-bit IDs.
