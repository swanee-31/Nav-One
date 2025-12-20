#pragma once

#include "app/DataSourceConfig.hpp"
#include "network/IService.hpp"
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <mutex>

namespace App {

class ServiceManager {
public:
    using LogCallback = std::function<void(const std::string& source, const std::string& frame)>;

    ServiceManager();
    ~ServiceManager();

    void setLogCallback(LogCallback callback) { 
        std::lock_guard<std::recursive_mutex> lock(mutex);
        logCallback = callback; 
    }

    void loadConfig();
    void saveConfig();

    void updateServiceState(const DataSourceConfig& config);
    void stopService(const std::string& id);
    void stopAll();

    // Thread-safe accessors for GUI
    std::vector<DataSourceConfig>& getSources() { return sources; }
    std::vector<DataOutputConfig>& getOutputs() { return outputs; }
    
    // Locking helper for GUI
    std::unique_lock<std::recursive_mutex> getLock() const { return std::unique_lock<std::recursive_mutex>(mutex); }

    const std::map<std::string, std::unique_ptr<Network::IService>>& getActiveServices() const { return activeServices; }
    const std::map<std::string, std::unique_ptr<Network::IService>>& getActiveOutputs() const { return activeOutputs; }

    void updateOutputState(const DataOutputConfig& config);
    void stopOutput(const std::string& id);

    // Multiplexing
    void broadcast(const std::string& data, const std::string& sourceId);

    bool isSourceEnabled(const std::string& id) const;

private:
    mutable std::recursive_mutex mutex;
    std::vector<DataSourceConfig> sources;
    std::vector<DataOutputConfig> outputs;
    
    std::map<std::string, std::unique_ptr<Network::IService>> activeServices;
    std::map<std::string, std::unique_ptr<Network::IService>> activeOutputs;
    
    LogCallback logCallback;
};

} // namespace App
