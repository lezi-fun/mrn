cmake_minimum_required(VERSION 3.15)
project(mrn_compressor VERSION 2.0.0 LANGUAGES CXX)

# 核心选项
option(MRN_BUILD_PLUGINS "Build plugin system" ON)
option(MRN_BUILD_TESTS "Build tests" ON)
option(MRN_BUILD_BENCHMARKS "Build benchmarks" OFF)

# 编译器设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -DNDEBUG)
else()
    add_compile_options(-O0 -g -DDEBUG)
endif()

# 主可执行文件
add_executable(mrn
    src/main.cpp
    src/core/compressor.cpp
    src/core/plugin_manager.cpp
    src/core/archive_format.cpp
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

# 包含目录
target_include_directories(mrn PRIVATE 
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 依赖
find_package(Threads REQUIRED)
target_link_libraries(mrn PRIVATE Threads::Threads)

# 插件系统
if(MRN_BUILD_PLUGINS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/plugins/CMakeLists.txt")
    add_subdirectory(plugins)
endif()

# 安装
install(TARGETS mrn DESTINATION bin)
install(DIRECTORY include/ DESTINATION include/mrn)
