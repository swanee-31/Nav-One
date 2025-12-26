#pragma once

#include <string>
#include <vector>
#include "../app/DataSourceConfig.hpp"
#include "../simulator/SimulatorConfig.hpp"

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
    std::vector<App::DataSourceConfig> getSources() const { return _sources; }
    void setSources(const std::vector<App::DataSourceConfig>& newSources) { _sources = newSources; }

    std::vector<App::DataOutputConfig> getOutputs() const { return _outputs; }
    void setOutputs(const std::vector<App::DataOutputConfig>& newOutputs) { _outputs = newOutputs; }

    App::DisplayConfig getDisplayConfig() const { return _displayConfig; }
    void setDisplayConfig(const App::DisplayConfig& config) { _displayConfig = config; }

    Simulator::SimulatorConfig getSimulatorConfig() const { return _simulatorConfig; }
    void setSimulatorConfig(const Simulator::SimulatorConfig& config) { _simulatorConfig = config; }

private:
    ConfigManager() = default;
    std::vector<App::DataSourceConfig> _sources;
    std::vector<App::DataOutputConfig> _outputs;
    App::DisplayConfig _displayConfig;
    Simulator::SimulatorConfig _simulatorConfig;
};

} // namespace Utils
