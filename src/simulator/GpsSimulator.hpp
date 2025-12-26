#pragma once

#include "SimulatorDecorator.hpp"

namespace Simulator {

class GpsSimulator : public SimulatorDecorator {
public:
    GpsSimulator(std::unique_ptr<ISimulator> simulator);
    
    void update(double dt) override;
    Core::NavData getCurrentData() const override;
    std::vector<std::string> getNmeaSentences() const override;

private:
    std::string generateRMC(const Core::NavData& data) const;
    std::string calculateChecksum(const std::string& sentence) const;

    mutable double _timeSinceLastEmit = 0.0;
};

} // namespace Simulator
