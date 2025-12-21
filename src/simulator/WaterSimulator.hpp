#pragma once

#include "SimulatorDecorator.hpp"

namespace Simulator {

class WaterSimulator : public SimulatorDecorator {
public:
    WaterSimulator(std::unique_ptr<ISimulator> simulator);
    
    void update(double dt) override;
    Core::NavData getCurrentData() const override;
    std::vector<std::string> getNmeaSentences() const override;

private:
    std::string generateDBS(const Core::NavData& data) const;
    std::string generateDPT(const Core::NavData& data) const;
    std::string generateMTW(const Core::NavData& data) const;
    std::string generateHDT(const Core::NavData& data) const;
    std::string generateVHW(const Core::NavData& data) const;
    
    std::string calculateChecksum(const std::string& sentence) const;

    // Water state
    double currentDepth = 0.0;
    double currentWaterTemp = 0.0;
    double timer = 0.0;
    bool increasing = true;

    mutable double timeSinceLastEmit = 0.0;
};

} // namespace Simulator
