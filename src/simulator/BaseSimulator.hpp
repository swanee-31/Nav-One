#pragma once

#include "ISimulator.hpp"
#include <mutex>
#include <random>

namespace Simulator {

class BaseSimulator : public ISimulator {
public:
    BaseSimulator();
    
    void update(double dt) override;
    Core::NavData getCurrentData() const override;
    std::vector<std::string> getNmeaSentences() const override;
    
    void setConfig(const SimulatorConfig& config) override;
    SimulatorConfig getConfig() const override;
    
    void setPosition(double lat, double lon) override;

private:
    mutable std::mutex mutex;
    SimulatorConfig config;
    
    // Physics state
    double currentLat;
    double currentLon;
    double currentSog;
    double currentCog;
    
    // Variation logic
    double targetSog;
    double targetCog;
    double variationTimer = 0.0;

    // Random
    std::mt19937 rng;
};

} // namespace Simulator
