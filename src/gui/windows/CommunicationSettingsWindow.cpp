#include "CommunicationSettingsWindow.hpp"
#include "utils/SerialPortUtils.hpp"
#include <chrono>
#include <string>

namespace Gui {

CommunicationSettingsWindow::CommunicationSettingsWindow(App::ServiceManager& manager) 
    : serviceManager(manager) {}

void CommunicationSettingsWindow::render() {
    if (!visible) return;

    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Communication Settings", &visible)) {
        
        auto& sources = serviceManager.getSources();

        // Split window into two columns
        ImGui::Columns(2, "SettingsColumns", true);
        
        // Left Column: List of Sources
        ImGui::Text("Data Sources");
        ImGui::Separator();
        
        static int selectedSourceIdx = -1;
        
        for (int i = 0; i < sources.size(); i++) {
            if (ImGui::Selectable(sources[i].name.c_str(), selectedSourceIdx == i)) {
                selectedSourceIdx = i;
            }
        }
        
        if (ImGui::Button("Add Source")) {
            App::DataSourceConfig newSource;
            newSource.id = "SRC_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
            newSource.name = "New Source";
            newSource.type = App::SourceType::Serial;
            sources.push_back(newSource);
            selectedSourceIdx = sources.size() - 1;
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove") && selectedSourceIdx >= 0 && selectedSourceIdx < sources.size()) {
            serviceManager.stopService(sources[selectedSourceIdx].id);
            sources.erase(sources.begin() + selectedSourceIdx);
            selectedSourceIdx = -1;
        }

        ImGui::NextColumn();
        
        // Right Column: Details
        if (selectedSourceIdx >= 0 && selectedSourceIdx < sources.size()) {
            App::DataSourceConfig& config = sources[selectedSourceIdx];
            
            ImGui::Text("Settings for: %s", config.name.c_str());
            ImGui::Separator();
            
            char nameBuf[64];
            strncpy(nameBuf, config.name.c_str(), sizeof(nameBuf));
            if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
                config.name = nameBuf;
            }

            bool enabled = config.enabled;
            if (ImGui::Checkbox("Enabled", &enabled)) {
                config.enabled = enabled;
                serviceManager.updateServiceState(config);
            }

            // Source Type Combo
            const char* types[] = { "Serial", "UDP" };
            int currentType = (int)config.type;
            if (ImGui::Combo("Type", &currentType, types, IM_ARRAYSIZE(types))) {
                config.type = (App::SourceType)currentType;
                if (config.enabled) serviceManager.updateServiceState(config);
            }

            if (config.type == App::SourceType::Serial) {
                // Serial Settings
                static std::vector<Utils::SerialPortUtils::PortInfo> ports = Utils::SerialPortUtils::getAvailablePorts();
                if (ImGui::Button("Refresh Ports")) {
                    ports = Utils::SerialPortUtils::getAvailablePorts();
                }
                
                if (ImGui::BeginCombo("Port", config.portName.c_str())) {
                    for (const auto& port : ports) {
                        bool isSelected = (config.portName == port.port);
                        if (ImGui::Selectable(port.port.c_str(), isSelected)) {
                            config.portName = port.port;
                            if (config.enabled) serviceManager.updateServiceState(config);
                        }
                        if (isSelected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                
                if (ImGui::InputInt("Baud Rate", &config.baudRate)) {
                    if (config.enabled) serviceManager.updateServiceState(config);
                }

            } else if (config.type == App::SourceType::Udp) {
                // UDP Settings
                if (ImGui::InputInt("Port", &config.port)) {
                    if (config.enabled) serviceManager.updateServiceState(config);
                }
            }
            
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "ID: %s", config.id.c_str());
        } else {
            ImGui::Text("Select a source to edit");
        }

        ImGui::Columns(1);
        
        ImGui::Separator();
        if (ImGui::Button("Save Configuration")) {
            serviceManager.saveConfig();
        }

        ImGui::End();
    }
}

} // namespace Gui
