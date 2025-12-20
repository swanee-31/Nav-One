#include "NavOneApp.hpp"
#include "imgui.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace App {

NavOneApp::NavOneApp(Core::ThreadPool& pool) 
    : Gui::MainWindow(1280, 720, "NavOne - Navigation Hub"), 
      threadPool(pool),
      configWindow(serviceManager),
      simulatorWindow(simulator) {
    
    // Setup Service Manager Logging
    serviceManager.setLogCallback([this](const std::string& source, const std::string& frame) {
        monitorWindow.addLog(source, frame);
    });

    // Load Config
    serviceManager.loadConfig();
    
    // Subscribe to MessageBus
    busListenerId = Core::MessageBus::instance().subscribe([this](const Core::NavData& update) {
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            currentData = update;
        }
        dashboardWindow.updateData(update);
    });

    // Start a background task to simulate data acquisition (publishing to bus)
    threadPool.enqueue([this] {
        while(running) {
            if (isSimulatorActive) {
                // Update Simulator Physics (100ms step)
                simulator.update(0.1);
                
                // Only publish if Simulator is enabled as a Source in ServiceManager
                if (serviceManager.isSourceEnabled("SIMULATOR")) {
                    // Get Data
                    Core::NavData simData = simulator.getCurrentData();
                    
                    // Log simulated frames
                    auto sentences = simulator.getNmeaSentences();
                    for (const auto& sentence : sentences) {
                        monitorWindow.addLog("SIMULATOR", sentence);
                        // Broadcast to outputs
                        serviceManager.broadcast(sentence + "\r\n", "SIMULATOR");
                    }

                    Core::MessageBus::instance().publish(simData);
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
}

NavOneApp::~NavOneApp() {
    running = false;
    Core::MessageBus::instance().unsubscribe(busListenerId);
    serviceManager.stopAll();
}

bool NavOneApp::init() {
    if (!Gui::MainWindow::init()) return false;
    
    // Apply Display Settings after ImGui context is created
    displaySettingsWindow.applyConfig();
    
    return true;
}

void NavOneApp::render() {
    // Menu Bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Configuration")) {
            if (ImGui::MenuItem("Communication")) {
                configWindow.show();
            }
            if (ImGui::MenuItem("Display")) {
                displaySettingsWindow.show();
            }
            if (ImGui::MenuItem("NMEA Monitor", nullptr, monitorWindow.isVisible())) {
                monitorWindow.toggle();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Plugins")) {
            if (ImGui::MenuItem("Load GPS Plugin")) {
                // In a real app, we would scan the directory. 
                // Here we hardcode the path relative to the executable for the demo.
                // CMake puts it in src/plugins/
                pluginManager.loadPlugin("GpsPlugin.dll");
            }
            if (ImGui::MenuItem("Load GPS Big Plugin")) {
                pluginManager.loadPlugin("GpsBigPlugin.dll");
            }
            if (ImGui::MenuItem("Load Wind Plugin")) {
                pluginManager.loadPlugin("WindPlugin.dll");
            }
            
            ImGui::Separator();
            
            auto& plugins = pluginManager.getPlugins();
            for (auto& plugin : plugins) {
                if (ImGui::MenuItem(plugin.instance->getName(), nullptr, plugin.active)) {
                    plugin.active = !plugin.active;
                }
            }
            
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Simulator")) {
            bool active = isSimulatorActive;
            if (ImGui::MenuItem(active ? "Stop Simulator" : "Start Simulator")) {
                isSimulatorActive = !active;
            }
            if (ImGui::MenuItem("Configure Simulator")) {
                simulatorWindow.show();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    
    // Render Windows
    configWindow.render();
    displaySettingsWindow.render();
    simulatorWindow.render();
    monitorWindow.render();
    dashboardWindow.render(threadPool, serviceManager);
    
    // Render Plugins
    {
        std::lock_guard<std::mutex> lock(dataMutex);
        pluginManager.renderPlugins(currentData);
    }
}

} // namespace App
