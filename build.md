# MRN压缩工具开发文档

## 项目概述

**项目名称**: MRN压缩工具  
**算法核心**: MoveRun压缩算法 + 插件化架构  
**文件扩展名**: `.mrn`  
**文件头标识**: "MRN" (3字节魔数)

## 项目结构 (插件化设计)

```
mrn-compressor/
├── CMakeLists.txt
├── include/
│   ├── core/
│   │   ├── compressor.h
│   │   ├── plugin_interface.h
│   │   ├── plugin_manager.h
│   │   ├── archive_format.h
│   │   └── config.h
│   ├── algorithms/
│   │   ├── move_optimizer.h
│   │   ├── lz77_compressor.h
│   │   ├── huffman_encoder.h
│   │   └── algorithm_base.h
│   ├── io/
│   │   ├── file_io.h
│   │   ├── directory_scanner.h
│   │   └── archive_writer.h
│   └── utils/
│       ├── thread_pool.h
│       ├── progress_tracker.h
│       └── logger.h
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── compressor.cpp
│   │   ├── plugin_manager.cpp
│   │   └── archive_format.cpp
│   ├── algorithms/
│   │   ├── move_optimizer.cpp
│   │   ├── lz77_compressor.cpp
│   │   ├── huffman_encoder.cpp
│   │   └── algorithm_registry.cpp
│   ├── io/
│   │   ├── file_io.cpp
│   │   ├── directory_scanner.cpp
│   │   └── archive_writer.cpp
│   └── utils/
│       ├── thread_pool.cpp
│       ├── progress_tracker.cpp
│       └── logger.cpp
├── plugins/
│   ├── CMakeLists.txt
│   ├── entropy_coders/
│   │   ├── arithmetic_coder/
│   │   └── range_coder/
│   ├── preprocessors/
│   │   ├── burrows_wheeler/
│   │   └── delta_encoder/
│   └── specialized/
│       ├── image_compressor/
│       └── text_compressor/
├── tests/
└── docs/
```

## 核心架构设计

### 1. 插件接口系统 (plugin_interface.h)

```cpp
// 压缩算法插件接口
class ICompressionAlgorithm {
public:
    virtual ~ICompressionAlgorithm() = default;
    
    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    virtual uint32_t getAlgorithmId() const = 0;
    
    // 压缩接口
    virtual CompressionResult compress(const CompressParams& params, 
                                      const std::vector<uint8_t>& data) = 0;
    
    // 解压接口  
    virtual DecompressionResult decompress(const DecompressParams& params,
                                          const std::vector<uint8_t>& data) = 0;
    
    // 算法能力描述
    virtual AlgorithmCapabilities getCapabilities() const = 0;
    
    // 配置接口
    virtual void configure(const AlgorithmConfig& config) = 0;
};

// 预处理器插件接口
class IPreprocessor {
public:
    virtual ~IPreprocessor() = default;
    virtual std::vector<uint8_t> process(const std::vector<uint8_t>& data) = 0;
    virtual std::vector<uint8_t> inverseProcess(const std::vector<uint8_t>& data) = 0;
};
```

### 2. 插件管理器 (plugin_manager.h)

```cpp
class PluginManager {
public:
    static PluginManager& getInstance();
    
    // 插件注册
    bool registerAlgorithm(const std::string& type, 
                          std::unique_ptr<ICompressionAlgorithm> algorithm);
    bool registerPreprocessor(const std::string& name,
                             std::unique_ptr<IPreprocessor> preprocessor);
    
    // 插件发现
    std::vector<std::string> getAvailableAlgorithms() const;
    std::vector<std::string> getAvailablePreprocessors() const;
    
    // 插件获取
    ICompressionAlgorithm* getAlgorithm(const std::string& name);
    IPreprocessor* getPreprocessor(const std::string& name);
    
    // 动态加载
    void loadPluginsFromDirectory(const std::string& directory);
    
private:
    std::map<std::string, std::unique_ptr<ICompressionAlgorithm>> algorithms_;
    std::map<std::string, std::unique_ptr<IPreprocessor>> preprocessors_;
};
```

### 3. 压缩流水线架构 (compressor.h)

