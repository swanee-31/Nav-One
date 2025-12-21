#pragma once

#include "SimulatorDecorator.hpp"
#include <random>

namespace Simulator {

class WindSimulator : public SimulatorDecorator {
public:
    WindSimulator(std::unique_ptr<ISimulator> simulator);
    
    void update(double dt) override;
    Core::NavData getCurrentData() const override;
    std::vector<std::string> getNmeaSentences() const override;

private:
    std::string generateMWV(const Core::NavData& data) const;
    std::string calculateChecksum(const std::string& sentence) const;

    // Wind state
    double windAngle = 0.0;
    double windSpeed = 0.0;
    bool windClockwise = true;
    bool windSpeedIncreasing = true;
    double windTimer = 0.0;

    mutable double timeSinceLastEmit = 0.0;
};

} // namespace Simulator
