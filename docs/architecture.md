# Architecture Overview

Binary Scene Engine is organized around a small set of stable boundaries:

- `include/bse/scene.hpp` defines the public scene data model.
- `binary_reader` and `binary_writer` provide deterministic little-endian primitives.
- `parser` and `serializer` translate between scene objects and the binary format.
- `validator` checks object identity, cross references, and configurable resource limits.
- `diagnostics` collects structured validation messages for tools and tests.
- `compression` contains byte-level codecs used by future section compression.
- `resource_cache` provides a bounded least-recently-used cache for external resources.

The library keeps IO policy outside the core API. Callers provide byte buffers and decide how
files, memory maps, package archives, or network resources are managed.

## Determinism

The format is little-endian and stores explicit counts before variable-length collections.
Serialization writes metadata keys in sorted order, builds are configured with
`CMAKE_CXX_EXTENSIONS=OFF`, and all public targets use C++17.

## Evolution

The current format version is `1.0`. Major-version changes may break compatibility. Minor-version
changes must be parseable by the current major parser or rejected with a structured status.
