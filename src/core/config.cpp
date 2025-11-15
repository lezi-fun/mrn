#include "core/config.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace mrn {

namespace {
CompressionPreset makePreset(const std::string& name, int level, bool skipCompression = false) {
    CompressionPreset preset;
    preset.name = name;
    preset.pipeline.mainAlgorithm = "moverun";
    preset.options.compressionLevel = level;
    preset.options.skipCompression = skipCompression; // 标记是否跳过压缩
    return preset;
}
}

CompressionPreset CompressionPreset::createTextPreset() {
    return makePreset("text", 6);
}

CompressionPreset CompressionPreset::createBinaryPreset() {
    return makePreset("binary", 5);
}

CompressionPreset CompressionPreset::createMaximumPreset() {
    return makePreset("maximum", 9);
}

CompressionPreset CompressionPreset::createFastPreset() {
    return makePreset("fast", 3);
}

CompressionPreset CompressionPreset::createStorePreset() {
    return makePreset("store", 0, true); // 不压缩，直接存储
}

void ConfigurationManager::loadUserConfig(const std::string& configFile) {
    if (!std::filesystem::exists(configFile)) {
        return;
    }
    
    std::ifstream file(configFile);
    if (!file.is_open()) {
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            if (key == "preset") {
                // 加载预设配置
            } else if (key.find("filetype.") == 0) {
                std::string ext = key.substr(9);
                fileAssociations_[ext] = value;
            }
        }
    }
}

void ConfigurationManager::saveUserConfig(const std::string& configFile) {
    std::ofstream file(configFile);
    if (!file.is_open()) {
        return;
    }
    
    file << "# MRN Compressor Configuration\n";
    file << "# File type associations\n";
    for (const auto& [ext, preset] : fileAssociations_) {
        file << "filetype." << ext << "=" << preset << "\n";
    }
    
    file << "\n# Custom presets\n";
    for (const auto& [name, preset] : presets_) {
        file << "preset." << name << ".algorithm=" << preset.pipeline.mainAlgorithm << "\n";
        file << "preset." << name << ".level=" << preset.options.compressionLevel << "\n";
    }
}

CompressionPreset ConfigurationManager::getPreset(const std::string& name) {
    auto it = presets_.find(name);
    if (it != presets_.end()) {
        return it->second;
    }
    return CompressionPreset::createTextPreset();
}

void ConfigurationManager::addCustomPreset(const std::string& name,
                                           const CompressionPreset& preset) {
    presets_[name] = preset;
}

CompressionPreset ConfigurationManager::detectBestPreset(const std::string& filename) {
    std::filesystem::path path(filename);
    std::string ext = path.extension().string();
    if (!ext.empty() && ext[0] == '.') {
        ext = ext.substr(1);
    }
    
    // 转换为小写
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    // 检查文件类型关联
    auto it = fileAssociations_.find(ext);
    if (it != fileAssociations_.end()) {
        return getPreset(it->second);
    }
    
    // 根据扩展名推断 - 智能文件类型检测
    
    // 视频文件：通常已高度压缩，直接存储
    if (ext == "mp4" || ext == "avi" || ext == "mkv" || ext == "mov" || ext == "wmv" ||
        ext == "flv" || ext == "webm" || ext == "m4v" || ext == "mpg" || ext == "mpeg" ||
        ext == "3gp" || ext == "ogv" || ext == "ts" || ext == "mts") {
        return CompressionPreset::createStorePreset();
    }
    
    // 音频文件：通常已压缩，直接存储
    if (ext == "mp3" || ext == "aac" || ext == "ogg" || ext == "wma" || ext == "flac" ||
        ext == "m4a" || ext == "wav" || ext == "opus" || ext == "ape") {
        return CompressionPreset::createStorePreset();
    }
    
    // 已压缩的归档文件：直接存储
    if (ext == "zip" || ext == "gz" || ext == "bz2" || ext == "xz" || ext == "7z" ||
        ext == "rar" || ext == "tar" || ext == "cab" || ext == "iso" || ext == "dmg") {
        return CompressionPreset::createStorePreset();
    }
    
    // 已压缩的图片格式：使用快速模式（可能仍有少量冗余）
    if (ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "gif" || ext == "webp") {
        return CompressionPreset::createFastPreset();
    }
    
    // 未压缩的图片格式：使用二进制预设
    if (ext == "bmp" || ext == "tiff" || ext == "tif" || ext == "raw" || ext == "psd") {
        return CompressionPreset::createBinaryPreset();
    }
    
    // 文档文件：文本类，使用文本预设
    if (ext == "txt" || ext == "md" || ext == "rst" || ext == "log" || ext == "csv") {
        return CompressionPreset::createTextPreset();
    }
    
    // 代码文件：使用文本预设
    if (ext == "cpp" || ext == "hpp" || ext == "c" || ext == "h" || ext == "cc" ||
        ext == "cxx" || ext == "java" || ext == "py" || ext == "js" || ext == "ts" ||
        ext == "go" || ext == "rs" || ext == "swift" || ext == "kt" || ext == "scala" ||
        ext == "php" || ext == "rb" || ext == "pl" || ext == "sh" || ext == "bash") {
        return CompressionPreset::createTextPreset();
    }
    
    // 标记语言和配置文件：使用文本预设
    if (ext == "xml" || ext == "html" || ext == "htm" || ext == "css" || ext == "scss" ||
        ext == "json" || ext == "yaml" || ext == "yml" || ext == "toml" || ext == "ini" ||
        ext == "conf" || ext == "config" || ext == "properties") {
        return CompressionPreset::createTextPreset();
    }
    
    // 文档格式：根据类型选择
    if (ext == "pdf") {
        // PDF可能已压缩，使用快速模式
        return CompressionPreset::createFastPreset();
    }
    if (ext == "doc" || ext == "docx" || ext == "xls" || ext == "xlsx" || 
        ext == "ppt" || ext == "pptx" || ext == "odt" || ext == "ods" || ext == "odp") {
        // Office文档通常有压缩，使用快速模式
        return CompressionPreset::createFastPreset();
    }
    
    // 可执行文件和库文件：使用二进制预设
    if (ext == "exe" || ext == "dll" || ext == "so" || ext == "dylib" || ext == "bin" ||
        ext == "app" || ext == "deb" || ext == "rpm" || ext == "pkg") {
        return CompressionPreset::createBinaryPreset();
    }
    
    // 数据库文件：使用二进制预设
    if (ext == "db" || ext == "sqlite" || ext == "sqlite3" || ext == "mdb") {
        return CompressionPreset::createBinaryPreset();
    }
    
    // 默认使用二进制预设
    return CompressionPreset::createBinaryPreset();
}

} // namespace mrn
