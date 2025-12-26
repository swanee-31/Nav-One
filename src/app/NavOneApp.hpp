#pragma once

#include "gui/MainWindow.hpp"
#include "core/ThreadPool.hpp"
#include "core/MessageBus.hpp"
#include "app/services/ServiceManager.hpp"
#include "gui/windows/NmeaMonitorWindow.hpp"
#include "gui/windows/DashboardWindow.hpp"
#include "gui/windows/CommunicationSettingsWindow.hpp"
#include "gui/windows/DisplaySettingsWindow.hpp"
#include "gui/windows/SimulatorWindow.hpp"
#include "gui/windows/AboutWindow.hpp"
#include "app/PluginManager.hpp"
#include "simulator/ISimulator.hpp"
#include <atomic>
#include <memory>

namespace App {

class NavOneApp : public Gui::MainWindow {
public:
    NavOneApp(Core::ThreadPool& pool, bool headless = false);
    ~NavOneApp();

    bool init() override;
    void run() override;
    void render() override;
    void stop();

private:
    void runHeadless();

    Core::ThreadPool& _threadPool;
    bool _headless;
    std::atomic<bool> _running{true};
    Core::MessageBus::ListenerId _busListenerId;
    
    // Managers
    ServiceManager _serviceManager;
    PluginManager _pluginManager;

    // Simulator (Must be declared before SimulatorWindow)
    std::unique_ptr<Simulator::ISimulator> _simulator;
    std::atomic<bool> _isSimulatorActive{false};

    // Windows
    Gui::NmeaMonitorWindow _monitorWindow;
    Gui::DashboardWindow _dashboardWindow;
    Gui::CommunicationSettingsWindow _configWindow;
    Gui::DisplaySettingsWindow _displaySettingsWindow;
    Gui::SimulatorWindow _simulatorWindow;
    Gui::AboutWindow _aboutWindow;
    
    // Data
    std::mutex _dataMutex;
    Core::NavData _currentData;
};

} // namespace App
