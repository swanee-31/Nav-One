#pragma once

#include "../plugin_api/IPlugin.hpp"
#include "../core/NavData.hpp"
#include <vector>
#include <string>
#include <map>

#ifdef _WIN32
    #include <windows.h>
    using PluginHandle = HMODULE;
#else
    using PluginHandle = void*;
#endif

namespace App {

struct LoadedPlugin {
    std::string path;
    PluginHandle handle;
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
    
    const std::vector<LoadedPlugin>& getPlugins() const { return _plugins; }
    std::vector<LoadedPlugin>& getPlugins() { return _plugins; }

private:
    std::vector<LoadedPlugin> _plugins;
};

} // namespace App
