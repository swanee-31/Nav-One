#include "BaseSimulator.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Simulator {

BaseSimulator::BaseSimulator() : _rng(std::random_device{}()) {
    _currentLat = _config.startLatitude;
    _currentLon = _config.startLongitude;
    _currentSog = _config.baseSpeed;
    _currentCog = _config.baseCourse;
    _targetSog = _config.baseSpeed;
    _targetCog = _config.baseCourse;
}

void BaseSimulator::setConfig(const SimulatorConfig& newConfig) {
    std::lock_guard<std::mutex> lock(_mutex);
    bool posChanged = (newConfig.startLatitude != _config.startLatitude || newConfig.startLongitude != _config.startLongitude);
    _config = newConfig;
    
    if (posChanged) {
        _currentLat = _config.startLatitude;
        _currentLon = _config.startLongitude;
    }
}

SimulatorConfig BaseSimulator::getConfig() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _config;
}

void BaseSimulator::setPosition(double lat, double lon) {
    std::lock_guard<std::mutex> lock(_mutex);
    _currentLat = lat;
    _currentLon = lon;
    _config.startLatitude = lat;
    _config.startLongitude = lon;
}

void BaseSimulator::update(double dt) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    // 1. Variation Logic (Every 60 seconds)
    _variationTimer += dt;
    if (_variationTimer >= 60.0) {
        _variationTimer = 0.0;
        
        std::uniform_real_distribution<double> dist(-0.10, 0.10);
        
        _targetSog = _config.baseSpeed * (1.0 + dist(_rng));
        _targetCog = _config.baseCourse * (1.0 + dist(_rng));
        
        if (_targetCog < 0) _targetCog += 360.0;
        if (_targetCog >= 360.0) _targetCog -= 360.0;
    }
    
    // Smooth movement
    double sogDiff = _targetSog - _currentSog;
    _currentSog += sogDiff * dt * 0.1; 
    
    double cogDiff = _targetCog - _currentCog;
    if (cogDiff > 180) cogDiff -= 360;
    if (cogDiff < -180) cogDiff += 360;
    
    _currentCog += cogDiff * dt * 0.1;
    if (_currentCog < 0) _currentCog += 360.0;
    if (_currentCog >= 360.0) _currentCog -= 360.0;
    
    // 2. Position Update
    double distNm = _currentSog * (dt / 3600.0);
    double cogRad = _currentCog * M_PI / 180.0;
    double latRad = _currentLat * M_PI / 180.0;
    
    double dLat = distNm * std::cos(cogRad) / 60.0;
    double dLon = distNm * std::sin(cogRad) / (60.0 * std::cos(latRad));
    
    _currentLat += dLat;
    _currentLon += dLon;
}

Core::NavData BaseSimulator::getCurrentData() const {
    std::lock_guard<std::mutex> lock(_mutex);
    Core::NavData data;
    data.timestamp = std::chrono::system_clock::now();
    data.sourceId = "SIMULATOR";
    
    data.latitude = _currentLat;
    data.longitude = _currentLon;
    data.speedOverGround = _currentSog;
    data.courseOverGround = _currentCog;
    
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
