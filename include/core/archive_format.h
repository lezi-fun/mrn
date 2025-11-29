#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <map>
#include "plugin_interface.h"

namespace mrn {

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
    
    MRNArchiveHeader() {
        std::memset(reserved, 0, sizeof(reserved));
        flags = 0;
        creationTime = 0;
        fileCount = 0;
        totalUncompressedSize = 0;
        totalCompressedSize = 0;
        compressionPipelineId = 0;
    }
    
    bool isValid() const {
        return magic[0] == 'M' && magic[1] == 'R' && magic[2] == 'N';
    }
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
    
    FileEntryHeader() {
        std::memset(filename, 0, sizeof(filename));
        uncompressedSize = 0;
        compressedSize = 0;
        fileOffset = 0;
        checksum = 0;
        permissions = 0;
        compressionLevel = 0;
        flags = 0;
    }
    
    void setFilename(const std::string& name) {
        std::strncpy(filename, name.c_str(), sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
    }
    
    std::string getFilename() const {
        return std::string(filename);
    }
};

// 压缩流水线配置
struct CompressionPipeline {
    std::vector<std::string> preprocessors;  // 预处理器链
    std::string mainAlgorithm;               // 主压缩算法
    std::map<std::string, AlgorithmConfig> algorithmConfigs; // 算法配置
};

// 压缩选项
struct CompressionOptions {
    int threadCount = 1;
    int compressionLevel = 6;
    bool verbose = false;
    bool overwrite = false;
    bool preservePaths = true;
    bool skipCompression = false;           // 对于已压缩文件直接存储
    uint64_t maxFileSize = 0;               // 最大文件大小限制
};

// 扫描选项
struct ScanOptions {
    bool recursive = true;
    bool followSymlinks = false;
    bool includeHidden = false;
    std::vector<std::string> includePatterns;
    std::vector<std::string> excludePatterns;
};

} // namespace mrn
