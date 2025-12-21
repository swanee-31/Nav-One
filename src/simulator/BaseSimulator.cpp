#include "BaseSimulator.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Simulator {

BaseSimulator::BaseSimulator() : rng(std::random_device{}()) {
    currentLat = config.startLatitude;
    currentLon = config.startLongitude;
    currentSog = config.baseSpeed;
    currentCog = config.baseCourse;
    targetSog = config.baseSpeed;
    targetCog = config.baseCourse;
}

void BaseSimulator::setConfig(const SimulatorConfig& newConfig) {
    std::lock_guard<std::mutex> lock(mutex);
    bool posChanged = (newConfig.startLatitude != config.startLatitude || newConfig.startLongitude != config.startLongitude);
    config = newConfig;
    
    if (posChanged) {
        currentLat = config.startLatitude;
        currentLon = config.startLongitude;
    }
}

SimulatorConfig BaseSimulator::getConfig() const {
    std::lock_guard<std::mutex> lock(mutex);
    return config;
}

void BaseSimulator::setPosition(double lat, double lon) {
    std::lock_guard<std::mutex> lock(mutex);
    currentLat = lat;
    currentLon = lon;
    config.startLatitude = lat;
    config.startLongitude = lon;
}

void BaseSimulator::update(double dt) {
    std::lock_guard<std::mutex> lock(mutex);
    
    // 1. Variation Logic (Every 60 seconds)
    variationTimer += dt;
    if (variationTimer >= 60.0) {
        variationTimer = 0.0;
        
        std::uniform_real_distribution<double> dist(-0.10, 0.10);
        
        targetSog = config.baseSpeed * (1.0 + dist(rng));
        targetCog = config.baseCourse * (1.0 + dist(rng));
        
        if (targetCog < 0) targetCog += 360.0;
        if (targetCog >= 360.0) targetCog -= 360.0;
    }
    
    // Smooth movement
    double sogDiff = targetSog - currentSog;
    currentSog += sogDiff * dt * 0.1; 
    
    double cogDiff = targetCog - currentCog;
    if (cogDiff > 180) cogDiff -= 360;
    if (cogDiff < -180) cogDiff += 360;
    
    currentCog += cogDiff * dt * 0.1;
    if (currentCog < 0) currentCog += 360.0;
    if (currentCog >= 360.0) currentCog -= 360.0;
    
    // 2. Position Update
    double distNm = currentSog * (dt / 3600.0);
    double cogRad = currentCog * M_PI / 180.0;
    double latRad = currentLat * M_PI / 180.0;
    
    double dLat = distNm * std::cos(cogRad) / 60.0;
    double dLon = distNm * std::sin(cogRad) / (60.0 * std::cos(latRad));
    
    currentLat += dLat;
    currentLon += dLon;
}

Core::NavData BaseSimulator::getCurrentData() const {
    std::lock_guard<std::mutex> lock(mutex);
    Core::NavData data;
    data.timestamp = std::chrono::system_clock::now();
    data.sourceId = "SIMULATOR";
    
    data.latitude = currentLat;
    data.longitude = currentLon;
    data.speedOverGround = currentSog;
    data.courseOverGround = currentCog;
    
    // Base simulator provides raw physics data
    // Validity flags are set by decorators or here if we consider base as "GPS"
    // But let's leave specific flags to decorators if possible, 
    // or set basic ones here.
    // For now, let's say Base provides the "Truth".
    
    return data;
}

std::vector<std::string> BaseSimulator::getNmeaSentences() const {
    return {}; // Base simulator produces no NMEA
}

} // namespace Simulator
