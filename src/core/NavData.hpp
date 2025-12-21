#pragma once

#include <string>
#include <chrono>

namespace Core {

struct NavData {
    // Timestamp of the data
    std::chrono::system_clock::time_point timestamp;
    
    // Basic Navigation Data
    double heading = 0.0;       // Degrees
    double speedOverGround = 0.0; // Knots
    double courseOverGround = 0.0; // Degrees
    
    // Position
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0; // Meters
    
    // Depth
    double depth = 0.0; // Meters
    
    // Water Environment
    double waterTemperature = 0.0; // Celsius
    double speedThroughWater = 0.0; // Knots

    // Wind
    double windSpeed = 0.0; // Knots
    double windAngle = 0.0; // Degrees relative to bow
    
    // Source ID (e.g., "NMEA_UDP_1", "SIMULATOR")
    std::string sourceId;

    // Status
    bool isGpsValid = false; // GPS Fix is valid
    
    // Data Availability Flags
    bool hasPosition = false; // Lat/Lon/Alt
    bool hasSpeed = false;    // SOG/COG
    bool hasWind = false;
    bool hasDepth = false;
    bool hasHeading = false;
    bool hasWaterTemperature = false;
    bool hasWaterSpeed = false;
};

} // namespace Core
