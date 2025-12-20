#pragma once
#include "imgui.h"
#include "utils/ConfigManager.hpp"

namespace Gui {

class DisplaySettingsWindow {
public:
    DisplaySettingsWindow() = default;

    void show() { visible = true; }
    bool isVisible() const { return visible; }

    void render() {
        if (!visible) return;

        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Display Settings", &visible)) {
            
            auto& configManager = Utils::ConfigManager::instance();
            auto config = configManager.getDisplayConfig();
            bool changed = false;

            ImGui::Text("Appearance");
            ImGui::Separator();

            // Theme Selection
            const char* themes[] = { "Dark", "Light", "Classic" };
            if (ImGui::Combo("Theme", &config.theme, themes, IM_ARRAYSIZE(themes))) {
                changed = true;
                applyTheme(config.theme);
            }

            // Font Scale
            if (ImGui::SliderFloat("Font Scale", &config.fontScale, 0.5f, 2.0f, "%.2f")) {
                changed = true;
                ImGui::GetIO().FontGlobalScale = config.fontScale;
            }

            ImGui::Separator();
            
            if (changed) {
                configManager.setDisplayConfig(config);
            }

            if (ImGui::Button("Save Configuration")) {
                configManager.save();
            }

            ImGui::End();
        }
    }

    void applyTheme(int themeIdx) {
        switch (themeIdx) {
            case 0: ImGui::StyleColorsDark(); break;
            case 1: ImGui::StyleColorsLight(); break;
            case 2: ImGui::StyleColorsClassic(); break;
            default: ImGui::StyleColorsDark(); break;
        }
    }

    void applyConfig() {
        auto config = Utils::ConfigManager::instance().getDisplayConfig();
        applyTheme(config.theme);
        ImGui::GetIO().FontGlobalScale = config.fontScale;
    }

private:
    bool visible = false;
};

} // namespace Gui
