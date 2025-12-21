#pragma once

#include <string>
#include <vector>

namespace Simulator {

struct AisTargetConfig {
    std::string name;
    std::string callsign;
    int mmsi;
    int shipType; // 30=Fishing, 36=Sailing, 35=Military, 60=Passenger
    int length; // meters
    int width; // meters
    double speed; // knots
    double course; // degrees
    double latitude;
    double longitude;
    bool enabled = true;
    int updateFrequency = 10000; // ms
};

struct SimulatorConfig {
    bool enableGps = true;
    bool enableWind = true;
    bool enableWater = true;
    bool enableAis = true;
    
    double startLatitude = 43.2965; // Marseille
    double startLongitude = 5.3698;
    double baseSpeed = 10.0; // knots
    double baseCourse = 90.0; // degrees

    // Water Simulation
    double minDepth = 5.0; // meters
    double maxDepth = 50.0; // meters
    double minWaterTemp = 15.0; // Celsius
    double maxWaterTemp = 25.0; // Celsius

    // AIS Simulation
    std::vector<AisTargetConfig> aisTargets;

    // Frequencies (ms)
    int gpsFrequency = 1000;
    int windFrequency = 1000;
    int waterFrequency = 1000;
};

} // namespace Simulator
