# Fuzzing Guide

Binary Scene Engine includes six libFuzzer-compatible targets:

- `mesh_fuzzer`
- `scene_fuzzer`
- `serializer_fuzzer`
- `animation_fuzzer`
- `compression_fuzzer`
- `validator_fuzzer`

## Local Build

```sh
cmake -S . -B build-fuzz -DBSE_BUILD_FUZZERS=ON -DBSE_BUILD_TESTS=OFF -DCMAKE_CXX_COMPILER=clang++
cmake --build build-fuzz --parallel
```

Without libFuzzer, use the standalone driver to compile the same harness code and run corpus files
through it:

```sh
cmake -S . -B build-fuzz-standalone -DBSE_BUILD_FUZZERS=ON -DBSE_STANDALONE_FUZZERS=ON
cmake --build build-fuzz-standalone --parallel
./build-fuzz-standalone/scene_fuzzer fuzz/corpora/scene_fuzzer/*.bsen
```

## ClusterFuzzLite

ClusterFuzzLite uses `.clusterfuzzlite/build.sh`. The script builds all fuzzers and copies the
executables into `$OUT`, as required by CFLite.

Seed corpora live under `fuzz/corpora/<target>/`.

The harnesses exercise the parser, serializer, validator, compression codec, and scene model. Valid
serialized `.bsen` seeds are kept alongside malformed inputs so fuzzing can reach semantic checks
after header parsing.
