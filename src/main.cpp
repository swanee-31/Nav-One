#include "app/NavOneApp.hpp"
#include "core/ThreadPool.hpp"
#include <iostream>
#include <csignal>
#include <string>

// Global pointer for signal handler
App::NavOneApp* g_app = nullptr;

void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received.\n";
    if (g_app) {
        g_app->stop();
    }
}

int main(int argc, char* argv[]) {
    try {
        bool headless = false;
        
        // Parse arguments
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-nogui") {
                headless = true;
            }
        }

        // 1. Initialize Core Services
        Core::ThreadPool pool(4); // 4 worker threads

        // 2. Initialize App
        App::NavOneApp app(pool, headless);
        g_app = &app;
        
        // Register signal handler for Ctrl+C
        signal(SIGINT, signalHandler);

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
