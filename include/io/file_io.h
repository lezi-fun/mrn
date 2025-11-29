#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <fstream>

namespace mrn {

class FileIO {
public:
    // 读取文件内容
    static std::vector<uint8_t> readFile(const std::string& filename);
    
    // 写入文件内容
    static bool writeFile(const std::string& filename, const std::vector<uint8_t>& data);
    
    // 检查文件是否存在
    static bool fileExists(const std::string& filename);
    
    // 获取文件大小
    static uint64_t getFileSize(const std::string& filename);
    
    // 获取文件修改时间
    static uint64_t getFileModifiedTime(const std::string& filename);
    
    // 获取文件权限
    static uint16_t getFilePermissions(const std::string& filename);
    
    // 创建目录
    static bool createDirectory(const std::string& path);
    
    // 检查是否为目录
    static bool isDirectory(const std::string& path);
    
    // 获取文件扩展名
    static std::string getFileExtension(const std::string& filename);
    
    // 获取文件名（不含路径）
    static std::string getFilename(const std::string& path);
    
    // 获取目录路径
    static std::string getDirectory(const std::string& path);
    
    // 路径拼接
    static std::string joinPaths(const std::string& path1, const std::string& path2);
    
    // 规范化路径（处理../和./）
    static std::string normalizePath(const std::string& path);
    
    // 文件类型检测
    enum FileType {
        UNKNOWN,
        TEXT,
        BINARY,
        COMPRESSED_ARCHIVE,
        COMPRESSED_IMAGE,
        COMPRESSED_AUDIO,
        COMPRESSED_VIDEO,
        EXECUTABLE
    };
    
    static FileType detectFileType(const std::string& filename);
    
    // 判断是否应该跳过压缩的文件类型
    static bool shouldSkipCompression(const std::string& filename);
    
private:
    // 内部实现方法
    static bool isTextFile(const std::vector<uint8_t>& data);
    static bool isCompressedArchive(const std::string& extension);
    static bool isCompressedImage(const std::string& extension);
    static bool isCompressedAudio(const std::string& extension);
    static bool isCompressedVideo(const std::string& extension);
    static bool isExecutable(const std::string& extension);
};

} // namespace mrn
