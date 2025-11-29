#pragma once

#include "plugin_interface.h"
#include <map>
#include <memory>
#include <vector>
#include <string>

namespace mrn {

class PluginManager {
public:
    static PluginManager& getInstance();
    
    // 插件注册
    bool registerAlgorithm(const std::string& type, 
                          std::unique_ptr<ICompressionAlgorithm> algorithm);
    bool registerPreprocessor(const std::string& name,
                             std::unique_ptr<IPreprocessor> preprocessor);
    
    // 插件发现
    std::vector<std::string> getAvailableAlgorithms() const;
    std::vector<std::string> getAvailablePreprocessors() const;
    
    // 插件获取
    ICompressionAlgorithm* getAlgorithm(const std::string& name);
    IPreprocessor* getPreprocessor(const std::string& name);
    
    // 动态加载
    void loadPluginsFromDirectory(const std::string& directory);
    
    // 初始化内置插件
    void initializeBuiltinPlugins();
    
private:
    PluginManager() = default;
    ~PluginManager() = default;
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;
    
    std::map<std::string, std::unique_ptr<ICompressionAlgorithm>> algorithms_;
    std::map<std::string, std::unique_ptr<IPreprocessor>> preprocessors_;
};

} // namespace mrn
