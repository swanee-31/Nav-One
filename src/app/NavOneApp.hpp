#pragma once

#include "gui/MainWindow.hpp"
#include "core/ThreadPool.hpp"
#include "core/MessageBus.hpp"
#include "app/services/ServiceManager.hpp"
#include "gui/windows/NmeaMonitorWindow.hpp"
#include "gui/windows/DashboardWindow.hpp"
#include "gui/windows/CommunicationSettingsWindow.hpp"
#include <atomic>
#include <memory>

namespace App {

class NavOneApp : public Gui::MainWindow {
public:
    NavOneApp(Core::ThreadPool& pool);
    ~NavOneApp();

    void render() override;

private:
    Core::ThreadPool& threadPool;
    std::atomic<bool> running{true};
    Core::MessageBus::ListenerId busListenerId;
    
    // Managers
    ServiceManager serviceManager;

    // Windows
    Gui::NmeaMonitorWindow monitorWindow;
    Gui::DashboardWindow dashboardWindow;
    Gui::CommunicationSettingsWindow configWindow;
    
    // Simulator
    std::atomic<bool> isSimulatorActive{false};
};

} // namespace App
