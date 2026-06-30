#!/usr/bin/env bash
set -eu

if [ -z "${OUT:-}" ]; then
  echo "OUT must point to the ClusterFuzzLite output directory" >&2
  exit 2
fi

mkdir -p "${OUT}"

cmake -S . -B build-cflite \
  -DCMAKE_CXX_COMPILER="${CXX:-clang++}" \
  -DCMAKE_C_COMPILER="${CC:-clang}" \
  -DBSE_BUILD_TESTS=OFF \
  -DBSE_BUILD_EXAMPLES=OFF \
  -DBSE_BUILD_BENCHMARKS=OFF \
  -DBSE_BUILD_FUZZERS=ON \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo

cmake --build build-cflite --parallel

for target in mesh_fuzzer scene_fuzzer serializer_fuzzer animation_fuzzer compression_fuzzer validator_fuzzer; do
  install -m 0755 "build-cflite/${target}" "${OUT}/${target}"
done
