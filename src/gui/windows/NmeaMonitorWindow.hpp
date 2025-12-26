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
    
    void show() { _visible = true; }
    void hide() { _visible = false; }
    void toggle() { _visible = !_visible; }
    bool isVisible() const { return _visible; }
    bool* getVisiblePtr() { return &_visible; }

private:
    bool _visible = false;
    bool _autoScroll = true;
    bool _paused = false;
    std::deque<std::string> _logs;
    std::string _textBuffer; // For InputTextMultiline
    const size_t _maxLogs = 100;
    std::mutex _logMutex;
};

} // namespace Gui
