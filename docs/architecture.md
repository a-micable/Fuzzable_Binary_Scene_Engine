# Architecture Overview

Subsystems are organized around parsing, scene representation, serialization, validation, caching, compression, logging, and utilities. Parser stages are exposed as individual functions for tests and fuzzing.

Binary Scene Engine is organized around a small set of stable boundaries:

- `include/bse/binary_scene_engine.hpp` defines the primary public scene data model.
- `binary_reader` and `binary_writer` provide deterministic byte-buffer primitives.
- `parser` and `serializer` translate between scene objects and the binary format.
- `validator` checks object identity, cross references, and configurable resource limits.
- `diagnostics` collects structured validation messages for tools and tests.
- `compression` contains byte-level codecs used by future section compression.
- `resource_cache` provides a bounded least-recently-used cache for external resources.
- `catalog` exposes a queryable rule-profile catalog for validation and policy tooling.

The library keeps IO policy outside the core API. Callers provide byte buffers and decide how
files, memory maps, package archives, or network resources are managed.

## Determinism

The format is little-endian and stores explicit counts before variable-length collections.
Serialization writes metadata keys in sorted order, builds are configured with
`CMAKE_CXX_EXTENSIONS=OFF`, and all public targets use C++17.

## Evolution

The current format version is `1.0`. Major-version changes may break compatibility. Minor-version
changes must be parseable by the current major parser or rejected with a structured status.