```cpp
struct CompressionPipeline {
    std::vector<std::string> preprocessors;  // 预处理器链
    std::string mainAlgorithm;               // 主压缩算法
    std::map<std::string, AlgorithmConfig> algorithmConfigs; // 算法配置
};

class ModularCompressor {
public:
    ModularCompressor();
    
    // 文件夹压缩
    CompressionResult compressDirectory(const std::string& inputDir,
                                       const std::string& outputFile,
                                       const CompressionPipeline& pipeline,
                                       const CompressionOptions& options);
    
    // 文件压缩
    CompressionResult compressFile(const std::string& inputFile,
                                  const std::string& outputFile, 
                                  const CompressionPipeline& pipeline,
                                  const CompressionOptions& options);
    
    // 解压
    DecompressionResult decompress(const std::string& inputFile,
                                  const std::string& outputPath);
    
    // 流水线构建
    void setDefaultPipeline(const std::string& preset); // "text", "binary", "maximum"
    CompressionPipeline createCustomPipeline(const std::vector<std::string>& steps);
    
private:
    PluginManager& pluginManager_;
    std::unique_ptr<ThreadPool> threadPool_;
};
```

### 4. 归档文件格式 (archive_format.h)

```cpp
struct MRNArchiveHeader {
    char magic[3] = {'M', 'R', 'N'};
    uint8_t version = 2;                    // 版本2支持文件夹
    uint16_t flags;
    uint64_t creationTime;
    uint32_t fileCount;                     // 文件数量
    uint64_t totalUncompressedSize;
    uint64_t totalCompressedSize;
    uint8_t compressionPipelineId;          // 使用的流水线ID
    char reserved[16];                      // 预留扩展
};

struct FileEntryHeader {
    char filename[256];                     // 文件名
    uint64_t uncompressedSize;
    uint64_t compressedSize;
    uint64_t fileOffset;                    // 在归档中的偏移量
    uint32_t checksum;
    uint16_t permissions;                   // 文件权限
    uint8_t compressionLevel;               // 该文件的压缩级别
    uint8_t flags;
};
```

## 文件夹压缩实现

### 1. 目录扫描器 (directory_scanner.h)

```cpp
class DirectoryScanner {
public:
    struct FileInfo {
        std::string path;
        std::string relativePath;
        uint64_t size;
        uint64_t modifiedTime;
        uint16_t permissions;
        bool isDirectory;
    };
    
    // 扫描目录
    std::vector<FileInfo> scanDirectory(const std::string& rootPath,
                                       const ScanOptions& options);
    
    // 过滤规则
    void addIncludeFilter(const std::string& pattern);
    void addExcludeFilter(const std::string& pattern);
    void setMaxFileSize(uint64_t maxSize);
    
private:
    std::vector<std::regex> includeFilters_;
    std::vector<std::regex> excludeFilters_;
    uint64_t maxFileSize_ = 0;
    
    void scanRecursive(const std::string& currentPath, 
                      const std::string& relativePath,
                      std::vector<FileInfo>& results,
                      const ScanOptions& options);
};
```

### 2. 归档写入器 (archive_writer.h)

```cpp
class ArchiveWriter {
public:
    ArchiveWriter(const std::string& filename, 
                  const CompressionPipeline& pipeline);
    
    bool addFile(const std::string& filepath, 
                 const std::string& archivePath,
                 const CompressionOptions& options);
    
    bool addDirectory(const std::string& dirpath,
                     const CompressionOptions& options);
    
    bool finalize();
    
private:
    std::ofstream archiveStream_;
    MRNArchiveHeader header_;
    std::vector<FileEntryHeader> fileEntries_;
    CompressionPipeline pipeline_;
    std::unique_ptr<ThreadPool> threadPool_;
    
    // 多线程文件处理
    void processFileBatch(const std::vector<std::string>& fileBatch,
                         const CompressionOptions& options);
};
```

## 插件系统实现

### 1. 算法注册表 (algorithm_registry.cpp)

