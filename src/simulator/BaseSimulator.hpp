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
    mutable std::mutex _mutex;
    SimulatorConfig _config;
    
    // Physics state
    double _currentLat;
    double _currentLon;
    double _currentSog;
    double _currentCog;
    
    // Variation logic
    double _targetSog;
    double _targetCog;
    double _variationTimer = 0.0;

    // Random
    std::mt19937 _rng;
};

} // namespace Simulator
