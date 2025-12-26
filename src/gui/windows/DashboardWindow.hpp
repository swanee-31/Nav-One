#pragma once

#include "core/NavData.hpp"
#include "core/ThreadPool.hpp"
#include "imgui.h"
#include <mutex>
#include <string>

namespace App { class ServiceManager; }

namespace Gui {

class DashboardWindow {
public:
    DashboardWindow();

    void render(Core::ThreadPool& threadPool, const App::ServiceManager& serviceManager);
    void updateData(const Core::NavData& data);

private:
    std::mutex _dataMutex;
    Core::NavData _lastData;
    std::string _lastSource = "None";
    uint64_t _packetCount = 0;
};

} // namespace Gui
