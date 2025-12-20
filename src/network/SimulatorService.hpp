#pragma once

#include "IService.hpp"

namespace Network {

class SimulatorService : public IService {
public:
    SimulatorService() = default;
    ~SimulatorService() = default;

    void start() override { running = true; }
    void stop() override { running = false; }
    bool isRunning() const override { return running; }
    void send(const std::string& data) override {} // Simulator doesn't accept input this way

private:
    bool running = true; // Default to true to avoid race condition at startup
};

} // namespace Network
