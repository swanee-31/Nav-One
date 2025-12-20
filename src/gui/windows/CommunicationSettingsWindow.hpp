#pragma once

#include "app/services/ServiceManager.hpp"
#include "imgui.h"

namespace Gui {

class CommunicationSettingsWindow {
public:
    CommunicationSettingsWindow(App::ServiceManager& manager);

    void render();
    
    void show() { visible = true; }
    void hide() { visible = false; }
    void toggle() { visible = !visible; }
    bool isVisible() const { return visible; }
    bool* getVisiblePtr() { return &visible; }

private:
    App::ServiceManager& serviceManager;
    bool visible = false;
};

} // namespace Gui
