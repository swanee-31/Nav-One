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
    NavOneApp(Core::ThreadPool& pool);
    ~NavOneApp();

    bool init() override;
    void render() override;

private:
    Core::ThreadPool& threadPool;
    std::atomic<bool> running{true};
    Core::MessageBus::ListenerId busListenerId;
    
    // Managers
    ServiceManager serviceManager;
    PluginManager pluginManager;

    // Simulator (Must be declared before SimulatorWindow)
    std::unique_ptr<Simulator::ISimulator> simulator;
    std::atomic<bool> isSimulatorActive{false};

    // Windows
    Gui::NmeaMonitorWindow monitorWindow;
    Gui::DashboardWindow dashboardWindow;
    Gui::CommunicationSettingsWindow configWindow;
    Gui::DisplaySettingsWindow displaySettingsWindow;
    Gui::SimulatorWindow simulatorWindow;
    Gui::AboutWindow aboutWindow;
    
    // Data
    std::mutex dataMutex;
    Core::NavData currentData;
};

} // namespace App
