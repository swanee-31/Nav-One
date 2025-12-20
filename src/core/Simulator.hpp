#pragma once

#include "NavData.hpp"
#include <vector>
#include <string>
#include <random>
#include <mutex>

namespace Core {

struct SimulatorConfig {
    bool enableGps = true;
    bool enableWind = true;
    
    double startLatitude = 43.2965; // Marseille
    double startLongitude = 5.3698;
    double baseSpeed = 10.0; // knots
    double baseCourse = 90.0; // degrees
};

class Simulator {
public:
    Simulator();
    
    void update(double dt); // dt in seconds
    
    NavData getCurrentData() const;
    std::vector<std::string> getNmeaSentences() const;
    
    void setConfig(const SimulatorConfig& config);
    SimulatorConfig getConfig() const;
    
    // Force position (e.g. from real GPS before switching to sim)
    void setPosition(double lat, double lon);

private:
    mutable std::mutex mutex;
    SimulatorConfig config;
    
    // Physics state
    double currentLat;
    double currentLon;
    double currentSog;
    double currentCog;
    
    // Variation logic
    double targetSog;
    double targetCog;
    double variationTimer = 0.0; // Counts up to 60s
    
    // Wind state
    double windAngle = 0.0;
    double windSpeed = 0.0;
    bool windClockwise = true;
    bool windSpeedIncreasing = true;
    double windTimer = 0.0; // Counts up to 60s for direction switch

    // Random
    std::mt19937 rng;
    
    void updateGps(double dt);
    void updateWind(double dt);
    
    // Helpers
    std::string generateRMC() const;
    std::string generateMWV() const;
    std::string calculateChecksum(const std::string& sentence) const;
};

} // namespace Core
