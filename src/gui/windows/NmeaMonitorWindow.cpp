#include "NmeaMonitorWindow.hpp"

namespace Gui {

NmeaMonitorWindow::NmeaMonitorWindow() {}

void NmeaMonitorWindow::addLog(const std::string& source, const std::string& frame) {
    if (!_visible) return;
    
    std::lock_guard<std::mutex> lock(_logMutex);
    
    if (_paused) return; // Don't add logs if paused

    std::string logEntry = "[" + source + "] " + frame + "\n";
    _logs.push_back(logEntry);
    if (_logs.size() > _maxLogs) {
        _logs.pop_front();
    }

    // Rebuild buffer for InputTextMultiline
    // Optimization: In a real app, we might want to append or use a circular buffer, 
    // but for 100 lines, rebuilding is fine.
    _textBuffer.clear();
    for (const auto& log : _logs) {
        _textBuffer += log;
    }
}

void NmeaMonitorWindow::render() {
    if (!_visible) return;

    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("NMEA Monitor", &_visible)) {
        
        if (ImGui::Button("Clear")) {
            std::lock_guard<std::mutex> lock(_logMutex);
            _logs.clear();
            _textBuffer.clear();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Pause", &_paused);
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &_autoScroll);
        
        ImGui::Separator();
        
        // Use InputTextMultiline for selection support
        // ReadOnly flag prevents editing
        std::lock_guard<std::mutex> lock(_logMutex);
        ImGui::InputTextMultiline("##logs", &_textBuffer[0], _textBuffer.size() + 1, 
            ImVec2(-FLT_MIN, -FLT_MIN), ImGuiInputTextFlags_ReadOnly);

        if (_autoScroll && !_paused && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
    }
    ImGui::End();
}

} // namespace Gui
