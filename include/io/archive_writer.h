#pragma once

#include "../core/archive_format.h"
#include "../core/plugin_interface.h"
#include "../core/compressor.h"
#include <string>
#include <vector>
#include <fstream>
#include <memory>

namespace mrn {

class ArchiveWriter {
public:
    ArchiveWriter(const std::string& filename, 
                  const CompressionPipeline& pipeline);
    ~ArchiveWriter();
    
    // 添加单个文件到归档
    bool addFile(const std::string& filepath, 
                 const std::string& archivePath,
                 const CompressionOptions& options);
    
    // 添加整个目录到归档
    bool addDirectory(const std::string& dirpath,
                     const CompressionOptions& options);
    
    // 添加已压缩的文件数据
    bool addCompressedFile(const FileCompressionResult& result,
                          const std::string& archivePath);
    
    // 完成归档写入
    bool finalize();
    
    // 获取归档统计信息
    struct ArchiveStats {
        uint32_t totalFiles = 0;
        uint64_t totalUncompressedSize = 0;
        uint64_t totalCompressedSize = 0;
        double compressionRatio = 0.0;
    };
    
    ArchiveStats getStats() const { return stats_; }
    
    // 检查归档是否有效
    bool isValid() const { return archiveStream_.is_open(); }
    
private:
    std::ofstream archiveStream_;
    MRNArchiveHeader header_;
    std::vector<FileEntryHeader> fileEntries_;
    CompressionPipeline pipeline_;
    std::unique_ptr<class ThreadPool> threadPool_;
    ArchiveStats stats_;
    uint64_t currentOffset_;
    
    // 初始化归档头
    void initializeHeader();
    
    // 写入归档头
    bool writeHeader();
    
    // 写入文件条目表
    bool writeFileTable();
    
    // 写入单个文件数据
    bool writeFileData(const std::string& filepath,
                      const std::string& archivePath,
                      const CompressionOptions& options,
                      FileEntryHeader& entry);
    
    // 压缩单个文件
    FileCompressionResult compressFile(const std::string& filepath,
                                      const CompressionOptions& options);
    
    // 多线程文件处理
    void processFileBatch(const std::vector<std::string>& fileBatch,
                         const CompressionOptions& options);
    
    // 计算文件校验和
    uint32_t calculateFileChecksum(const std::vector<uint8_t>& data) const;
    
    // 获取当前时间戳
    uint64_t getCurrentTimestamp() const;
};

} // namespace mrn
