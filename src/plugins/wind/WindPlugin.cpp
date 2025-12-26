#include "../../plugin_api/IPlugin.hpp"
#include "imgui.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <string>
#include <algorithm>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class WindPlugin : public PluginApi::IPlugin {
public:
    const char* getName() const override { return "Wind Monitor"; }
    const char* getVersion() const override { return "1.0.0"; }

    void init(const PluginApi::PluginContext& context) override {
        ImGui::SetCurrentContext(context.imguiContext);
    }

    void render(const Core::NavData& data) override {
        if (ImGui::Begin("Wind Monitor")) {
            
            // --- Logic ---
            bool hasGps = (data.speedOverGround >= 0); // Simple check
            
            // Toggle Mode
            if (ImGui::Button(_showTrueWind ? "Mode: TWA (True)" : "Mode: AWA (Apparent)")) {
                if (!_showTrueWind && hasGps) {
                    _showTrueWind = true;
                } else {
                    _showTrueWind = false;
                }
            }
            if (_showTrueWind && !hasGps) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1,0,0,1), "(Need GPS SOG)");
            }

            // Calculate Values
            double displayAngle = data.windAngle;
            double displaySpeed = data.windSpeed;
            
            if (_showTrueWind) {
                // Calculate True Wind from Apparent
                // AWS, AWA, SOG
                double aws = data.windSpeed;
                double awaRad = data.windAngle * M_PI / 180.0;
                double sog = data.speedOverGround;

                // Vector Math
                // Apparent Wind Vector (x, y) - blowing FROM
                // Let's use standard convention: 0 deg = North/Bow.
                // Wind Vector is usually "Where it comes FROM".
                
                // Tangential and Longitudinal components of Apparent Wind
                // Wx = AWS * sin(AWA)
                // Wy = AWS * cos(AWA)
                
                // True Wind components
                // Tx = Wx (Cross component unchanged by fwd motion) = AWS * sin(AWA)
                // Ty = Wy - SOG (Longitudinal component corrected) = AWS * cos(AWA) - SOG
                
                double tx = aws * sin(awaRad);
                double ty = aws * cos(awaRad) - sog;
                
                double tws = sqrt(tx*tx + ty*ty);
                double twa = atan2(tx, ty) * 180.0 / M_PI;
                
                if (twa < 0) twa += 360.0;
                
                displaySpeed = tws;
                displayAngle = twa;
            }

            // --- Visualization ---
            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
            ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
            if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
            if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
            
            // Keep aspect ratio square-ish for the gauge, but allow text below
            float size = std::min(canvas_sz.x, canvas_sz.y - 50); // Reserve space for text
            if (size < 100) size = 100;

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            
            ImVec2 center = ImVec2(canvas_p0.x + canvas_sz.x * 0.5f, canvas_p0.y + size * 0.5f);
            float radius = size * 0.4f;

            // 1. Sectors (Red Left, Green Right)
            // Angles in ImGui are radians, clockwise, 0 = Right (East).
            // We want 0 = Up (North). So -PI/2.
            // Port (Left): -180 to 0 (relative to bow). In ImGui: -PI/2 to -PI/2 - PI = -3PI/2?
            // Let's map:
            // Bow = -PI/2
            // Starboard (Right) = -PI/2 to -PI/2 + PI (0 to 180 deg)
            // Port (Left) = -PI/2 to -PI/2 - PI (0 to -180 deg)
            
            // Green Sector (Starboard/Right)
            draw_list->PathArcTo(center, radius, -M_PI/2, -M_PI/2 + M_PI, 32);
            draw_list->PathLineTo(center);
            draw_list->PathFillConvex(IM_COL32(0, 255, 0, 100)); // Transparent Green

            // Red Sector (Port/Left)
            draw_list->PathArcTo(center, radius, -M_PI/2 - M_PI, -M_PI/2, 32);
            draw_list->PathLineTo(center);
            draw_list->PathFillConvex(IM_COL32(255, 0, 0, 100)); // Transparent Red

            // 2. Boat (Center)
            float boatWidth = radius * 0.3f;
            float boatLength = radius * 0.6f;
            ImU32 boatColor = IM_COL32(200, 200, 200, 255);
            
            // Hull (Rectangle)
            ImVec2 hullMin(center.x - boatWidth/2, center.y - boatLength/2);
            ImVec2 hullMax(center.x + boatWidth/2, center.y + boatLength/2);
            draw_list->AddRectFilled(hullMin, hullMax, boatColor);
            
            // Bow (Triangle)
            ImVec2 bow1(hullMin.x, hullMin.y);
            ImVec2 bow2(hullMax.x, hullMin.y);
            ImVec2 bowTip(center.x, center.y - boatLength/2 - boatWidth/2);
            draw_list->AddTriangleFilled(bow1, bow2, bowTip, boatColor);

            // 3. Needle
            // Angle conversion: 
            // displayAngle is 0-360 clockwise from Bow.
            // ImGui 0 is Right. Bow is -PI/2.
            // So angleRad = (displayAngle * PI/180) - PI/2.
            float angleRad = (float)(displayAngle * M_PI / 180.0f) - (float)M_PI/2.0f;
            
            ImVec2 needleTip(center.x + cos(angleRad) * radius * 0.9f, center.y + sin(angleRad) * radius * 0.9f);
            draw_list->AddLine(center, needleTip, IM_COL32(255, 255, 0, 255), 3.0f);
            draw_list->AddCircleFilled(center, 5.0f, IM_COL32(255, 255, 0, 255));

            // --- Text Data ---
            ImGui::SetCursorScreenPos(ImVec2(canvas_p0.x, canvas_p0.y + size + 10));
            
            // Dynamic Font Scale based on window width
            float fontScale = canvas_sz.x / 200.0f;
            if (fontScale < 1.0f) fontScale = 1.0f;
            if (fontScale > 3.0f) fontScale = 3.0f;
            
            ImGui::SetWindowFontScale(fontScale);
            
            ImGui::Text("%s: %.1f deg", _showTrueWind ? "TWA" : "AWA", displayAngle);
            ImGui::Text("%s: %.1f kn", _showTrueWind ? "TWS" : "AWS", displaySpeed);
            
            ImGui::SetWindowFontScale(1.0f); // Reset

        }
        ImGui::End();
    }

    void shutdown() override {}

private:
    bool _showTrueWind = false;
};

// Export
extern "C" {
    __declspec(dllexport) PluginApi::IPlugin* createPlugin() {
        return new WindPlugin();
    }

    __declspec(dllexport) void destroyPlugin(PluginApi::IPlugin* plugin) {
        delete plugin;
    }
}
