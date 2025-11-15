#include "core/plugin_manager.h"

#include <filesystem>

#include "utils/logger.h"

namespace mrn {

PluginManager& PluginManager::getInstance() {
    static PluginManager instance;
    return instance;
}

bool PluginManager::registerAlgorithm(const std::string& name,
                                      std::unique_ptr<ICompressionAlgorithm> algorithm) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (algorithms_.count(name)) {
        return false;
    }
    algorithms_[name] = std::move(algorithm);
    Logger::instance().log(Logger::Level::Debug, "Registered algorithm: " + name);
    return true;
}

bool PluginManager::registerPreprocessor(const std::string& name,
                                         std::unique_ptr<IPreprocessor> preprocessor) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (preprocessors_.count(name)) {
        return false;
    }
    preprocessors_[name] = std::move(preprocessor);
    Logger::instance().log(Logger::Level::Debug, "Registered preprocessor: " + name);
    return true;
}

std::vector<std::string> PluginManager::getAvailableAlgorithms() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> names;
    for (const auto& pair : algorithms_) {
        names.push_back(pair.first);
    }
    return names;
}

std::vector<std::string> PluginManager::getAvailablePreprocessors() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> names;
    for (const auto& pair : preprocessors_) {
        names.push_back(pair.first);
    }
    return names;
}

ICompressionAlgorithm* PluginManager::getAlgorithm(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = algorithms_.find(name);
    if (it != algorithms_.end()) {
        return it->second.get();
    }
    return nullptr;
}

IPreprocessor* PluginManager::getPreprocessor(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = preprocessors_.find(name);
    if (it != preprocessors_.end()) {
        return it->second.get();
    }
    return nullptr;
}

void PluginManager::loadPluginsFromDirectory(const std::string& directory) {
    namespace fs = std::filesystem;
    if (!fs::exists(directory)) {
        return;
    }
    Logger::instance().log(Logger::Level::Info, "Scanning plugin directory: " + directory);
}

} // namespace mrn
