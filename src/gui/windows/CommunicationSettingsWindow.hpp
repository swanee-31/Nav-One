#pragma once

#include "app/services/ServiceManager.hpp"
#include "imgui.h"

namespace Gui {

class CommunicationSettingsWindow {
public:
    CommunicationSettingsWindow(App::ServiceManager& manager);

    void render();
    
    void show() { _visible = true; }
    void hide() { _visible = false; }
    void toggle() { _visible = !_visible; }
    bool isVisible() const { return _visible; }
    bool* getVisiblePtr() { return &_visible; }

private:
    App::ServiceManager& _serviceManager;
    bool _visible = false;
};

} // namespace Gui
