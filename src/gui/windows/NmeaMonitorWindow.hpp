#pragma once

#include "imgui.h"
#include <string>
#include <deque>
#include <mutex>

namespace Gui {

class NmeaMonitorWindow {
public:
    NmeaMonitorWindow();
    
    void render();
    void addLog(const std::string& source, const std::string& frame);
    
    void show() { visible = true; }
    void hide() { visible = false; }
    void toggle() { visible = !visible; }
    bool isVisible() const { return visible; }
    bool* getVisiblePtr() { return &visible; }

private:
    bool visible = false;
    bool autoScroll = true;
    std::deque<std::string> logs;
    const size_t MAX_LOGS = 100;
    std::mutex logMutex;
};

} // namespace Gui
