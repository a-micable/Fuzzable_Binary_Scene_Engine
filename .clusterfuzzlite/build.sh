#!/usr/bin/env bash
set -eu

if [ -z "${OUT:-}" ]; then
  echo "OUT must point to the ClusterFuzzLite output directory" >&2
  exit 2
fi

mkdir -p "${OUT}"

BUILD_DIR="${BUILD_DIR:-/tmp/bse-cflite-build}"
STANDALONE_FUZZERS=OFF
if [ -z "${LIB_FUZZING_ENGINE:-}" ]; then
  STANDALONE_FUZZERS=ON
fi

cmake -S . -B "${BUILD_DIR}" \
  -DCMAKE_CXX_COMPILER="${CXX:-c++}" \
  -DCMAKE_C_COMPILER="${CC:-cc}" \
  -DBSE_BUILD_TESTS=OFF \
  -DBSE_BUILD_EXAMPLES=OFF \
  -DBSE_BUILD_BENCHMARKS=OFF \
  -DBSE_BUILD_FUZZERS=ON \
  -DBSE_STANDALONE_FUZZERS="${STANDALONE_FUZZERS}" \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo

cmake --build "${BUILD_DIR}" --parallel

for target in mesh_fuzzer scene_fuzzer serializer_fuzzer animation_fuzzer compression_fuzzer validator_fuzzer; do
  install -m 0755 "${BUILD_DIR}/${target}" "${OUT}/${target}"
done
