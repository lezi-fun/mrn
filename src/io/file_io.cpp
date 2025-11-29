#include "io/file_io.h"
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace mrn {

std::vector<uint8_t> FileIO::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return {};
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return {};
    }
    
    return buffer;
}

bool FileIO::writeFile(const std::string& filename, const std::vector<uint8_t>& data) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    return file.good();
}

bool FileIO::fileExists(const std::string& filename) {
    return std::filesystem::exists(filename);
}

uint64_t FileIO::getFileSize(const std::string& filename) {
    try {
        return std::filesystem::file_size(filename);
    } catch (...) {
        return 0;
    }
}

uint64_t FileIO::getFileModifiedTime(const std::string& filename) {
    try {
        auto ftime = std::filesystem::last_write_time(filename);
        return std::chrono::duration_cast<std::chrono::seconds>(
            ftime.time_since_epoch()).count();
    } catch (...) {
        return 0;
    }
}

uint16_t FileIO::getFilePermissions(const std::string& filename) {
    // Default permissions
    return 0644;
}

bool FileIO::createDirectory(const std::string& path) {
    return std::filesystem::create_directories(path);
}

bool FileIO::isDirectory(const std::string& path) {
    return std::filesystem::is_directory(path);
}

std::string FileIO::getFileExtension(const std::string& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "";
    }
    std::string ext = filename.substr(dotPos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

std::string FileIO::getFilename(const std::string& path) {
    return std::filesystem::path(path).filename().string();
}

std::string FileIO::getDirectory(const std::string& path) {
    return std::filesystem::path(path).parent_path().string();
}

std::string FileIO::joinPaths(const std::string& path1, const std::string& path2) {
    return (std::filesystem::path(path1) / path2).string();
}

std::string FileIO::normalizePath(const std::string& path) {
    return std::filesystem::path(path).lexically_normal().string();
}

FileIO::FileType FileIO::detectFileType(const std::string& filename) {
    auto extension = getFileExtension(filename);
    
    if (isCompressedArchive(extension)) return COMPRESSED_ARCHIVE;
    if (isCompressedImage(extension)) return COMPRESSED_IMAGE;
    if (isCompressedAudio(extension)) return COMPRESSED_AUDIO;
    if (isCompressedVideo(extension)) return COMPRESSED_VIDEO;
    if (isExecutable(extension)) return EXECUTABLE;
    
    // For text detection, we'd need to read the file content
    // For now, assume unknown files are binary
    return BINARY;
}

bool FileIO::shouldSkipCompression(const std::string& filename) {
    auto extension = getFileExtension(filename);
    return isCompressedArchive(extension) || 
           isCompressedImage(extension) ||
           isCompressedAudio(extension) ||
           isCompressedVideo(extension);
}

bool FileIO::isTextFile(const std::vector<uint8_t>& data) {
    // Simple heuristic: if most bytes are printable ASCII, it's text
    size_t printable = 0;
    for (uint8_t byte : data) {
        if (byte >= 32 && byte <= 126 || byte == 9 || byte == 10 || byte == 13) {
            printable++;
        }
    }
    return printable * 100 / data.size() > 80;
}

bool FileIO::isCompressedArchive(const std::string& extension) {
    static const std::vector<std::string> extensions = {
        "zip", "gz", "bz2", "xz", "7z", "rar", "tar", "cab", "iso", "dmg"
    };
    return std::find(extensions.begin(), extensions.end(), extension) != extensions.end();
}

bool FileIO::isCompressedImage(const std::string& extension) {
    static const std::vector<std::string> extensions = {
        "jpg", "jpeg", "png", "gif", "webp"
    };
    return std::find(extensions.begin(), extensions.end(), extension) != extensions.end();
}

bool FileIO::isCompressedAudio(const std::string& extension) {
    static const std::vector<std::string> extensions = {
        "mp3", "aac", "ogg", "wma", "flac", "m4a", "wav", "opus", "ape"
    };
    return std::find(extensions.begin(), extensions.end(), extension) != extensions.end();
}

bool FileIO::isCompressedVideo(const std::string& extension) {
    static const std::vector<std::string> extensions = {
        "mp4", "avi", "mkv", "mov", "wmv", "flv", "webm", "m4v", "mpg", "mpeg", "3gp", "ogv", "ts", "mts"
    };
    return std::find(extensions.begin(), extensions.end(), extension) != extensions.end();
}

bool FileIO::isExecutable(const std::string& extension) {
    static const std::vector<std::string> extensions = {
        "exe", "dll", "so", "dylib", "bin", "app", "deb", "rpm", "pkg"
    };
    return std::find(extensions.begin(), extensions.end(), extension) != extensions.end();
}

} // namespace mrn
