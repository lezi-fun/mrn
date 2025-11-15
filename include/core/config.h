#pragma once

#include <map>
#include <string>

#include "core/compressor.h"

namespace mrn {

struct CompressionPreset {
    std::string name;
    CompressionPipeline pipeline;
    CompressionOptions options;

    static CompressionPreset createTextPreset();
    static CompressionPreset createBinaryPreset();
    static CompressionPreset createMaximumPreset();
    static CompressionPreset createFastPreset();
};

class ConfigurationManager {
public:
    void loadUserConfig(const std::string& configFile);
    void saveUserConfig(const std::string& configFile);

    CompressionPreset getPreset(const std::string& name);
    void addCustomPreset(const std::string& name, const CompressionPreset& preset);

    CompressionPreset detectBestPreset(const std::string& filename);

private:
    std::map<std::string, CompressionPreset> presets_;
    std::map<std::string, std::string> fileAssociations_;
};

} // namespace mrn
