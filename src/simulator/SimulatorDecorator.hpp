#pragma once

#include "ISimulator.hpp"
#include <memory>

namespace Simulator {

class SimulatorDecorator : public ISimulator {
public:
    SimulatorDecorator(std::unique_ptr<ISimulator> simulator) 
        : wrapped(std::move(simulator)) {}

    void update(double dt) override {
        if (wrapped) wrapped->update(dt);
    }

    Core::NavData getCurrentData() const override {
        return wrapped ? wrapped->getCurrentData() : Core::NavData{};
    }

    std::vector<std::string> getNmeaSentences() const override {
        return wrapped ? wrapped->getNmeaSentences() : std::vector<std::string>{};
    }

    void setConfig(const SimulatorConfig& config) override {
        if (wrapped) wrapped->setConfig(config);
    }

    SimulatorConfig getConfig() const override {
        return wrapped ? wrapped->getConfig() : SimulatorConfig{};
    }

    void setPosition(double lat, double lon) override {
        if (wrapped) wrapped->setPosition(lat, lon);
    }

protected:
    std::unique_ptr<ISimulator> wrapped;
};

} // namespace Simulator
