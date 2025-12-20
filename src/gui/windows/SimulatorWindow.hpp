#pragma once
#include "imgui.h"
#include "core/Simulator.hpp"

namespace Gui {

class SimulatorWindow {
public:
    SimulatorWindow(Core::Simulator& sim) : simulator(sim) {}

    void show() { visible = true; }
    bool isVisible() const { return visible; }
    void toggle() { visible = !visible; }

    void render() {
        if (!visible) return;

        ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Simulator Configuration", &visible)) {
            
            auto config = simulator.getConfig();
            bool changed = false;

            ImGui::Text("Simulation Modules");
            ImGui::Separator();
            
            if (ImGui::Checkbox("Enable GPS Simulation", &config.enableGps)) changed = true;
            if (ImGui::Checkbox("Enable Wind Simulation", &config.enableWind)) changed = true;

            ImGui::Spacing();
            ImGui::Text("GPS Parameters");
            ImGui::Separator();

            if (ImGui::InputDouble("Start Latitude", &config.startLatitude, 0.0001, 1.0, "%.6f")) {
                changed = true;
                simulator.setPosition(config.startLatitude, config.startLongitude);
            }
            if (ImGui::InputDouble("Start Longitude", &config.startLongitude, 0.0001, 1.0, "%.6f")) {
                changed = true;
                simulator.setPosition(config.startLatitude, config.startLongitude);
            }
            
            if (ImGui::InputDouble("Base Speed (kn)", &config.baseSpeed, 0.1, 1.0, "%.1f")) changed = true;
            if (ImGui::InputDouble("Base Course (deg)", &config.baseCourse, 1.0, 10.0, "%.1f")) changed = true;

            if (changed) {
                simulator.setConfig(config);
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Note: Speed and Course will vary +/- 10%% automatically.");
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Wind will rotate 360 deg every minute.");

            ImGui::End();
        }
    }

private:
    bool visible = false;
    Core::Simulator& simulator;
};

} // namespace Gui
