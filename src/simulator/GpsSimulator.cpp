#include "GpsSimulator.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace Simulator {

GpsSimulator::GpsSimulator(std::unique_ptr<ISimulator> simulator)
    : SimulatorDecorator(std::move(simulator)) {}

void GpsSimulator::update(double dt) {
    SimulatorDecorator::update(dt);
    _timeSinceLastEmit += dt * 1000.0; // Convert to ms
}

Core::NavData GpsSimulator::getCurrentData() const {
    auto data = SimulatorDecorator::getCurrentData();
    auto config = getConfig();
    
    if (config.enableGps) {
        data.hasPosition = true;
        data.hasSpeed = true;
        data.isGpsValid = true;
    }
    
    return data;
}

std::vector<std::string> GpsSimulator::getNmeaSentences() const {
    auto sentences = SimulatorDecorator::getNmeaSentences();
    auto config = getConfig();
    
    if (config.enableGps && _timeSinceLastEmit >= config.gpsFrequency) {
        auto data = getCurrentData(); // Get data with flags
        sentences.push_back(generateRMC(data));
        _timeSinceLastEmit = 0.0;
    }
    
    return sentences;
}

std::string GpsSimulator::generateRMC(const Core::NavData& data) const {
    std::stringstream ss;
    
    // Time
    auto now = std::chrono::system_clock::to_time_t(data.timestamp);
    std::tm tm = *std::gmtime(&now);
    
    ss << "GPRMC,";
    ss << std::put_time(&tm, "%H%M%S") << ",";
    ss << "A,"; // Active
    
    // Lat
    double lat = std::abs(data.latitude);
    int latDeg = static_cast<int>(lat);
    double latMin = (lat - latDeg) * 60.0;
    ss << std::setfill('0') << std::setw(2) << latDeg << std::fixed << std::setprecision(4) << std::setw(7) << latMin << ",";
    ss << (data.latitude >= 0 ? "N" : "S") << ",";
    
    // Lon
    double lon = std::abs(data.longitude);
    int lonDeg = static_cast<int>(lon);
    double lonMin = (lon - lonDeg) * 60.0;
    ss << std::setfill('0') << std::setw(3) << lonDeg << std::fixed << std::setprecision(4) << std::setw(7) << lonMin << ",";
    ss << (data.longitude >= 0 ? "E" : "W") << ",";
    
    // Speed/Course
    ss << std::fixed << std::setprecision(1) << data.speedOverGround << ",";
    ss << std::fixed << std::setprecision(1) << data.courseOverGround << ",";
    
    // Date
    ss << std::put_time(&tm, "%d%m%y") << ",";
    
    // Mag Var
    ss << ",,";
    
    // Mode
    ss << "A";
    
    std::string content = ss.str();
    return "$" + content + "*" + calculateChecksum(content);
}

std::string GpsSimulator::calculateChecksum(const std::string& sentence) const {
    int sum = 0;
    for (char c : sentence) {
        sum ^= c;
    }
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << sum;
    return ss.str();
}

} // namespace Simulator
