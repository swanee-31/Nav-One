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
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _logCallback = callback; 
    }

    void loadConfig();
    void saveConfig();

    void updateServiceState(const DataSourceConfig& config);
    void stopService(const std::string& id);
    void stopAll();

    // Thread-safe accessors for GUI
    std::vector<DataSourceConfig>& getSources() { return _sources; }
    const std::vector<DataSourceConfig>& getSources() const { return _sources; }
    
    std::vector<DataOutputConfig>& getOutputs() { return _outputs; }
    const std::vector<DataOutputConfig>& getOutputs() const { return _outputs; }
    
    // Locking helper for GUI
    std::unique_lock<std::recursive_mutex> getLock() const { return std::unique_lock<std::recursive_mutex>(_mutex); }

    const std::map<std::string, std::unique_ptr<Network::IService>>& getActiveServices() const { return _activeServices; }
    const std::map<std::string, std::unique_ptr<Network::IService>>& getActiveOutputs() const { return _activeOutputs; }

    void updateOutputState(const DataOutputConfig& config);
    void stopOutput(const std::string& id);

    // Multiplexing
    void broadcast(const std::string& data, const std::string& sourceId);

    bool isSourceEnabled(const std::string& id) const;

private:
    mutable std::recursive_mutex _mutex;
    std::vector<DataSourceConfig> _sources;
    std::vector<DataOutputConfig> _outputs;
    
    std::map<std::string, std::unique_ptr<Network::IService>> _activeServices;
    std::map<std::string, std::unique_ptr<Network::IService>> _activeOutputs;
    
    LogCallback _logCallback;
};

} // namespace App
