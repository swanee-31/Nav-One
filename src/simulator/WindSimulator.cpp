#include "WindSimulator.hpp"
#include <sstream>
#include <iomanip>

namespace Simulator {

WindSimulator::WindSimulator(std::unique_ptr<ISimulator> simulator)
    : SimulatorDecorator(std::move(simulator)) {
}

void WindSimulator::update(double dt) {
    SimulatorDecorator::update(dt);
    timeSinceLastEmit += dt * 1000.0; // Convert to ms
    
    auto config = getConfig();
    if (!config.enableWind) return;

    // Wind Logic
    windTimer += dt;
    if (windTimer >= 60.0) {
        windTimer = 0.0;
        windClockwise = !windClockwise;
        windSpeedIncreasing = !windSpeedIncreasing;
    }
    
    // Oscillate direction
    double angleChange = (windClockwise ? 1.0 : -1.0) * dt * 2.0;
    windAngle += angleChange;
    if (windAngle < 0) windAngle += 360.0;
    if (windAngle >= 360.0) windAngle -= 360.0;
    
    // Oscillate speed
    double speedChange = (windSpeedIncreasing ? 0.1 : -0.1) * dt;
    windSpeed += speedChange;
    if (windSpeed < 0) { windSpeed = 0; windSpeedIncreasing = true; }
    if (windSpeed > 30) { windSpeed = 30; windSpeedIncreasing = false; }
}

Core::NavData WindSimulator::getCurrentData() const {
    auto data = SimulatorDecorator::getCurrentData();
    auto config = getConfig();
    
    if (config.enableWind) {
        data.hasWind = true;
        data.windAngle = windAngle;
        data.windSpeed = windSpeed;
    }
    
    return data;
}

std::vector<std::string> WindSimulator::getNmeaSentences() const {
    auto sentences = SimulatorDecorator::getNmeaSentences();
    auto config = getConfig();
    
    if (config.enableWind && timeSinceLastEmit >= config.windFrequency) {
        auto data = getCurrentData();
        // Override data with local wind state because BaseSimulator doesn't know about wind
        // Wait, getCurrentData() above already merges it.
        sentences.push_back(generateMWV(data));
        timeSinceLastEmit = 0.0;
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
