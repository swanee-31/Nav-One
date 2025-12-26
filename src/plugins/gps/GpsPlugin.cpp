#include "../../plugin_api/IPlugin.hpp"
#include "imgui.h"
#include <string>
#include <iomanip>
#include <sstream>

class GpsPlugin : public PluginApi::IPlugin {
public:
    const char* getName() const override { return "GPS Data Display"; }
    const char* getVersion() const override { return "1.0.0"; }

    void init(const PluginApi::PluginContext& context) override {
        // Important: Set the ImGui context to match the host application
        ImGui::SetCurrentContext(context.imguiContext);
    }

    void render(const Core::NavData& data) override {
        if (ImGui::Begin("GPS Data (Plugin)")) {
            
            ImGui::Text("Source: %s", data.sourceId.c_str());
            ImGui::Separator();

            // Position
            ImGui::Text("Latitude:  %.6f", data.latitude);
            ImGui::Text("Longitude: %.6f", data.longitude);
            
            // Altitude
            ImGui::Text("Altitude:  %.1f m", data.altitude); 

            ImGui::Separator();

            // Speed / Course
            ImGui::Text("SOG: %.1f kn", data.speedOverGround);
            ImGui::Text("COG: %.1f deg", data.courseOverGround);

            ImGui::Separator();

            // Time
            auto time = std::chrono::system_clock::to_time_t(data.timestamp);
            std::tm tm = *std::gmtime(&time);
            ImGui::Text("UTC: %02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);

        }
        ImGui::End();
    }

    void shutdown() override {
        // Cleanup if necessary
    }
};

// Export Factory Functions
extern "C" {
    PLUGIN_EXPORT PluginApi::IPlugin* createPlugin() {
        return new GpsPlugin();
    }

    PLUGIN_EXPORT void destroyPlugin(PluginApi::IPlugin* plugin) {
        delete plugin;
    }
}
