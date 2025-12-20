#pragma once

#include "app/DataSourceConfig.hpp"
#include "network/IService.hpp"
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace App {

class ServiceManager {
public:
    using LogCallback = std::function<void(const std::string& source, const std::string& frame)>;

    ServiceManager();
    ~ServiceManager();

    void setLogCallback(LogCallback callback) { logCallback = callback; }

    void loadConfig();
    void saveConfig();

    void updateServiceState(const DataSourceConfig& config);
    void stopService(const std::string& id);
    void stopAll();

    std::vector<DataSourceConfig>& getSources() { return sources; }
    const std::vector<DataSourceConfig>& getSources() const { return sources; }
    const std::map<std::string, std::unique_ptr<Network::IService>>& getActiveServices() const { return activeServices; }

private:
    std::vector<DataSourceConfig> sources;
    std::map<std::string, std::unique_ptr<Network::IService>> activeServices;
    LogCallback logCallback;
};

} // namespace App
