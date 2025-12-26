#include "WaterSimulator.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace Simulator {

WaterSimulator::WaterSimulator(std::unique_ptr<ISimulator> simulator)
    : SimulatorDecorator(std::move(simulator)) {
    auto config = getConfig();
    _currentDepth = config.minDepth;
    _currentWaterTemp = config.minWaterTemp;
}

void WaterSimulator::update(double dt) {
    SimulatorDecorator::update(dt);
    _timeSinceLastEmit += dt * 1000.0; // Convert to ms
    
    auto config = getConfig();
    if (!config.enableWater) return;

    // Oscillation logic (1 minute period)
    _timer += dt;
    if (_timer >= 60.0) {
        _timer = 0.0;
        _increasing = !_increasing;
    }
    
    // Linear interpolation based on timer/60.0 would be a sawtooth wave if we just reset.
    // But "rotation on 1 mn" usually implies a full cycle or back and forth.
    // Let's do a sine wave for smoother transition over 60 seconds (2PI period = 60s?)
    // Or just linear back and forth as requested "min et max".
    
    double factor = 0.0;
    if (_increasing) {
        factor = _timer / 60.0;
    } else {
        factor = 1.0 - (_timer / 60.0);
    }
    
    // Use sine for smoother "natural" feel
    // Period 60s -> freq = 1/60 Hz. 
    // val = min + (max-min) * (0.5 * (1 + sin(2*PI * t / 60)))
    // This goes min->max->min in 60 seconds.
    
    double sineFactor = 0.5 * (1.0 + std::sin(2.0 * 3.14159 * _timer / 60.0));
    
    _currentDepth = config.minDepth + (config.maxDepth - config.minDepth) * sineFactor;
    _currentWaterTemp = config.minWaterTemp + (config.maxWaterTemp - config.minWaterTemp) * sineFactor;
}

Core::NavData WaterSimulator::getCurrentData() const {
    auto data = SimulatorDecorator::getCurrentData();
    auto config = getConfig();
    
    if (config.enableWater) {
        data.hasDepth = true;
        data.depth = _currentDepth;
        
        data.hasWaterTemperature = true;
        data.waterTemperature = _currentWaterTemp;
        
        data.hasWaterSpeed = true;
        data.speedThroughWater = data.speedOverGround; // Assume STW = SOG for sim
        
        data.hasHeading = true;
        data.heading = data.courseOverGround; // Assume Heading = COG for sim
    }
    
    return data;
}

std::vector<std::string> WaterSimulator::getNmeaSentences() const {
    auto sentences = SimulatorDecorator::getNmeaSentences();
    auto config = getConfig();
    
    if (config.enableWater && _timeSinceLastEmit >= config.waterFrequency) {
        auto data = getCurrentData();
        sentences.push_back(generateDBS(data));
        sentences.push_back(generateDPT(data));
        sentences.push_back(generateMTW(data));
        sentences.push_back(generateHDT(data));
        sentences.push_back(generateVHW(data));
        _timeSinceLastEmit = 0.0;
    }
    
    return sentences;
}

std::string WaterSimulator::generateDBS(const Core::NavData& data) const {
    // $IIDBS,x.x,f,y.y,M,z.z,F*hh
    // Depth Below Surface
    std::stringstream ss;
    ss << "IIDBS,";
    double feet = data.depth * 3.28084;
    double fathoms = data.depth * 0.546807;
    
    ss << std::fixed << std::setprecision(1) << feet << ",f,";
    ss << std::fixed << std::setprecision(1) << data.depth << ",M,";
    ss << std::fixed << std::setprecision(1) << fathoms << ",F";
    
    std::string content = ss.str();
    return "$" + content + "*" + calculateChecksum(content);
}

std::string WaterSimulator::generateDPT(const Core::NavData& data) const {
    // $IIDPT,x.x,x.x,x.x*hh
    // Depth relative to transducer, offset
    std::stringstream ss;
    ss << "IIDPT,";
    ss << std::fixed << std::setprecision(1) << data.depth << ",";
    ss << "0.0,"; // Offset
    ss << "100.0"; // Max range scale (optional)
    
    std::string content = ss.str();
    return "$" + content + "*" + calculateChecksum(content);
}

std::string WaterSimulator::generateMTW(const Core::NavData& data) const {
    // $IIMTW,x.x,C*hh
    std::stringstream ss;
    ss << "IIMTW,";
    ss << std::fixed << std::setprecision(1) << data.waterTemperature << ",C";
    
    std::string content = ss.str();
    return "$" + content + "*" + calculateChecksum(content);
}

std::string WaterSimulator::generateHDT(const Core::NavData& data) const {
    // $IIHDT,x.x,T*hh
    std::stringstream ss;
    ss << "IIHDT,";
    ss << std::fixed << std::setprecision(1) << data.heading << ",T";
    
    std::string content = ss.str();
    return "$" + content + "*" + calculateChecksum(content);
}

std::string WaterSimulator::generateVHW(const Core::NavData& data) const {
    // $IIVHW,x.x,T,x.x,M,x.x,N,x.x,K*hh
    // Heading True, Heading Mag, Speed Knots, Speed KPH
    std::stringstream ss;
    ss << "IIVHW,";
    ss << std::fixed << std::setprecision(1) << data.heading << ",T,";
    ss << std::fixed << std::setprecision(1) << data.heading << ",M,"; // Using same for Mag for simplicity
    ss << std::fixed << std::setprecision(1) << data.speedThroughWater << ",N,";
    ss << std::fixed << std::setprecision(1) << (data.speedThroughWater * 1.852) << ",K";
    
    std::string content = ss.str();
    return "$" + content + "*" + calculateChecksum(content);
}

std::string WaterSimulator::calculateChecksum(const std::string& sentence) const {
    int sum = 0;
    for (char c : sentence) {
        sum ^= c;
    }
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << sum;
    return ss.str();
}

} // namespace Simulator
