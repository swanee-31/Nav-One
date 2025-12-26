#pragma once
#include "imgui.h"
#include <string>

#ifndef NAVONE_GIT_VERSION
#define NAVONE_GIT_VERSION "Unknown"
#endif

namespace Gui {

class AboutWindow {
public:
    void show() { _visible = true; }
    bool isVisible() const { return _visible; }
    void toggle() { _visible = !_visible; }

    void render() {
        if (!_visible) return;

        ImGui::OpenPopup("About NavOne");

        if (ImGui::BeginPopupModal("About NavOne", &_visible, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("NavOne - Navigation Hub");
            ImGui::Separator();
            ImGui::Text("Version: %s", NAVONE_GIT_VERSION);
            ImGui::Text("Built on: %s %s", __DATE__, __TIME__);
            ImGui::Separator();
            ImGui::Text("Developed by: Fabrice Meynckens");
            ImGui::Text("License: MIT");
            
            ImGui::Spacing();
            
            if (ImGui::Button("Close", ImVec2(120, 0))) { 
                _visible = false; 
                ImGui::CloseCurrentPopup(); 
            }
            ImGui::EndPopup();
        }
    }

private:
    bool _visible = false;
};

} // namespace Gui
