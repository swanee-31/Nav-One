#include "NmeaMonitorWindow.hpp"

namespace Gui {

NmeaMonitorWindow::NmeaMonitorWindow() {}

void NmeaMonitorWindow::addLog(const std::string& source, const std::string& frame) {
    if (!visible) return; // Optimization: don't log if not visible? Or maybe we want history? 
    // Let's keep history even if closed, but maybe limit it.
    
    std::lock_guard<std::mutex> lock(logMutex);
    std::string logEntry = "[" + source + "] " + frame;
    logs.push_back(logEntry);
    if (logs.size() > MAX_LOGS) {
        logs.pop_front();
    }
}

void NmeaMonitorWindow::render() {
    if (!visible) return;

    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("NMEA Monitor", &visible)) {
        
        if (ImGui::Button("Clear")) {
            std::lock_guard<std::mutex> lock(logMutex);
            logs.clear();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &autoScroll);
        
        ImGui::Separator();
        
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        
        {
            std::lock_guard<std::mutex> lock(logMutex);
            for (const auto& log : logs) {
                ImGui::TextUnformatted(log.c_str());
            }
        }

        if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
    }
    ImGui::End();
}

} // namespace Gui
