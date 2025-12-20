#include "NmeaMonitorWindow.hpp"

namespace Gui {

NmeaMonitorWindow::NmeaMonitorWindow() {}

void NmeaMonitorWindow::addLog(const std::string& source, const std::string& frame) {
    if (!visible) return;
    
    std::lock_guard<std::mutex> lock(logMutex);
    
    if (paused) return; // Don't add logs if paused

    std::string logEntry = "[" + source + "] " + frame + "\n";
    logs.push_back(logEntry);
    if (logs.size() > MAX_LOGS) {
        logs.pop_front();
    }

    // Rebuild buffer for InputTextMultiline
    // Optimization: In a real app, we might want to append or use a circular buffer, 
    // but for 100 lines, rebuilding is fine.
    textBuffer.clear();
    for (const auto& log : logs) {
        textBuffer += log;
    }
}

void NmeaMonitorWindow::render() {
    if (!visible) return;

    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("NMEA Monitor", &visible)) {
        
        if (ImGui::Button("Clear")) {
            std::lock_guard<std::mutex> lock(logMutex);
            logs.clear();
            textBuffer.clear();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Pause", &paused);
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &autoScroll);
        
        ImGui::Separator();
        
        // Use InputTextMultiline for selection support
        // ReadOnly flag prevents editing
        std::lock_guard<std::mutex> lock(logMutex);
        ImGui::InputTextMultiline("##logs", &textBuffer[0], textBuffer.size() + 1, 
            ImVec2(-FLT_MIN, -FLT_MIN), ImGuiInputTextFlags_ReadOnly);

        if (autoScroll && !paused && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
    }
    ImGui::End();
}

} // namespace Gui
