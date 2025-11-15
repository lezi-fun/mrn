#pragma once

#include <future>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "core/plugin_manager.h"
#include "io/directory_scanner.h"
#include "utils/thread_pool.h"

namespace mrn {

struct CompressionOptions {
    int compressionLevel = 6;
    bool overwrite = false;
    bool verbose = false;
    size_t batchSize = 4;
    ScanOptions scanOptions;
};

struct CompressionPipeline {
    std::vector<std::string> preprocessors;
    std::string mainAlgorithm = "moverun";
    std::map<std::string, AlgorithmConfig> algorithmConfigs;
};

struct FileCompressionResult {
    std::string originalPath;
    std::string archivePath;
    CompressionResult result;
    uint16_t filePermissions = 0;
    uint64_t modifiedTime = 0;
    uint32_t checksum = 0;
};

class DirectoryScanner;
class ArchiveWriter;

class ModularCompressor {
public:
    explicit ModularCompressor(size_t threadCount = 0);

    CompressionResult compressFile(const std::string& inputFile,
                                    const std::string& outputFile,
                                    const CompressionPipeline& pipeline,
                                    const CompressionOptions& options);

    CompressionResult compressDirectory(const std::string& inputDir,
                                         const std::string& outputFile,
                                         const CompressionPipeline& pipeline,
                                         const CompressionOptions& options);

    DecompressionResult decompress(const std::string& inputFile,
                                   const std::string& outputPath);

    // 列出归档内容
    void listArchive(const std::string& inputFile);

    // 测试归档完整性
    bool testArchive(const std::string& inputFile);

    void setDefaultPipeline(const std::string& preset);
    CompressionPipeline createCustomPipeline(const std::vector<std::string>& steps);

private:
    PluginManager& pluginManager_;
    std::unique_ptr<ThreadPool> threadPool_;
    std::unique_ptr<DirectoryScanner> directoryScanner_;
    CompressionPipeline defaultPipeline_;

    FileCompressionResult compressSingleFile(const std::string& filepath,
                                             const std::string& archivePath,
                                             const CompressionPipeline& pipeline,
                                             const CompressionOptions& options);
};

} // namespace mrn
