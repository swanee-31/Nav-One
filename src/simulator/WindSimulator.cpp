#include "WindSimulator.hpp"
#include <sstream>
#include <iomanip>

namespace Simulator {

WindSimulator::WindSimulator(std::unique_ptr<ISimulator> simulator)
    : SimulatorDecorator(std::move(simulator)) {
}

void WindSimulator::update(double dt) {
    SimulatorDecorator::update(dt);
    _timeSinceLastEmit += dt * 1000.0; // Convert to ms
    
    auto config = getConfig();
    if (!config.enableWind) return;

    // Wind Logic
    _windTimer += dt;
    if (_windTimer >= 60.0) {
        _windTimer = 0.0;
        _windClockwise = !_windClockwise;
        _windSpeedIncreasing = !_windSpeedIncreasing;
    }
    
    // Oscillate direction
    double angleChange = (_windClockwise ? 1.0 : -1.0) * dt * 2.0;
    _windAngle += angleChange;
    if (_windAngle < 0) _windAngle += 360.0;
    if (_windAngle >= 360.0) _windAngle -= 360.0;
    
    // Oscillate speed
    double speedChange = (_windSpeedIncreasing ? 0.1 : -0.1) * dt;
    _windSpeed += speedChange;
    if (_windSpeed < 0) { _windSpeed = 0; _windSpeedIncreasing = true; }
    if (_windSpeed > 30) { _windSpeed = 30; _windSpeedIncreasing = false; }
}

Core::NavData WindSimulator::getCurrentData() const {
    auto data = SimulatorDecorator::getCurrentData();
    auto config = getConfig();
    
    if (config.enableWind) {
        data.hasWind = true;
        data.windAngle = _windAngle;
        data.windSpeed = _windSpeed;
    }
    
    return data;
}

std::vector<std::string> WindSimulator::getNmeaSentences() const {
    auto sentences = SimulatorDecorator::getNmeaSentences();
    auto config = getConfig();
    
    if (config.enableWind && _timeSinceLastEmit >= config.windFrequency) {
        auto data = getCurrentData();
        // Override data with local wind state because BaseSimulator doesn't know about wind
        // Wait, getCurrentData() above already merges it.
        sentences.push_back(generateMWV(data));
        _timeSinceLastEmit = 0.0;
    }
    
    return sentences;
}

std::string WindSimulator::generateMWV(const Core::NavData& data) const {
    std::stringstream ss;
    ss << "IIMWV,";
    ss << std::fixed << std::setprecision(1) << data.windAngle << ",";
    ss << "R,"; // Relative
    ss << std::fixed << std::setprecision(1) << data.windSpeed << ",";
    ss << "N,"; // Knots
    ss << "A";  // Valid
    
    std::string content = ss.str();
    return "$" + content + "*" + calculateChecksum(content);
}

std::string WindSimulator::calculateChecksum(const std::string& sentence) const {
    int sum = 0;
    for (char c : sentence) {
        sum ^= c;
    }
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << sum;
    return ss.str();
}

} // namespace Simulator
