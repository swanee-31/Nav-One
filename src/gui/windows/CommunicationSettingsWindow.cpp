#include "CommunicationSettingsWindow.hpp"
#include "utils/SerialPortUtils.hpp"
#include <chrono>
#include <string>
#include <algorithm>

namespace Gui {

CommunicationSettingsWindow::CommunicationSettingsWindow(App::ServiceManager& manager) 
    : serviceManager(manager) {}

void CommunicationSettingsWindow::render() {
    if (!visible) return;

    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Communication Settings", &visible)) {
        auto lock = serviceManager.getLock();
        
        if (ImGui::BeginTabBar("CommSettingsTabBar")) {
            
            // --- INPUTS TAB ---
            if (ImGui::BeginTabItem("Inputs")) {
                auto& sources = serviceManager.getSources();

                // Split window into two columns
                ImGui::Columns(2, "InputColumns", true);
                
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
                    const char* types[] = { "Serial", "UDP", "Simulator" };
                    int currentType = (int)config.type;
                    if (ImGui::Combo("Type", &currentType, types, IM_ARRAYSIZE(types))) {
                        config.type = (App::SourceType)currentType;
                        if (config.enabled) serviceManager.updateServiceState(config);
                    }

                    if (config.type == App::SourceType::Simulator) {
                        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Simulator settings are available in the 'Simulator' menu.");
                    } else if (config.type == App::SourceType::Serial) {
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
                        
                        const int baudRates[] = { 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200 };
                        const char* baudRateLabels[] = { "1200", "2400", "4800", "9600", "19200", "38400", "57600", "115200" };
                        
                        int selectedBaud = -1;
                        for (int i = 0; i < IM_ARRAYSIZE(baudRates); i++) {
                            if (config.baudRate == baudRates[i]) {
                                selectedBaud = i;
                                break;
                            }
                        }

                        if (ImGui::Combo("Baud Rate", &selectedBaud, baudRateLabels, IM_ARRAYSIZE(baudRateLabels))) {
                            if (selectedBaud >= 0) {
                                config.baudRate = baudRates[selectedBaud];
                                if (config.enabled) serviceManager.updateServiceState(config);
                            }
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
                ImGui::EndTabItem();
            }

            // --- OUTPUTS TAB ---
            if (ImGui::BeginTabItem("Outputs")) {
                auto& outputs = serviceManager.getOutputs();

                ImGui::Columns(2, "OutputColumns", true);
                
                ImGui::Text("Data Outputs");
                ImGui::Separator();
                
                static int selectedOutputIdx = -1;
                
                for (int i = 0; i < outputs.size(); i++) {
                    if (ImGui::Selectable(outputs[i].name.c_str(), selectedOutputIdx == i)) {
                        selectedOutputIdx = i;
                    }
                }
                
                if (ImGui::Button("Add Output")) {
                    App::DataOutputConfig newOutput;
                    newOutput.id = "OUT_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
                    newOutput.name = "New Output";
                    newOutput.type = App::OutputType::Serial;
                    outputs.push_back(newOutput);
                    selectedOutputIdx = outputs.size() - 1;
                }
                ImGui::SameLine();
                if (ImGui::Button("Remove") && selectedOutputIdx >= 0 && selectedOutputIdx < outputs.size()) {
                    serviceManager.stopOutput(outputs[selectedOutputIdx].id);
                    outputs.erase(outputs.begin() + selectedOutputIdx);
                    selectedOutputIdx = -1;
                }

                ImGui::NextColumn();
                
                if (selectedOutputIdx >= 0 && selectedOutputIdx < outputs.size()) {
                    App::DataOutputConfig& config = outputs[selectedOutputIdx];
                    
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
                        serviceManager.updateOutputState(config);
                    }

                    const char* types[] = { "Serial", "UDP" };
                    int currentType = (int)config.type;
                    if (ImGui::Combo("Type", &currentType, types, IM_ARRAYSIZE(types))) {
                        config.type = (App::OutputType)currentType;
                        if (config.enabled) serviceManager.updateOutputState(config);
                    }

                    if (config.type == App::OutputType::Serial) {
                        static std::vector<Utils::SerialPortUtils::PortInfo> ports = Utils::SerialPortUtils::getAvailablePorts();
                        if (ImGui::Button("Refresh Ports")) {
                            ports = Utils::SerialPortUtils::getAvailablePorts();
                        }
                        
                        if (ImGui::BeginCombo("Port", config.portName.c_str())) {
                            for (const auto& port : ports) {
                                bool isSelected = (config.portName == port.port);
                                if (ImGui::Selectable(port.port.c_str(), isSelected)) {
                                    config.portName = port.port;
                                    if (config.enabled) serviceManager.updateOutputState(config);
                                }
                                if (isSelected) ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }
                        
                        if (ImGui::InputInt("Baud Rate", &config.baudRate)) {
                            if (config.enabled) serviceManager.updateOutputState(config);
                        }

                    } else if (config.type == App::OutputType::Udp) {
                        char addrBuf[64];
                        strncpy(addrBuf, config.address.c_str(), sizeof(addrBuf));
                        if (ImGui::InputText("Target IP", addrBuf, sizeof(addrBuf))) {
                            config.address = addrBuf;
                            if (config.enabled) serviceManager.updateOutputState(config);
                        }

                        if (ImGui::InputInt("Target Port", &config.port)) {
                            if (config.enabled) serviceManager.updateOutputState(config);
                        }
                    }

                    ImGui::Separator();
                    ImGui::Text("Multiplexing");
                    
                    if (ImGui::Checkbox("Multiplex All Sources", &config.multiplexAll)) {
                        // No immediate action needed, logic is in broadcast
                    }

                    if (!config.multiplexAll) {
                        ImGui::Indent();
                        ImGui::Text("Select Sources to Forward:");
                        
                        // Simulator
                        {
                            bool isSelected = false;
                            for (const auto& id : config.sourceIds) {
                                if (id == "SIMULATOR") {
                                    isSelected = true;
                                    break;
                                }
                            }
                            if (ImGui::Checkbox("Simulator", &isSelected)) {
                                if (isSelected) {
                                    config.sourceIds.push_back("SIMULATOR");
                                } else {
                                    auto it = std::remove(config.sourceIds.begin(), config.sourceIds.end(), "SIMULATOR");
                                    config.sourceIds.erase(it, config.sourceIds.end());
                                }
                            }
                        }

                        const auto& sources = serviceManager.getSources();
                        for (const auto& source : sources) {
                            bool isSelected = false;
                            for (const auto& id : config.sourceIds) {
                                if (id == source.id) {
                                    isSelected = true;
                                    break;
                                }
                            }

                            if (ImGui::Checkbox(source.name.c_str(), &isSelected)) {
                                if (isSelected) {
                                    config.sourceIds.push_back(source.id);
                                } else {
                                    auto it = std::remove(config.sourceIds.begin(), config.sourceIds.end(), source.id);
                                    config.sourceIds.erase(it, config.sourceIds.end());
                                }
                            }
                        }
                        ImGui::Unindent();
                    }
                    
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "ID: %s", config.id.c_str());
                } else {
                    ImGui::Text("Select an output to edit");
                }

                ImGui::Columns(1);
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
        
        ImGui::Separator();
        if (ImGui::Button("Save Configuration")) {
            serviceManager.saveConfig();
        }

        ImGui::End();
    }
}

} // namespace Gui
