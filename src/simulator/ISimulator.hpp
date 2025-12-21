#pragma once

#include "core/NavData.hpp"
#include "SimulatorConfig.hpp"
#include <vector>
#include <string>

namespace Simulator {

class ISimulator {
public:
    virtual ~ISimulator() = default;

    virtual void update(double dt) = 0;
    virtual Core::NavData getCurrentData() const = 0;
    virtual std::vector<std::string> getNmeaSentences() const = 0;
    
    virtual void setConfig(const SimulatorConfig& config) = 0;
    virtual SimulatorConfig getConfig() const = 0;
    
    virtual void setPosition(double lat, double lon) = 0;
};

} // namespace Simulator
