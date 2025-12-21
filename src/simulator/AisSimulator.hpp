#pragma once

#include "SimulatorDecorator.hpp"
#include <vector>

namespace Simulator {

struct AisShipState {
    AisTargetConfig config;
    double currentLat;
    double currentLon;
    double timeSinceLastEmit = 0.0;
    double timeSinceLastStaticEmit = 0.0;
};

class AisSimulator : public SimulatorDecorator {
public:
    AisSimulator(std::unique_ptr<ISimulator> simulator);
    
    void update(double dt) override;
    Core::NavData getCurrentData() const override;
    std::vector<std::string> getNmeaSentences() const override;
    
    void setConfig(const SimulatorConfig& config) override;

private:
    std::vector<AisShipState> ships;
    bool initialized = false;

    void initShips(const SimulatorConfig& config);
    void updateShipPhysics(AisShipState& ship, double dt);
    
    std::string generatePositionReport(const AisShipState& ship) const;
    std::string generateStaticDataReport(const AisShipState& ship) const;
    
    // AIVDM Encoding Helpers
    std::string encodeAivdm(const std::vector<bool>& bits, int fillBits) const;
    std::string calculateChecksum(const std::string& sentence) const;
};

} // namespace Simulator
