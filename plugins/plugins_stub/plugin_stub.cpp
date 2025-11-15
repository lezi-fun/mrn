#include <memory>

#include "core/plugin_manager.h"

using namespace mrn;

class StubPreprocessor : public IPreprocessor {
public:
    std::vector<uint8_t> process(const std::vector<uint8_t>& data) override {
        return data;
    }
    std::vector<uint8_t> inverseProcess(const std::vector<uint8_t>& data) override {
        return data;
    }
};

static struct StubPluginLoader {
    StubPluginLoader() {
        PluginManager::getInstance().registerPreprocessor("stub", std::make_unique<StubPreprocessor>());
    }
} stubPluginLoader;
