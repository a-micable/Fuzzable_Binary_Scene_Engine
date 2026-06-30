cmake_minimum_required(VERSION 3.16)

project(BinarySceneEngine
  VERSION 0.1.0
  DESCRIPTION "A deterministic C++17 binary scene parsing and serialization engine"
  LANGUAGES CXX)

option(BSE_BUILD_TESTS "Build Binary Scene Engine tests" ON)
option(BSE_BUILD_FUZZERS "Build libFuzzer targets" OFF)
option(BSE_BUILD_EXAMPLES "Build examples" ON)
option(BSE_BUILD_BENCHMARKS "Build lightweight benchmarks" ON)
option(BSE_STANDALONE_FUZZERS "Build fuzz targets with a local corpus-file driver" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(binary_scene_engine
  src/bse/binary_reader.cpp
  src/bse/binary_writer.cpp
  src/bse/binary_inspector.cpp
  src/bse/animation_tools.cpp
  src/bse/asset_catalog.cpp
  src/bse/compression.cpp
  src/bse/corpus_tools.cpp
  src/bse/diagnostics.cpp
  src/bse/format.cpp
  src/bse/mesh_tools.cpp
  src/bse/obj_io.cpp
  src/bse/package_index.cpp
  src/bse/parser.cpp
  src/bse/resource_cache.cpp
  src/bse/scene.cpp
  src/bse/scene_builder.cpp
  src/bse/scene_collection.cpp
  src/bse/scene_diff.cpp
  src/bse/scene_graph.cpp
  src/bse/scene_lint.cpp
  src/bse/scene_manifest.cpp
  src/bse/scene_metrics.cpp
  src/bse/scene_mutator.cpp
  src/bse/scene_ops.cpp
  src/bse/scene_optimizer.cpp
  src/bse/scene_query.cpp
  src/bse/scene_report.cpp
  src/bse/scene_signature.cpp
  src/bse/scene_stats.cpp
  src/bse/serializer.cpp
  src/bse/text_scene.cpp
  src/bse/transform.cpp
  src/bse/validation_profile.cpp
  src/bse/validator.cpp)

target_include_directories(binary_scene_engine
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

target_compile_options(binary_scene_engine PRIVATE
  $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion>)

add_executable(bsectl tools/bsectl.cpp)
target_link_libraries(bsectl PRIVATE binary_scene_engine)

if(BSE_BUILD_EXAMPLES)
  add_executable(write_minimal_scene examples/write_minimal_scene.cpp)
  target_link_libraries(write_minimal_scene PRIVATE binary_scene_engine)
endif()

if(BSE_BUILD_TESTS)
  enable_testing()
  foreach(test_name
      test_binary_io
      test_compression
      test_scene_tools
      test_parser_serializer
      test_package_index
      test_resource_cache
      test_validation)
    add_executable(${test_name} tests/${test_name}.cpp)
    target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    target_link_libraries(${test_name} PRIVATE binary_scene_engine)
    add_test(NAME ${test_name} COMMAND ${test_name})
  endforeach()
  set(BSE_TEST_SCENE "${CMAKE_CURRENT_BINARY_DIR}/minimal.bsen")
  set(BSE_SAMPLE_SCENE "${CMAKE_CURRENT_BINARY_DIR}/sample.bsen")
  add_test(NAME cli_create_minimal
    COMMAND $<TARGET_FILE:bsectl> create-minimal "${BSE_TEST_SCENE}")
  add_test(NAME cli_create_sample
    COMMAND $<TARGET_FILE:bsectl> create-sample "${BSE_SAMPLE_SCENE}")
  add_test(NAME cli_validate_minimal
    COMMAND $<TARGET_FILE:bsectl> validate "${BSE_TEST_SCENE}")
  add_test(NAME cli_inspect_minimal
    COMMAND $<TARGET_FILE:bsectl> inspect "${BSE_TEST_SCENE}")
  add_test(NAME cli_validate_sample
    COMMAND $<TARGET_FILE:bsectl> validate "${BSE_SAMPLE_SCENE}")
  add_test(NAME cli_inspect_sample
    COMMAND $<TARGET_FILE:bsectl> inspect "${BSE_SAMPLE_SCENE}")
  add_test(NAME cli_rejects_missing_file
    COMMAND $<TARGET_FILE:bsectl> inspect "${CMAKE_CURRENT_BINARY_DIR}/missing.bsen")
  set_tests_properties(cli_validate_minimal cli_inspect_minimal
    PROPERTIES DEPENDS cli_create_minimal)
  set_tests_properties(cli_validate_sample cli_inspect_sample
    PROPERTIES DEPENDS cli_create_sample)
  set_tests_properties(cli_rejects_missing_file PROPERTIES WILL_FAIL TRUE)
endif()

if(BSE_BUILD_FUZZERS)
  set(BSE_LIB_FUZZING_ENGINE "$ENV{LIB_FUZZING_ENGINE}" CACHE STRING
      "Fuzzing engine link flags or archive, usually provided by ClusterFuzzLite")
  if(NOT BSE_LIB_FUZZING_ENGINE AND NOT BSE_STANDALONE_FUZZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(BSE_LIB_FUZZING_ENGINE "-fsanitize=fuzzer")
  endif()
  if(NOT BSE_LIB_FUZZING_ENGINE AND NOT BSE_STANDALONE_FUZZERS)
    message(FATAL_ERROR
      "BSE_BUILD_FUZZERS requires LIB_FUZZING_ENGINE or a Clang compiler with -fsanitize=fuzzer")
  endif()
  foreach(target_name
      mesh_fuzzer
      scene_fuzzer
      serializer_fuzzer
      animation_fuzzer
      compression_fuzzer
      validator_fuzzer)
    if(BSE_STANDALONE_FUZZERS)
      add_executable(${target_name} fuzz/${target_name}.cpp fuzz/standalone_driver.cpp)
    else()
      add_executable(${target_name} fuzz/${target_name}.cpp)
    endif()
    target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/fuzz)
    target_link_libraries(${target_name} PRIVATE binary_scene_engine)
    if(BSE_LIB_FUZZING_ENGINE)
      target_link_options(${target_name} PRIVATE ${BSE_LIB_FUZZING_ENGINE})
    endif()
  endforeach()
endif()

if(BSE_BUILD_BENCHMARKS)
  add_executable(bse_roundtrip_benchmark benchmarks/roundtrip_benchmark.cpp)
  target_link_libraries(bse_roundtrip_benchmark PRIVATE binary_scene_engine)
endif()
