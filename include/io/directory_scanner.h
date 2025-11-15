#pragma once

#include <cstdint>
#include <regex>
#include <string>
#include <vector>

namespace mrn {

struct ScanOptions {
    bool followSymlinks = false;
    bool includeHidden = true;
};

class DirectoryScanner {
public:
    struct FileInfo {
        std::string path;
        std::string relativePath;
        uint64_t size = 0;
        uint64_t modifiedTime = 0;
        uint16_t permissions = 0;
        bool isDirectory = false;
    };

    std::vector<FileInfo> scanDirectory(const std::string& rootPath,
                                        const ScanOptions& options);

    void addIncludeFilter(const std::string& pattern);
    void addExcludeFilter(const std::string& pattern);
    void setMaxFileSize(uint64_t maxSize);

private:
    std::vector<std::regex> includeFilters_;
    std::vector<std::regex> excludeFilters_;
    uint64_t maxFileSize_ = 0;

    void scanRecursive(const std::string& rootPath,
                       const std::string& currentPath,
                       std::vector<FileInfo>& results,
                       const ScanOptions& options);

    bool shouldInclude(const std::string& relativePath, uint64_t size) const;
};

} // namespace mrn
