#include "core/plugin_manager.h"
#include "algorithms/move_optimizer.h"
#include "algorithms/lz77_compressor.h"
#include "algorithms/huffman_encoder.h"
#include <stdexcept>

namespace mrn {

PluginManager& PluginManager::getInstance() {
    static PluginManager instance;
    return instance;
}

bool PluginManager::registerAlgorithm(const std::string& type, 
                                     std::unique_ptr<ICompressionAlgorithm> algorithm) {
    if (!algorithm) {
        return false;
    }
    
    auto result = algorithms_.emplace(type, std::move(algorithm));
    return result.second;
}

bool PluginManager::registerPreprocessor(const std::string& name,
                                       std::unique_ptr<IPreprocessor> preprocessor) {
    if (!preprocessor) {
        return false;
    }
    
    auto result = preprocessors_.emplace(name, std::move(preprocessor));
    return result.second;
}

std::vector<std::string> PluginManager::getAvailableAlgorithms() const {
    std::vector<std::string> names;
    for (const auto& pair : algorithms_) {
        names.push_back(pair.first);
    }
    return names;
}

std::vector<std::string> PluginManager::getAvailablePreprocessors() const {
    std::vector<std::string> names;
    for (const auto& pair : preprocessors_) {
        names.push_back(pair.first);
    }
    return names;
}

ICompressionAlgorithm* PluginManager::getAlgorithm(const std::string& name) {
    auto it = algorithms_.find(name);
    if (it != algorithms_.end()) {
        return it->second.get();
    }
    return nullptr;
}

IPreprocessor* PluginManager::getPreprocessor(const std::string& name) {
    auto it = preprocessors_.find(name);
    if (it != preprocessors_.end()) {
        return it->second.get();
    }
    return nullptr;
}

void PluginManager::loadPluginsFromDirectory(const std::string& directory) {
    // TODO: 实现动态库加载
    // 当前版本只支持内置插件
}

void PluginManager::initializeBuiltinPlugins() {
    // 注册内置压缩算法
    // MoveRun算法将在algorithm_registry.cpp中通过宏自动注册
}

} // namespace mrn
