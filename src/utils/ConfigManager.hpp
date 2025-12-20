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

private:
    ConfigManager() = default;
    std::vector<App::DataSourceConfig> sources;
};

} // namespace Utils
