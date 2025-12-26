#include "NavOneApp.hpp"
#include "imgui.h"
#include "simulator/BaseSimulator.hpp"
#include "simulator/GpsSimulator.hpp"
#include "simulator/WindSimulator.hpp"
#include "simulator/WaterSimulator.hpp"
#include "simulator/AisSimulator.hpp"
#include "utils/ConfigManager.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace App {

NavOneApp::NavOneApp(Core::ThreadPool& pool, bool headlessMode) 
    : Gui::MainWindow(1280, 720, "NavOne - Navigation Hub"), 
      _threadPool(pool),
      _headless(headlessMode),
      _simulator(std::make_unique<Simulator::AisSimulator>( // Load Simulator with decorators pattern
          std::make_unique<Simulator::WaterSimulator>(
              std::make_unique<Simulator::WindSimulator>(
                  std::make_unique<Simulator::GpsSimulator>(
                      std::make_unique<Simulator::BaseSimulator>()
                  )
              )
          )
      )),
      _configWindow(_serviceManager),
      _simulatorWindow(*_simulator) {
    
    // Setup Service Manager Logging
    _serviceManager.setLogCallback([this](const std::string& source, const std::string& frame) {
        if (_headless) {
            std::cout << "[" << source << "] " << frame << std::endl;
        } else {
            _monitorWindow.addLog(source, frame);
        }
    });

    // Load Config
    _serviceManager.loadConfig();
    
    // Apply Simulator Config
    _simulator->setConfig(Utils::ConfigManager::instance().getSimulatorConfig());
    
    // Subscribe to MessageBus
    _busListenerId = Core::MessageBus::instance().subscribe([this](const Core::NavData& update) {
        {
            std::lock_guard<std::mutex> lock(_dataMutex);
            // Merge logic to persist data across partial updates
            _currentData.timestamp = update.timestamp;
            _currentData.sourceId = update.sourceId;

            if (update.hasPosition) {
                _currentData.latitude = update.latitude;
                _currentData.longitude = update.longitude;
                _currentData.altitude = update.altitude;
                _currentData.isGpsValid = update.isGpsValid;
                _currentData.hasPosition = true;
            }
            
            if (update.hasSpeed) {
                _currentData.speedOverGround = update.speedOverGround;
                _currentData.courseOverGround = update.courseOverGround;
                _currentData.hasSpeed = true;
            }
            
            if (update.hasHeading) {
                _currentData.heading = update.heading;
                _currentData.hasHeading = true;
            }
            
            if (update.hasWind) {
                _currentData.windSpeed = update.windSpeed;
                _currentData.windAngle = update.windAngle;
                _currentData.hasWind = true;
            }
            
            if (update.hasDepth) {
                _currentData.depth = update.depth;
                _currentData.hasDepth = true;
            }

            if (update.hasWaterTemperature) {
                _currentData.waterTemperature = update.waterTemperature;
                _currentData.hasWaterTemperature = true;
            }

            if (update.hasWaterSpeed) {
                _currentData.speedThroughWater = update.speedThroughWater;
                _currentData.hasWaterSpeed = true;
            }
        }
        _dashboardWindow.updateData(update);
    });

    // Start a background task to simulate data acquisition (publishing to bus)
    _threadPool.enqueue([this] {
        while(_running) {
            if (_isSimulatorActive) {
                // Update Simulator Physics (100ms step)
                _simulator->update(0.1);
                
                // Only publish if Simulator is enabled as a Source in ServiceManager
                if (_serviceManager.isSourceEnabled("SIMULATOR")) {
                    // Get Data
                    Core::NavData simData = _simulator->getCurrentData();
                    
                    // Log simulated frames
                    auto sentences = _simulator->getNmeaSentences();
                    for (const auto& sentence : sentences) {
                        _monitorWindow.addLog("SIMULATOR", sentence);
                        // Broadcast to outputs
                        _serviceManager.broadcast(sentence + "\r\n", "SIMULATOR");
                    }

                    Core::MessageBus::instance().publish(simData);
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
}

NavOneApp::~NavOneApp() {
    _running = false;
    Core::MessageBus::instance().unsubscribe(_busListenerId);
    _serviceManager.stopAll();
}

bool NavOneApp::init() {
    if (!_headless) {
        if (!Gui::MainWindow::init()) return false;
        
        // Apply Display Settings after ImGui context is created
        _displaySettingsWindow.applyConfig();
    }
    
    return true;
}

void NavOneApp::run() {
    if (_headless) {
        runHeadless();
    } else {
        Gui::MainWindow::run();
    }
}

void NavOneApp::runHeadless() {
    std::cout << "NavOne running in headless mode." << std::endl;
    std::cout << "Press Ctrl+C to exit." << std::endl;
    
    while (_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void NavOneApp::stop() {
    _running = false;
}

void NavOneApp::render() {
#ifdef _WIN32
    const std::string prefix = "";
    const std::string ext = ".dll";
#else
    // On Linux, dlopen requires a path (e.g. "./") to look in the current directory
    const std::string prefix = "./lib";
    const std::string ext = ".so";
#endif

    // Menu Bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Configuration")) {
            if (ImGui::MenuItem("Communication")) {
                _configWindow.show();
            }
            if (ImGui::MenuItem("Display")) {
                _displaySettingsWindow.show();
            }
            if (ImGui::MenuItem("NMEA Monitor", nullptr, _monitorWindow.isVisible())) {
                _monitorWindow.toggle();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Plugins")) {
            if (ImGui::MenuItem("Load GPS Plugin")) {
                // Todo : scan the directory. 
                // Here we hardcode the path relative to the executable.
                // CMake puts it in src/plugins/
                _pluginManager.loadPlugin(prefix + "GpsPlugin" + ext);
            }
            if (ImGui::MenuItem("Load GPS Big Plugin")) {
                _pluginManager.loadPlugin(prefix + "GpsBigPlugin" + ext);
            }
            if (ImGui::MenuItem("Load Wind Plugin")) {
                _pluginManager.loadPlugin(prefix + "WindPlugin" + ext);
            }
            if (ImGui::MenuItem("Load Water Plugin")) {
                _pluginManager.loadPlugin(prefix + "WaterPlugin" + ext);
            }
            
            ImGui::Separator();
            
            auto& plugins = _pluginManager.getPlugins();
            for (auto& plugin : plugins) {
                if (ImGui::MenuItem(plugin.instance->getName(), nullptr, plugin.active)) {
                    plugin.active = !plugin.active;
                }
            }
            
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Simulator")) {
            bool active = _isSimulatorActive;
            if (ImGui::MenuItem(active ? "Stop Simulator" : "Start Simulator")) {
                _isSimulatorActive = !active;
            }
            if (ImGui::MenuItem("Configure Simulator")) {
                _simulatorWindow.show();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                _aboutWindow.show();
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
    
    // Render Windows
    _configWindow.render();
    _displaySettingsWindow.render();
    _simulatorWindow.render();
    _aboutWindow.render();
    _monitorWindow.render();
    _dashboardWindow.render(_threadPool, _serviceManager);
    
    // Render Plugins
    {
        std::lock_guard<std::mutex> lock(_dataMutex);
        _pluginManager.renderPlugins(_currentData);
    }
}

} // namespace App
