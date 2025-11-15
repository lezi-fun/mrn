#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "core/plugin_interface.h"

namespace mrn {

class PluginManager {
public:
    static PluginManager& getInstance();

    bool registerAlgorithm(const std::string& name,
                           std::unique_ptr<ICompressionAlgorithm> algorithm);
    bool registerPreprocessor(const std::string& name,
                              std::unique_ptr<IPreprocessor> preprocessor);

    std::vector<std::string> getAvailableAlgorithms() const;
    std::vector<std::string> getAvailablePreprocessors() const;

    ICompressionAlgorithm* getAlgorithm(const std::string& name);
    IPreprocessor* getPreprocessor(const std::string& name);

    void loadPluginsFromDirectory(const std::string& directory);

private:
    PluginManager() = default;

    mutable std::mutex mutex_;
    std::map<std::string, std::unique_ptr<ICompressionAlgorithm>> algorithms_;
    std::map<std::string, std::unique_ptr<IPreprocessor>> preprocessors_;
};

} // namespace mrn
