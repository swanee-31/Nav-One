#pragma once

#include "imgui.h"
#include "../core/NavData.hpp"
#include <string>

#ifdef _WIN32
    #define PLUGIN_EXPORT __declspec(dllexport)
#else
    #define PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

namespace PluginApi {

struct PluginContext {
    ImGuiContext* imguiContext;
};

class IPlugin {
public:
    virtual ~IPlugin() = default;
    
    virtual const char* getName() const = 0;
    virtual const char* getVersion() const = 0;
    
    virtual void init(const PluginContext& context) = 0;
    virtual void render(const Core::NavData& data) = 0;
    virtual void shutdown() = 0;
};

// Factory function types
typedef IPlugin* (*CreatePluginFunc)();
typedef void (*DestroyPluginFunc)(IPlugin*);

} // namespace PluginApi