```cpp
// 自动注册宏
#define REGISTER_ALGORITHM(ClassName) \
    namespace { \
        struct ClassName##Registrar { \
            ClassName##Registrar() { \
                PluginManager::getInstance().registerAlgorithm( \
                    ClassName::getStaticName(), \
                    std::make_unique<ClassName>() \
                ); \
            } \
        }; \
        static ClassName##Registrar ClassName##_registrar; \
    }

// MoveRun 算法实现
class MoveRunCompressor : public ICompressionAlgorithm {
public:
    static std::string getStaticName() { return "moverun"; }
    
    std::string getName() const override { return "MoveRun Compressor"; }
    std::string getVersion() const override { return "2.0"; }
    uint32_t getAlgorithmId() const override { return 0x4D52; } // "MR"
    
    CompressionResult compress(const CompressParams& params,
                              const std::vector<uint8_t>& data) override {
        // 实现压缩逻辑
        auto moved_data = moveOptimizer_.optimize(data, params.mode);
        auto lz_compressed = lz77_.compress(moved_data.data);
        return huffman_.encode(lz_compressed);
    }
    
private:
    MoveOptimizer moveOptimizer_;
    LZ77Compressor lz77_;
    HuffmanEncoder huffman_;
};

// 注册MoveRun算法
REGISTER_ALGORITHM(MoveRunCompressor);
```

### 2. 配置系统 (config.h)

```cpp
struct CompressionPreset {
    std::string name;
    CompressionPipeline pipeline;
    CompressionOptions options;
    
    // 内置预设
    static CompressionPreset createTextPreset();
    static CompressionPreset createBinaryPreset(); 
    static CompressionPreset createMaximumPreset();
    static CompressionPreset createFastPreset();
};

class ConfigurationManager {
public:
    void loadUserConfig(const std::string& configFile);
    void saveUserConfig(const std::string& configFile);
    
    CompressionPreset getPreset(const std::string& name);
    void addCustomPreset(const std::string& name, const CompressionPreset& preset);
    
    // 文件类型检测和自动预设选择
    CompressionPreset detectBestPreset(const std::string& filename);
    
private:
    std::map<std::string, CompressionPreset> presets_;
    std::map<std::string, std::string> fileAssociations_;
};
```

## 多线程优化

### 1. 并行压缩策略 (compressor.cpp)

```cpp
void ModularCompressor::compressDirectoryParallel(
    const std::string& inputDir,
    const std::string& outputFile,
    const CompressionPipeline& pipeline,
    const CompressionOptions& options) {
    
    // 1. 扫描目录
    auto files = directoryScanner_.scanDirectory(inputDir, options.scanOptions);
    
    // 2. 按文件类型分组
    auto fileGroups = groupFilesByType(files);
    
    // 3. 为每组文件分配合适的流水线
    std::vector<std::future<FileCompressionResult>> futures;
    
    for (const auto& group : fileGroups) {
        auto groupPipeline = selectPipelineForFileGroup(group.fileType, pipeline);
        
        for (const auto& file : group.files) {
            futures.push_back(threadPool_->enqueue([this, file, groupPipeline, options] {
                return compressSingleFile(file, groupPipeline, options);
            }));
        }
    }
    
    // 4. 收集结果并写入归档
    ArchiveWriter writer(outputFile, pipeline);
    for (auto& future : futures) {
        auto result = future.get();
        writer.addCompressedFile(result);
    }
    
    writer.finalize();
}
```

## 扩展的CMake配置

```cmake
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
if(MRN_BUILD_PLUGINS)
    add_subdirectory(plugins)
endif()

# 安装
install(TARGETS mrn DESTINATION bin)
install(DIRECTORY include/ DESTINATION include/mrn)
```

## 命令行接口增强

```cpp
// 在main.cpp中实现
struct CommandLineOptions {
    enum Operation { COMPRESS, DECOMPRESS, LIST, TEST };
    
    Operation operation;
    std::vector<std::string> inputPaths;
    std::string outputPath;
    int threadCount = 1;
    std::string preset = "auto";
    std::string algorithm;
    int compressionLevel = 6;
    bool verbose = false;
    bool overwrite = false;
    bool preservePaths = true;
};

// 使用示例:
// mrn -c folder1 file2.txt -o archive.mrn -j4 --preset maximum
// mrn -d archive.mrn -o extracted/ --threads 8
// mrn -l archive.mrn  # 列出内容
// mrn -t archive.mrn  # 测试完整性
```

## 开发扩展指南

### 添加新压缩算法:
1. 实现 `ICompressionAlgorithm` 接口
2. 使用 `REGISTER_ALGORITHM` 宏注册
3. 在配置文件中定义预设

### 添加新预处理器:
1. 实现 `IPreprocessor` 接口  
2. 注册到插件管理器
3. 在流水线配置中使用

### 添加文件类型支持:
1. 在 `ConfigurationManager` 中添加文件类型检测
2. 定义针对该类型的优化流水线
3. 测试压缩效果
