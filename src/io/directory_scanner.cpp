#include "io/directory_scanner.h"
#include "io/file_io.h"
#include <filesystem>
#include <regex>

namespace mrn {

DirectoryScanner::DirectoryScanner() : maxFileSize_(0) {}

std::vector<DirectoryScanner::FileInfo> DirectoryScanner::scanDirectory(const std::string& rootPath,
                                                                       const ScanOptions& options) {
    std::vector<FileInfo> results;
    stats_ = ScanStats();
    
    if (!FileIO::isDirectory(rootPath)) {
        return results;
    }
    
    scanRecursive(rootPath, "", results, options);
    return results;
}

void DirectoryScanner::addIncludeFilter(const std::string& pattern) {
    try {
        includeFilters_.emplace_back(pattern, std::regex::icase);
    } catch (const std::regex_error&) {
        // Ignore invalid regex patterns
    }
}

void DirectoryScanner::addExcludeFilter(const std::string& pattern) {
    try {
        excludeFilters_.emplace_back(pattern, std::regex::icase);
    } catch (const std::regex_error&) {
        // Ignore invalid regex patterns
    }
}

void DirectoryScanner::setMaxFileSize(uint64_t maxSize) {
    maxFileSize_ = maxSize;
}

void DirectoryScanner::scanRecursive(const std::string& currentPath, 
                                    const std::string& relativePath,
                                    std::vector<FileInfo>& results,
                                    const ScanOptions& options) {
    try {
        for (const auto& entry : std::filesystem::directory_iterator(currentPath)) {
            if (!options.includeHidden && isHiddenFile(entry.path().filename().string())) {
                continue;
            }
            
            std::string entryRelativePath = relativePath.empty() 
                ? entry.path().filename().string()
                : FileIO::joinPaths(relativePath, entry.path().filename().string());
            
            if (entry.is_directory()) {
                stats_.totalDirectories++;
                if (options.recursive) {
                    scanRecursive(entry.path().string(), entryRelativePath, results, options);
                }
            } else if (entry.is_regular_file()) {
                stats_.totalFiles++;
                uint64_t fileSize = entry.file_size();
                stats_.totalSize += fileSize;
                
                if (shouldIncludeFile(entry.path().filename().string(), entryRelativePath, fileSize)) {
                    FileInfo info;
                    info.path = entry.path().string();
                    info.relativePath = entryRelativePath;
                    info.size = fileSize;
                    info.modifiedTime = FileIO::getFileModifiedTime(entry.path().string());
                    info.permissions = FileIO::getFilePermissions(entry.path().string());
                    info.isDirectory = false;
                    
                    results.push_back(info);
                } else {
                    stats_.excludedFiles++;
                }
            }
        }
    } catch (const std::filesystem::filesystem_error&) {
        // Ignore permission errors and other filesystem issues
    }
}

bool DirectoryScanner::shouldIncludeFile(const std::string& filename, 
                                        const std::string& relativePath,
                                        uint64_t fileSize) const {
    if (maxFileSize_ > 0 && fileSize > maxFileSize_) {
        return false;
    }
    
    // Check exclude filters first
    if (!excludeFilters_.empty() && matchesPattern(relativePath, excludeFilters_)) {
        return false;
    }
    
    // Check include filters
    if (!includeFilters_.empty() && !matchesPattern(relativePath, includeFilters_)) {
        return false;
    }
    
    return true;
}

bool DirectoryScanner::isHiddenFile(const std::string& filename) const {
    return !filename.empty() && filename[0] == '.';
}

bool DirectoryScanner::matchesPattern(const std::string& text, const std::vector<std::regex>& patterns) const {
    for (const auto& pattern : patterns) {
        if (std::regex_search(text, pattern)) {
            return true;
        }
    }
    return false;
}

} // namespace mrn
