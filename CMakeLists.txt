cmake_minimum_required(VERSION 3.16)
project(BinarySceneEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BSE_BUILD_TESTS "Build unit tests" ON)
option(BSE_BUILD_EXAMPLES "Build examples" ON)
option(BSE_BUILD_FUZZERS "Build fuzz targets" OFF)

file(GLOB_RECURSE BSE_MODULE_SOURCES CONFIGURE_DEPENDS
    src/*/*.cpp
)

add_library(binary_scene_engine
    src/bse.cpp
    ${BSE_MODULE_SOURCES}
)

target_include_directories(binary_scene_engine PUBLIC include)
target_compile_features(binary_scene_engine PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(binary_scene_engine PRIVATE /W4)
else()
    target_compile_options(binary_scene_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BSE_BUILD_TESTS)
    enable_testing()
    add_executable(bse_tests tests/test_main.cpp)
    target_link_libraries(bse_tests PRIVATE binary_scene_engine)
    add_test(NAME bse_tests COMMAND bse_tests)
endif()

if(BSE_BUILD_EXAMPLES)
    add_executable(bse_seedgen tools/generate_seed_corpus.cpp)
    target_link_libraries(bse_seedgen PRIVATE binary_scene_engine)
    add_executable(bse_dump examples/dump_scene.cpp)
    target_link_libraries(bse_dump PRIVATE binary_scene_engine)
    add_executable(bse_roundtrip examples/roundtrip.cpp)
    target_link_libraries(bse_roundtrip PRIVATE binary_scene_engine)
    add_executable(bse_metrics tools/metrics.cpp)
    target_link_libraries(bse_metrics PRIVATE binary_scene_engine)
endif()

if(BSE_BUILD_FUZZERS)
    foreach(target mesh scene animation serializer compression validator)
        add_executable(${target}_fuzzer fuzz/${target}_fuzzer.cc)
        target_link_libraries(${target}_fuzzer PRIVATE binary_scene_engine)
    endforeach()
endif()
