#pragma once

#include "plugin_interface.h"
#include "archive_format.h"
#include "plugin_manager.h"
#include <string>
#include <memory>

namespace mrn {

// 文件压缩结果
struct FileCompressionResult {
    std::string filename;
    ::mrn::CompressionResult result;
    bool success = false;
    std::string errorMessage;
};

// 总体压缩结果
struct OverallCompressionResult {
    bool success = false;
    uint64_t totalUncompressedSize = 0;
    uint64_t totalCompressedSize = 0;
    uint32_t fileCount = 0;
    std::string errorMessage;
    std::vector<FileCompressionResult> fileResults;
};

// 总体解压结果
struct OverallDecompressionResult {
    bool success = false;
    uint32_t fileCount = 0;
    std::string errorMessage;
};

class ModularCompressor {
public:
    ModularCompressor();
    ~ModularCompressor();
    
    // 文件夹压缩
    OverallCompressionResult compressDirectory(const std::string& inputDir,
                                             const std::string& outputFile,
                                             const CompressionPipeline& pipeline,
                                             const CompressionOptions& options);
    
    // 文件压缩
    OverallCompressionResult compressFile(const std::string& inputFile,
                                        const std::string& outputFile, 
                                        const CompressionPipeline& pipeline,
                                        const CompressionOptions& options);
    
    // 解压
    OverallDecompressionResult decompress(const std::string& inputFile,
                                        const std::string& outputPath);
    
    // 流水线构建
    void setDefaultPipeline(const std::string& preset); // "text", "binary", "maximum"
    CompressionPipeline createCustomPipeline(const std::vector<std::string>& steps);
    
    // 配置预设
    struct CompressionPreset {
        std::string name;
        CompressionPipeline pipeline;
        CompressionOptions options;
    };
    
    static CompressionPreset createTextPreset();
    static CompressionPreset createBinaryPreset(); 
    static CompressionPreset createMaximumPreset();
    static CompressionPreset createFastPreset();
    static CompressionPreset createStorePreset();
    
    // 文件类型检测
    static std::string detectFileType(const std::string& filename);
    static CompressionPreset detectBestPreset(const std::string& filename);
    
private:
    PluginManager& pluginManager_;
    std::unique_ptr<class ThreadPool> threadPool_;
    CompressionPipeline defaultPipeline_;
    
    // 内部方法
    FileCompressionResult compressSingleFile(const std::string& filepath,
                                            const CompressionPipeline& pipeline,
                                            const CompressionOptions& options);
    
    void compressDirectoryParallel(const std::string& inputDir,
                                  const std::string& outputFile,
                                  const CompressionPipeline& pipeline,
                                  const CompressionOptions& options);
    
    std::vector<std::vector<std::string>> groupFilesByType(const std::vector<std::string>& files);
    CompressionPipeline selectPipelineForFileGroup(const std::string& fileType, 
                                                  const CompressionPipeline& basePipeline);
};

} // namespace mrn
