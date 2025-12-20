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
      configWindow(serviceManager) {
    
    // Setup Service Manager Logging
    serviceManager.setLogCallback([this](const std::string& source, const std::string& frame) {
        monitorWindow.addLog(source, frame);
    });

    // Load Config
    serviceManager.loadConfig();

    // Subscribe to MessageBus
    busListenerId = Core::MessageBus::instance().subscribe([this](const Core::NavData& update) {
        dashboardWindow.updateData(update);
    });

    // Start a background task to simulate data acquisition (publishing to bus)
    threadPool.enqueue([this] {
        while(running) {
            if (isSimulatorActive) {
                Core::NavData simData;
                simData.sourceId = "SIMULATOR";
                simData.timestamp = std::chrono::system_clock::now();
                
                // Simulate some movement
                static double heading = 0.0;
                heading = fmod(heading + 1.0, 360.0);
                simData.heading = heading;
                simData.speedOverGround = 5.0 + (rand() % 100) / 10.0;

                // Log simulated frame (fake NMEA)
                std::stringstream ss;
                ss << "$GPHDT," << std::fixed << std::setprecision(1) << heading << ",T";
                monitorWindow.addLog("SIMULATOR", ss.str());

                Core::MessageBus::instance().publish(simData);
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

void NavOneApp::render() {
    // Menu Bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Configuration")) {
            if (ImGui::MenuItem("Communication")) {
                configWindow.show();
            }
            if (ImGui::MenuItem("NMEA Monitor", nullptr, monitorWindow.isVisible())) {
                monitorWindow.toggle();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Simulator")) {
            bool active = isSimulatorActive;
            if (ImGui::MenuItem(active ? "Stop Simulator" : "Start Simulator")) {
                isSimulatorActive = !active;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // Render Windows
    configWindow.render();
    monitorWindow.render();
    dashboardWindow.render(threadPool, serviceManager);
    
}

} // namespace App
