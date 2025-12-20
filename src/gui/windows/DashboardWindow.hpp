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
    std::mutex dataMutex;
    Core::NavData lastData;
    std::string lastSource = "None";
    uint64_t packetCount = 0;
};

} // namespace Gui
