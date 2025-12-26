#pragma once
#include "imgui.h"
#include "simulator/ISimulator.hpp"
#include "utils/ConfigManager.hpp"
#include <vector>
#include <string>

namespace Gui {

class SimulatorWindow {
public:
    SimulatorWindow(Simulator::ISimulator& sim) : _simulator(sim) {}

    void show() { _visible = true; }
    bool isVisible() const { return _visible; }
    void toggle() { _visible = !_visible; }

    void render() {
        if (!_visible) return;

        ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Simulator Configuration", &_visible)) {
            
            auto config = _simulator.getConfig();
            bool changed = false;

            if (ImGui::BeginTabBar("SimTabs")) {
                if (ImGui::BeginTabItem("General")) {
                    ImGui::Text("Simulation Modules");
                    ImGui::Separator();
                    
                    if (ImGui::Checkbox("Enable GPS Simulation", &config.enableGps)) changed = true;
                    if (renderFrequencyCombo("GPS Freq", config.gpsFrequency)) changed = true;

                    if (ImGui::Checkbox("Enable Wind Simulation", &config.enableWind)) changed = true;
                    if (renderFrequencyCombo("Wind Freq", config.windFrequency)) changed = true;

                    if (ImGui::Checkbox("Enable Water Simulation", &config.enableWater)) changed = true;
                    if (renderFrequencyCombo("Water Freq", config.waterFrequency)) changed = true;

                    ImGui::Spacing();
                    ImGui::Text("GPS Parameters");
                    ImGui::Separator();

                    if (ImGui::InputDouble("Start Latitude", &config.startLatitude, 0.0001, 1.0, "%.6f")) {
                        changed = true;
                        _simulator.setPosition(config.startLatitude, config.startLongitude);
                    }
                    if (ImGui::InputDouble("Start Longitude", &config.startLongitude, 0.0001, 1.0, "%.6f")) {
                        changed = true;
                        _simulator.setPosition(config.startLatitude, config.startLongitude);
                    }
                    
                    if (ImGui::InputDouble("Base Speed (kn)", &config.baseSpeed, 0.1, 1.0, "%.1f")) changed = true;
                    if (ImGui::InputDouble("Base Course (deg)", &config.baseCourse, 1.0, 10.0, "%.1f")) changed = true;

                    ImGui::Spacing();
                    ImGui::Text("Water Parameters");
                    ImGui::Separator();
                    
                    float minDepth = (float)config.minDepth;
                    float maxDepth = (float)config.maxDepth;
                    if (ImGui::DragFloatRange2("Depth Range (m)", &minDepth, &maxDepth, 0.1f, 0.0f, 1000.0f, "Min: %.1f", "Max: %.1f")) {
                        config.minDepth = (double)minDepth;
                        config.maxDepth = (double)maxDepth;
                        changed = true;
                    }

                    float minTemp = (float)config.minWaterTemp;
                    float maxTemp = (float)config.maxWaterTemp;
                    if (ImGui::DragFloatRange2("Water Temp Range (C)", &minTemp, &maxTemp, 0.1f, -5.0f, 40.0f, "Min: %.1f", "Max: %.1f")) {
                        config.minWaterTemp = (double)minTemp;
                        config.maxWaterTemp = (double)maxTemp;
                        changed = true;
                    }
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("AIS Targets")) {
                    if (ImGui::Checkbox("Enable AIS", &config.enableAis)) changed = true;
                    ImGui::Separator();

                    for (size_t i = 0; i < config.aisTargets.size(); ++i) {
                        auto& target = config.aisTargets[i];
                        ImGui::PushID((int)i);
                        
                        bool open = ImGui::TreeNode(target.name.c_str());
                        ImGui::SameLine();
                        if (ImGui::Checkbox("##Enabled", &target.enabled)) changed = true;

                        if (open) {
                            if (ImGui::InputDouble("Lat", &target.latitude, 0.0001, 0.0, "%.6f")) changed = true;
                            if (ImGui::InputDouble("Lon", &target.longitude, 0.0001, 0.0, "%.6f")) changed = true;
                            if (ImGui::InputDouble("Speed (kn)", &target.speed, 0.1, 1.0, "%.1f")) changed = true;
                            if (ImGui::InputDouble("Course", &target.course, 1.0, 10.0, "%.1f")) changed = true;
                            
                            // Frequency Combo
                            const std::vector<int> aisFreqs = {10000, 30000, 60000, 120000, 180000};
                            const std::vector<std::string> aisFreqLabels = {"10s", "30s", "1mn", "2mn", "3mn"};
                            
                            std::string currentLabel = "Custom";
                            for(size_t k=0; k<aisFreqs.size(); ++k) {
                                if (target.updateFrequency == aisFreqs[k]) currentLabel = aisFreqLabels[k];
                            }
                            
                            if (ImGui::BeginCombo("Update Freq", currentLabel.c_str())) {
                                for(size_t k=0; k<aisFreqs.size(); ++k) {
                                    bool isSelected = (target.updateFrequency == aisFreqs[k]);
                                    if (ImGui::Selectable(aisFreqLabels[k].c_str(), isSelected)) {
                                        target.updateFrequency = aisFreqs[k];
                                        changed = true;
                                    }
                                }
                                ImGui::EndCombo();
                            }

                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

            if (changed) {
                _simulator.setConfig(config);
                Utils::ConfigManager::instance().setSimulatorConfig(config);
                Utils::ConfigManager::instance().save();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Note: Speed and Course will vary +/- 10%% automatically.");
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Wind will rotate 360 deg every minute.");

        }
        ImGui::End();
    }

private:
    bool _visible = false;
    Simulator::ISimulator& _simulator;

    bool renderFrequencyCombo(const char* label, int& currentFreq) {
        const std::vector<int> freqs = {100, 200, 300, 400, 500, 800, 1000, 1500, 2000};
        bool changed = false;
        
        std::string preview = std::to_string(currentFreq) + " ms";
        if (ImGui::BeginCombo(label, preview.c_str())) {
            for (int freq : freqs) {
                bool isSelected = (currentFreq == freq);
                std::string itemLabel = std::to_string(freq) + " ms";
                if (ImGui::Selectable(itemLabel.c_str(), isSelected)) {
                    currentFreq = freq;
                    changed = true;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        return changed;
    }
};

} // namespace Gui
