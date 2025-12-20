#include "../../plugin_api/IPlugin.hpp"
#include "imgui.h"
#include <string>
#include <cmath>

class GpsBigPlugin : public PluginApi::IPlugin {
public:
    const char* getName() const override { return "GPS Big Display"; }
    const char* getVersion() const override { return "1.0.0"; }

    void init(const PluginApi::PluginContext& context) override {
        ImGui::SetCurrentContext(context.imguiContext);
    }

    void render(const Core::NavData& data) override {
        if (ImGui::Begin("GPS Big")) {
            
            // Auto-scaling based on window width
            // Base width 300px = Scale 1.0
            float windowWidth = ImGui::GetContentRegionAvail().x;
            float scale = windowWidth / 250.0f; 
            if (scale < 0.5f) scale = 0.5f;
            
            ImGui::SetWindowFontScale(scale);

            // Status Indicator
            if (!data.isGpsValid) {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 50, 50, 255)); // Red
                ImGui::Text("VOID");
                ImGui::PopStyleColor();
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(50, 255, 50, 255)); // Green
                ImGui::Text("ACTIVE");
                ImGui::PopStyleColor();
            }

            ImGui::Separator();

            // Use a monospaced look if possible, or just standard font
            // LAT
            char latDir = (data.latitude >= 0) ? 'N' : 'S';
            ImGui::Text("LAT: %.4f %c", std::abs(data.latitude), latDir);

            // LON
            char lonDir = (data.longitude >= 0) ? 'E' : 'W';
            ImGui::Text("LON: %.4f %c", std::abs(data.longitude), lonDir);
            
            ImGui::Separator();

            // SOG
            ImGui::Text("SOG: %5.1f kn", data.speedOverGround);

            // COG
            ImGui::Text("COG: %05.1f deg", data.courseOverGround);
            
            // Reset scale for other widgets in this window if any (none here)
            ImGui::SetWindowFontScale(1.0f);

            ImGui::End();
        }
    }

    void shutdown() override {}
};

extern "C" {
    __declspec(dllexport) PluginApi::IPlugin* createPlugin() {
        return new GpsBigPlugin();
    }

    __declspec(dllexport) void destroyPlugin(PluginApi::IPlugin* plugin) {
        delete plugin;
    }
}
