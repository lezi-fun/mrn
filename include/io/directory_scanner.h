#pragma once

#include "../core/archive_format.h"
#include <string>
#include <vector>
#include <regex>

namespace mrn {

class DirectoryScanner {
public:
    struct FileInfo {
        std::string path;
        std::string relativePath;
        uint64_t size;
        uint64_t modifiedTime;
        uint16_t permissions;
        bool isDirectory;
        
        FileInfo() : size(0), modifiedTime(0), permissions(0), isDirectory(false) {}
    };
    
    DirectoryScanner();
    ~DirectoryScanner() = default;
    
    // 扫描目录
    std::vector<FileInfo> scanDirectory(const std::string& rootPath,
                                       const ScanOptions& options);
    
    // 过滤规则
    void addIncludeFilter(const std::string& pattern);
    void addExcludeFilter(const std::string& pattern);
    void setMaxFileSize(uint64_t maxSize);
    
    // 获取扫描统计信息
    struct ScanStats {
        uint32_t totalFiles = 0;
        uint32_t totalDirectories = 0;
        uint64_t totalSize = 0;
        uint32_t skippedFiles = 0;
        uint32_t excludedFiles = 0;
    };
    
    ScanStats getStats() const { return stats_; }
    
private:
    std::vector<std::regex> includeFilters_;
    std::vector<std::regex> excludeFilters_;
    uint64_t maxFileSize_ = 0;
    ScanStats stats_;
    
    // 递归扫描
    void scanRecursive(const std::string& currentPath, 
                      const std::string& relativePath,
                      std::vector<FileInfo>& results,
                      const ScanOptions& options);
    
    // 检查文件是否应该被包含
    bool shouldIncludeFile(const std::string& filename, 
                          const std::string& relativePath,
                          uint64_t fileSize) const;
    
    // 检查是否为隐藏文件
    bool isHiddenFile(const std::string& filename) const;
    
    // 应用过滤规则
    bool matchesPattern(const std::string& text, const std::vector<std::regex>& patterns) const;
};

} // namespace mrn
