cmake_minimum_required(VERSION 3.15)
project(mrn_compressor VERSION 2.0.0 LANGUAGES CXX)

# 核心选项
option(MRN_BUILD_PLUGINS "Build plugin system" ON)
option(MRN_BUILD_TESTS "Build tests" ON)
option(MRN_BUILD_BENCHMARKS "Build benchmarks" OFF)

# 编译器设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 检测目标架构
if(CMAKE_SYSTEM_PROCESSOR)
    message(STATUS "Target architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# 平台特定的编译选项
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Building for Linux")
    add_compile_options(-Wall -Wextra -Wpedantic)
    
    # ARM64 特定优化
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        message(STATUS "Building for ARM64 architecture")
        add_compile_options(-mcpu=native)
    endif()
    
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "Building for macOS")
    add_compile_options(-Wall -Wextra -Wpedantic)
    
    # Apple Silicon 特定设置
    if(CMAKE_OSX_ARCHITECTURES MATCHES "arm64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        message(STATUS "Building for Apple Silicon (ARM64)")
    elseif(CMAKE_OSX_ARCHITECTURES MATCHES "x86_64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        message(STATUS "Building for Intel x86_64")
    endif()
    
    # macOS 通用设置
    add_compile_options(-stdlib=libc++)
endif()

# 优化设置
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -DNDEBUG)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        add_compile_options(-march=native)
    endif()
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

# macOS 特定链接库
if(APPLE)
    find_library(CORE_FOUNDATION CoreFoundation)
    if(CORE_FOUNDATION)
        target_link_libraries(mrn PRIVATE ${CORE_FOUNDATION})
    endif()
endif()

# 插件系统
if(MRN_BUILD_PLUGINS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/plugins/CMakeLists.txt")
    add_subdirectory(plugins)
endif()

# 安装
install(TARGETS mrn DESTINATION bin)
install(DIRECTORY include/ DESTINATION include/mrn)
