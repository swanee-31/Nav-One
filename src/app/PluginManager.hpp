#pragma once

#include "../plugin_api/IPlugin.hpp"
#include "../core/NavData.hpp"
#include <vector>
#include <string>
#include <map>
#include <windows.h>

namespace App {

struct LoadedPlugin {
    std::string path;
    HMODULE handle;
    PluginApi::IPlugin* instance;
    PluginApi::DestroyPluginFunc destroyFunc;
    bool active = true;
};

class PluginManager {
public:
    PluginManager();
    ~PluginManager();

    void loadPlugin(const std::string& path);
    void unloadPlugin(const std::string& path);
    
    void renderPlugins(const Core::NavData& data);
    
    const std::vector<LoadedPlugin>& getPlugins() const { return plugins; }
    std::vector<LoadedPlugin>& getPlugins() { return plugins; }

private:
    std::vector<LoadedPlugin> plugins;
};

} // namespace App
