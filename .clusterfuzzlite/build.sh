#!/usr/bin/env bash
set -euxo pipefail
: "${OUT:?OUT must be set}"
for target in mesh scene animation serializer compression validator; do
  ${CXX:-clang++} -std=c++17 -DBSE_LIBFUZZER -Iinclude -fsanitize=fuzzer,address,undefined fuzz/${target}_fuzzer.cc src/bse.cpp -o "${OUT}/${target}_fuzzer"
done
