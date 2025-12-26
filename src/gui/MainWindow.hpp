#pragma once

#include <string>
#include <memory>

struct GLFWwindow;

namespace Gui {

class MainWindow {
public:
    MainWindow(int width, int height, const std::string& title);
    ~MainWindow();

    virtual bool init();
    virtual void run();
    
    // Method to be called every frame to render custom UI
    virtual void render();

private:
    int _width;
    int _height;
    std::string _title;
    GLFWwindow* _window;
    bool _initialized = false;
    
    void setupStyle();
};

} // namespace Gui
