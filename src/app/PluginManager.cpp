#include "PluginManager.hpp"
#include "imgui.h"
#include <iostream>
#include <filesystem>

#ifndef _WIN32
#include <dlfcn.h>
#endif

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
#ifdef _WIN32
            FreeLibrary(it->handle);
#else
            dlclose(it->handle);
#endif
        }
    }
    _plugins.clear();
}

void PluginManager::loadPlugin(const std::string& path) {
    // Check if already loaded
    for (const auto& p : _plugins) {
        if (p.path == path) return;
    }

    PluginHandle handle = nullptr;
    PluginApi::CreatePluginFunc createFunc = nullptr;
    PluginApi::DestroyPluginFunc destroyFunc = nullptr;

#ifdef _WIN32
    handle = LoadLibraryA(path.c_str());
    if (!handle) {
        std::cerr << "Failed to load plugin DLL: " << path << " Error: " << GetLastError() << std::endl;
        return;
    }

    createFunc = (PluginApi::CreatePluginFunc)GetProcAddress(handle, "createPlugin");
    destroyFunc = (PluginApi::DestroyPluginFunc)GetProcAddress(handle, "destroyPlugin");
#else
    handle = dlopen(path.c_str(), RTLD_NOW);
    if (!handle) {
        std::cerr << "Failed to load plugin SO: " << path << " Error: " << dlerror() << std::endl;
        return;
    }

    createFunc = (PluginApi::CreatePluginFunc)dlsym(handle, "createPlugin");
    destroyFunc = (PluginApi::DestroyPluginFunc)dlsym(handle, "destroyPlugin");
#endif

    if (!createFunc || !destroyFunc) {
        std::cerr << "Invalid plugin (missing factory functions): " << path << std::endl;
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return;
    }

    PluginApi::IPlugin* instance = createFunc();
    if (!instance) {
        std::cerr << "Failed to create plugin instance: " << path << std::endl;
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
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
#ifdef _WIN32
            FreeLibrary(it->handle);
#else
            dlclose(it->handle);
#endif
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
