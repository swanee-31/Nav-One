#include "DashboardWindow.hpp"
#include "app/services/ServiceManager.hpp"
#include <iomanip>
#include <sstream>
#include <ctime>

namespace Gui {

DashboardWindow::DashboardWindow() {}

void DashboardWindow::updateData(const Core::NavData& update) {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    // Merge logic
    if (update.heading != 0.0) lastData.heading = update.heading;
    if (update.speedOverGround != 0.0) lastData.speedOverGround = update.speedOverGround;
    if (update.courseOverGround != 0.0) lastData.courseOverGround = update.courseOverGround;
    if (update.windSpeed != 0.0) lastData.windSpeed = update.windSpeed;
    if (update.windAngle != 0.0) lastData.windAngle = update.windAngle;
    
    lastData.timestamp = update.timestamp;
    lastData.sourceId = update.sourceId;
    
    packetCount++;
    lastSource = update.sourceId;
}

void DashboardWindow::render(Core::ThreadPool& threadPool, const App::ServiceManager& serviceManager) {
    ImGui::Begin("Navigation Dashboard");
    auto lock = serviceManager.getLock();
    
    size_t poolThreads = threadPool.getBusyCount();
    size_t serviceThreads = serviceManager.getActiveServices().size();
    size_t outputThreads = serviceManager.getActiveOutputs().size();
    size_t totalThreads = poolThreads + serviceThreads + outputThreads;

    ImGui::Text("System Status: Running");
    ImGui::Text("Active Threads: %zu (Pool: %zu, IO: %zu)", totalThreads, poolThreads, serviceThreads + outputThreads);
    ImGui::Separator();

    {
        std::lock_guard<std::mutex> lock(dataMutex);
        
        ImGui::TextColored(ImVec4(0,1,0,1), "Live Data");
        ImGui::Text("Source: %s", lastSource.c_str());
        ImGui::Text("Packets Received: %llu", packetCount);
        
        ImGui::Separator();
        ImGui::Text("Heading: %.1f deg", lastData.heading);
        ImGui::Text("Speed:   %.1f kts", lastData.speedOverGround);
        ImGui::Text("Wind:    %.1f kts @ %.1f deg", lastData.windSpeed, lastData.windAngle);
        
        // Time display
        auto time = std::chrono::system_clock::to_time_t(lastData.timestamp);
        std::tm* tm = std::localtime(&time);
        char timeBuffer[32];
        std::strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", tm);
        ImGui::Text("Last Update: %s", timeBuffer);
    }

    ImGui::Separator();
    ImGui::Text("Active Services");
    
    const auto& sources = serviceManager.getSources();
    const auto& activeServices = serviceManager.getActiveServices();

    if (sources.empty()) {
        ImGui::TextDisabled("No services configured");
    } else {
        if (ImGui::BeginTable("ServicesTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableHeadersRow();

            for (const auto& source : sources) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(source.name.c_str());

                ImGui::TableSetColumnIndex(1);
                const char* typeStr = "Unknown";
                if (source.type == App::SourceType::Serial) typeStr = "Serial";
                else if (source.type == App::SourceType::Udp) typeStr = "UDP";
                else if (source.type == App::SourceType::Simulator) typeStr = "Sim";
                ImGui::TextUnformatted(typeStr);

                ImGui::TableSetColumnIndex(2);
                if (!source.enabled) {
                    ImGui::TextDisabled("Disabled");
                } else {
                    auto it = activeServices.find(source.id);
                    if (it == activeServices.end()) {
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Not Found");
                    } else if (!it->second) {
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Null Ptr");
                    } else if (!it->second->isRunning()) {
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Stopped");
                    } else {
                        ImGui::TextColored(ImVec4(0, 1, 0, 1), "Running");
                    }
                }
            }
            ImGui::EndTable();
        }
    }

    ImGui::End();
}

} // namespace Gui
