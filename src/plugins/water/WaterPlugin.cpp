#include "../../plugin_api/IPlugin.hpp"
#include "imgui.h"
#include <vector>
#include <deque>
#include <algorithm>
#include <string>
#include <cmath>

class WaterPlugin : public PluginApi::IPlugin {
public:
    const char* getName() const override { return "Water Environment"; }
    const char* getVersion() const override { return "1.0.0"; }

    void init(const PluginApi::PluginContext& context) override {
        ImGui::SetCurrentContext(context.imguiContext);
    }

    void render(const Core::NavData& data) override {
        // Update history
        updateHistory(data);

        if (ImGui::Begin("Water Environment (Plugin)")) {
            
            // 1. Controls (Time Scale)
            ImGui::Text("Time Scale:");
            ImGui::SameLine();
            if (ImGui::RadioButton("1m", timeScaleMinutes == 1)) timeScaleMinutes = 1;
            ImGui::SameLine();
            if (ImGui::RadioButton("5m", timeScaleMinutes == 5)) timeScaleMinutes = 5;
            ImGui::SameLine();
            if (ImGui::RadioButton("15m", timeScaleMinutes == 15)) timeScaleMinutes = 15;

            // 2. Depth Graph
            renderDepthGraph();

            ImGui::Separator();

            // 3. Numeric Data
            ImGui::Columns(2, "water_data", false);
            
            // Depth
            ImGui::Text("Depth");
            ImGui::SetWindowFontScale(2.0f);
            if (data.hasDepth) {
                ImGui::Text("%.1f m", data.depth);
            } else {
                ImGui::Text("---");
            }
            ImGui::SetWindowFontScale(1.0f);
            
            ImGui::NextColumn();
            
            // Temperature
            ImGui::Text("Water Temp");
            ImGui::SetWindowFontScale(2.0f);
            if (data.hasWaterTemperature) {
                ImGui::Text("%.1f C", data.waterTemperature);
            } else {
                ImGui::Text("---");
            }
            ImGui::SetWindowFontScale(1.0f);
            
            ImGui::Columns(1);
            
            ImGui::Separator();
            
            // Speed Through Water
            if (data.hasWaterSpeed) {
                ImGui::Text("Speed Through Water: %.1f kn", data.speedThroughWater);
            } else {
                ImGui::Text("Speed Through Water: ---");
            }

            ImGui::End();
        }
    }

    void shutdown() override {
        depthHistory.clear();
    }

private:
    std::deque<float> depthHistory;
    int timeScaleMinutes = 1;
    double lastUpdateTime = 0.0;
    
    // Assuming update rate is roughly 1Hz or we sample at 1Hz for the graph
    // 15 minutes * 60 seconds = 900 points max
    const size_t MAX_HISTORY_SIZE = 900; 

    void updateHistory(const Core::NavData& data) {
        double currentTime = ImGui::GetTime();
        if (currentTime - lastUpdateTime >= 1.0) { // Sample every second
            lastUpdateTime = currentTime;
            
            if (data.hasDepth) {
                depthHistory.push_back((float)data.depth);
            } else {
                // Push last known or 0? Let's push last known if available, or 0.
                if (!depthHistory.empty()) depthHistory.push_back(depthHistory.back());
                else depthHistory.push_back(0.0f);
            }
            
            if (depthHistory.size() > MAX_HISTORY_SIZE) {
                depthHistory.pop_front();
            }
        }
    }

    void renderDepthGraph() {
        if (depthHistory.empty()) return;

        int pointsToShow = timeScaleMinutes * 60;
        if (pointsToShow > depthHistory.size()) pointsToShow = (int)depthHistory.size();
        
        // Create a vector for PlotLines
        std::vector<float> plotData;
        plotData.reserve(pointsToShow);
        
        // Get the last 'pointsToShow' elements
        auto it = depthHistory.end() - pointsToShow;
        float minVal = 10000.0f;
        float maxVal = -10000.0f;
        
        for (; it != depthHistory.end(); ++it) {
            float val = *it;
            plotData.push_back(val);
            if (val < minVal) minVal = val;
            if (val > maxVal) maxVal = val;
        }
        
        // Auto-scale with some padding
        if (minVal > maxVal) { minVal = 0.0f; maxVal = 10.0f; } // Default if empty or flat
        float range = maxVal - minVal;
        if (range < 1.0f) range = 1.0f; // Minimum range
        
        float graphMin = minVal - range * 0.1f;
        float graphMax = maxVal + range * 0.1f;
        if (graphMin < 0) graphMin = 0; // Depth usually positive

        std::string overlay = "Min: " + std::to_string((int)minVal) + "m | Max: " + std::to_string((int)maxVal) + "m";

        ImGui::PlotLines("##DepthGraph", plotData.data(), (int)plotData.size(), 0, overlay.c_str(), graphMin, graphMax, ImVec2(0, 150));
    }
};

// Export functions
extern "C" {
    __declspec(dllexport) PluginApi::IPlugin* createPlugin() {
        return new WaterPlugin();
    }

    __declspec(dllexport) void destroyPlugin(PluginApi::IPlugin* plugin) {
        delete plugin;
    }
}
