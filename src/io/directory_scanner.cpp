#include "io/directory_scanner.h"

#include <filesystem>

namespace fs = std::filesystem;

namespace mrn {

namespace {
bool matchFilters(const std::vector<std::regex>& filters, const std::string& value) {
    if (filters.empty()) {
        return true;
    }
    for (const auto& filter : filters) {
        if (std::regex_match(value, filter)) {
            return true;
        }
    }
    return false;
}
}

std::vector<DirectoryScanner::FileInfo> DirectoryScanner::scanDirectory(
    const std::string& rootPath, const ScanOptions& options) {
    std::vector<FileInfo> results;
    scanRecursive(rootPath, "", results, options);
    return results;
}

void DirectoryScanner::addIncludeFilter(const std::string& pattern) {
    includeFilters_.emplace_back(pattern);
}

void DirectoryScanner::addExcludeFilter(const std::string& pattern) {
    excludeFilters_.emplace_back(pattern);
}

void DirectoryScanner::setMaxFileSize(uint64_t maxSize) {
    maxFileSize_ = maxSize;
}

void DirectoryScanner::scanRecursive(const std::string& rootPath,
                                     const std::string& currentPath,
                                     std::vector<FileInfo>& results,
                                     const ScanOptions& options) {
    const auto absolutePath = currentPath.empty() ? fs::path(rootPath)
                                                  : fs::path(rootPath) / currentPath;

    if (!fs::exists(absolutePath)) {
        return;
    }

    for (const auto& entry : fs::directory_iterator(absolutePath)) {
        const auto filename = entry.path().filename().string();
        if (!options.includeHidden && !filename.empty() && filename[0] == '.') {
            continue;
        }
        if (!options.followSymlinks && fs::is_symlink(entry.symlink_status())) {
            continue;
        }

        FileInfo info;
        info.path = entry.path().string();
        if (currentPath.empty()) {
            info.relativePath = filename;
        } else {
            info.relativePath = (fs::path(currentPath) / entry.path().filename()).string();
        }
        info.isDirectory = entry.is_directory();
        if (!info.isDirectory) {
            info.size = entry.file_size();
        }
        info.modifiedTime = entry.last_write_time().time_since_epoch().count();

        if (!shouldInclude(info.relativePath, info.size)) {
            continue;
        }

        results.push_back(info);

        if (info.isDirectory) {
            scanRecursive(rootPath, info.relativePath, results, options);
        }
    }
}

bool DirectoryScanner::shouldInclude(const std::string& relativePath, uint64_t size) const {
    if (maxFileSize_ != 0 && size > maxFileSize_) {
        return false;
    }

    if (!matchFilters(includeFilters_, relativePath)) {
        return false;
    }

    for (const auto& filter : excludeFilters_) {
        if (std::regex_match(relativePath, filter)) {
            return false;
        }
    }
    return true;
}

} // namespace mrn
