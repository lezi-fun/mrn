#include "core/config.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace mrn {

namespace {
CompressionPreset makePreset(const std::string& name, int level) {
    CompressionPreset preset;
    preset.name = name;
    preset.pipeline.mainAlgorithm = "moverun";
    preset.options.compressionLevel = level;
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
    
    // 根据扩展名推断
    if (ext == "txt" || ext == "md" || ext == "cpp" || ext == "h" || ext == "hpp" ||
        ext == "c" || ext == "java" || ext == "py" || ext == "js" || ext == "json" ||
        ext == "xml" || ext == "html" || ext == "css") {
        return CompressionPreset::createTextPreset();
    } else if (ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "gif" ||
               ext == "bmp" || ext == "tiff") {
        return CompressionPreset::createBinaryPreset();
    } else if (ext == "zip" || ext == "gz" || ext == "bz2" || ext == "xz" ||
               ext == "7z" || ext == "rar") {
        return CompressionPreset::createFastPreset(); // 已压缩文件，快速处理
    }
    
    // 默认使用二进制预设
    return CompressionPreset::createBinaryPreset();
}

} // namespace mrn
