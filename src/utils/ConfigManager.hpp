#pragma once

#include <string>
#include <vector>
#include "../app/DataSourceConfig.hpp"

namespace Utils {

class ConfigManager {
public:
    static ConfigManager& instance() {
        static ConfigManager instance;
        return instance;
    }

    void load(const std::string& filename = "nav-one.xml");
    void save(const std::string& filename = "nav-one.xml");

    // App Configuration
    std::vector<App::DataSourceConfig> getSources() const { return sources; }
    void setSources(const std::vector<App::DataSourceConfig>& newSources) { sources = newSources; }

    std::vector<App::DataOutputConfig> getOutputs() const { return outputs; }
    void setOutputs(const std::vector<App::DataOutputConfig>& newOutputs) { outputs = newOutputs; }

    App::DisplayConfig getDisplayConfig() const { return displayConfig; }
    void setDisplayConfig(const App::DisplayConfig& config) { displayConfig = config; }

private:
    ConfigManager() = default;
    std::vector<App::DataSourceConfig> sources;
    std::vector<App::DataOutputConfig> outputs;
    App::DisplayConfig displayConfig;
};

} // namespace Utils
