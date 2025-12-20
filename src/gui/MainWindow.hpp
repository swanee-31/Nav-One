#pragma once

#include <string>
#include <memory>

struct GLFWwindow;

namespace Gui {

class MainWindow {
public:
    MainWindow(int width, int height, const std::string& title);
    ~MainWindow();

    bool init();
    void run();
    
    // Method to be called every frame to render custom UI
    virtual void render();

private:
    int width;
    int height;
    std::string title;
    GLFWwindow* window;
    
    void setupStyle();
};

} // namespace Gui
