#include "PluginManager.hpp"
#include "imgui.h"
#include <iostream>
#include <filesystem>

namespace App {

PluginManager::PluginManager() {}

PluginManager::~PluginManager() {
    // Unload all plugins in reverse order
    for (auto it = _plugins.rbegin(); it != _plugins.rend(); ++it) {
        if (it->instance && it->destroyFunc) {
            it->instance->shutdown();
            it->destroyFunc(it->instance);
        }
        if (it->handle) {
            FreeLibrary(it->handle);
        }
    }
    _plugins.clear();
}

void PluginManager::loadPlugin(const std::string& path) {
    // Check if already loaded
    for (const auto& p : _plugins) {
        if (p.path == path) return;
    }

    HMODULE handle = LoadLibraryA(path.c_str());
    if (!handle) {
        std::cerr << "Failed to load plugin DLL: " << path << " Error: " << GetLastError() << std::endl;
        return;
    }

    auto createFunc = (PluginApi::CreatePluginFunc)GetProcAddress(handle, "createPlugin");
    auto destroyFunc = (PluginApi::DestroyPluginFunc)GetProcAddress(handle, "destroyPlugin");

    if (!createFunc || !destroyFunc) {
        std::cerr << "Invalid plugin DLL (missing factory functions): " << path << std::endl;
        FreeLibrary(handle);
        return;
    }

    PluginApi::IPlugin* instance = createFunc();
    if (!instance) {
        std::cerr << "Failed to create plugin instance: " << path << std::endl;
        FreeLibrary(handle);
        return;
    }

    // Initialize Plugin
    PluginApi::PluginContext ctx;
    ctx.imguiContext = ImGui::GetCurrentContext();
    instance->init(ctx);

    LoadedPlugin plugin;
    plugin.path = path;
    plugin.handle = handle;
    plugin.instance = instance;
    plugin.destroyFunc = destroyFunc;
    plugin.active = true;

    _plugins.push_back(plugin);
    std::cout << "Loaded plugin: " << instance->getName() << " (" << instance->getVersion() << ")" << std::endl;
}

void PluginManager::unloadPlugin(const std::string& path) {
    auto it = std::find_if(_plugins.begin(), _plugins.end(), 
        [&path](const LoadedPlugin& p) { return p.path == path; });

    if (it != _plugins.end()) {
        if (it->instance) {
            it->instance->shutdown();
            it->destroyFunc(it->instance);
        }
        if (it->handle) {
            FreeLibrary(it->handle);
        }
        _plugins.erase(it);
    }
}

void PluginManager::renderPlugins(const Core::NavData& data) {
    for (auto& plugin : _plugins) {
        if (plugin.active && plugin.instance) {
            plugin.instance->render(data);
        }
    }
}

} // namespace App
