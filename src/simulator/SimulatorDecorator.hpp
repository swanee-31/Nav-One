#pragma once

#include "ISimulator.hpp"
#include <memory>

namespace Simulator {

class SimulatorDecorator : public ISimulator {
public:
    SimulatorDecorator(std::unique_ptr<ISimulator> simulator) 
        : _wrapped(std::move(simulator)) {}

    void update(double dt) override {
        if (_wrapped) _wrapped->update(dt);
    }

    Core::NavData getCurrentData() const override {
        return _wrapped ? _wrapped->getCurrentData() : Core::NavData{};
    }

    std::vector<std::string> getNmeaSentences() const override {
        return _wrapped ? _wrapped->getNmeaSentences() : std::vector<std::string>{};
    }

    void setConfig(const SimulatorConfig& config) override {
        if (_wrapped) _wrapped->setConfig(config);
    }

    SimulatorConfig getConfig() const override {
        return _wrapped ? _wrapped->getConfig() : SimulatorConfig{};
    }

    void setPosition(double lat, double lon) override {
        if (_wrapped) _wrapped->setPosition(lat, lon);
    }

protected:
    std::unique_ptr<ISimulator> _wrapped;
};

} // namespace Simulator
