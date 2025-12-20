#include "app/NavOneApp.hpp"
#include "core/ThreadPool.hpp"
#include <iostream>

int main() {
    try {
        // 1. Initialize Core Services
        Core::ThreadPool pool(4); // 4 worker threads

        // 2. Initialize GUI
        App::NavOneApp app(pool);

        // 3. Initialize and Run App
        if (!app.init()) {
            std::cerr << "Failed to initialize application" << std::endl;
            return -1;
        }

        app.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
