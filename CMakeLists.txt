cmake_minimum_required(VERSION 3.15)
project(mrn_compressor VERSION 2.0.0 LANGUAGES CXX)

option(MRN_BUILD_PLUGINS "Build plugin system" ON)
option(MRN_BUILD_TESTS "Build tests" ON)
option(MRN_BUILD_BENCHMARKS "Build benchmarks" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -DNDEBUG)
else()
    add_compile_options(-O0 -g -DDEBUG)
endif()

add_executable(mrn
    src/main.cpp
    src/core/compressor.cpp
    src/core/plugin_manager.cpp
    src/core/archive_format.cpp
    src/core/config.cpp
    src/algorithms/move_optimizer.cpp
    src/algorithms/lz77_compressor.cpp
    src/algorithms/huffman_encoder.cpp
    src/algorithms/algorithm_registry.cpp
    src/io/file_io.cpp
    src/io/directory_scanner.cpp
    src/io/archive_writer.cpp
    src/utils/thread_pool.cpp
    src/utils/progress_tracker.cpp
    src/utils/logger.cpp
)

target_include_directories(mrn PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
target_link_libraries(mrn PRIVATE Threads::Threads ZLIB::ZLIB)

if(MRN_BUILD_PLUGINS)
    add_subdirectory(plugins)
endif()

install(TARGETS mrn DESTINATION bin)
install(DIRECTORY include/ DESTINATION include/mrn)
