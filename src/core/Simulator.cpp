#include "Simulator.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Core {

Simulator::Simulator() : rng(std::random_device{}()) {
    currentLat = config.startLatitude;
    currentLon = config.startLongitude;
    currentSog = config.baseSpeed;
    currentCog = config.baseCourse;
    targetSog = config.baseSpeed;
    targetCog = config.baseCourse;
}

void Simulator::setConfig(const SimulatorConfig& newConfig) {
    std::lock_guard<std::mutex> lock(mutex);
    bool posChanged = (newConfig.startLatitude != config.startLatitude || newConfig.startLongitude != config.startLongitude);
    config = newConfig;
    
    if (posChanged) {
        currentLat = config.startLatitude;
        currentLon = config.startLongitude;
    }
    
    // Reset targets if base changes significantly? 
    // For now, let's just let the variation logic handle it over time, 
    // or reset immediately if the difference is large.
}

SimulatorConfig Simulator::getConfig() const {
    std::lock_guard<std::mutex> lock(mutex);
    return config;
}

void Simulator::setPosition(double lat, double lon) {
    std::lock_guard<std::mutex> lock(mutex);
    currentLat = lat;
    currentLon = lon;
    config.startLatitude = lat;
    config.startLongitude = lon;
}

void Simulator::update(double dt) {
    std::lock_guard<std::mutex> lock(mutex);
    if (config.enableGps) {
        updateGps(dt);
    }
    if (config.enableWind) {
        updateWind(dt);
    }
}

void Simulator::updateGps(double dt) {
    // 1. Variation Logic (Every 60 seconds)
    variationTimer += dt;
    if (variationTimer >= 60.0) {
        variationTimer = 0.0;
        
        // Pick new targets +/- 10% of BASE
        std::uniform_real_distribution<double> dist(-0.10, 0.10);
        
        targetSog = config.baseSpeed * (1.0 + dist(rng));
        targetCog = config.baseCourse * (1.0 + dist(rng));
        
        // Normalize COG
        if (targetCog < 0) targetCog += 360.0;
        if (targetCog >= 360.0) targetCog -= 360.0;
    }
    
    // Smoothly move current towards target
    // Simple approach: move a fraction of the difference per second
    // or linear interpolation over the remaining time?
    // Let's use a simple approach: move 5% of the difference per second (exponential smoothing)
    // But user asked for "variation lissée chaque seconde".
    
    double sogDiff = targetSog - currentSog;
    currentSog += sogDiff * dt * 0.1; // Adjust factor for smoothness
    
    double cogDiff = targetCog - currentCog;
    // Handle angle wrapping for shortest path
    if (cogDiff > 180) cogDiff -= 360;
    if (cogDiff < -180) cogDiff += 360;
    
    currentCog += cogDiff * dt * 0.1;
    if (currentCog < 0) currentCog += 360.0;
    if (currentCog >= 360.0) currentCog -= 360.0;
    
    // 2. Position Update
    // Distance in Nautical Miles
    double distNm = currentSog * (dt / 3600.0);
    
    double cogRad = currentCog * M_PI / 180.0;
    double latRad = currentLat * M_PI / 180.0;
    
    double dLat = (distNm * cos(cogRad)) / 60.0;
    double dLon = (distNm * sin(cogRad)) / (60.0 * cos(latRad));
    
    currentLat += dLat;
    currentLon += dLon;
}

void Simulator::updateWind(double dt) {
    // Wind Direction: 360 in 60s -> 6 deg/s
    double dirChange = 6.0 * dt;
    if (windClockwise) {
        windAngle += dirChange;
        if (windAngle >= 360.0) {
            windAngle -= 360.0;
            // Check if full circle completed? 
            // Requirement: "360 en sens horaire en 1 minute, puis sens anti-horaire"
            // This implies a state switch after 1 minute.
        }
    } else {
        windAngle -= dirChange;
        if (windAngle < 0.0) {
            windAngle += 360.0;
        }
    }
    
    // Wind Speed: 0 to 60 in 60s -> 1 knot/s
    double speedChange = 1.0 * dt;
    if (windSpeedIncreasing) {
        windSpeed += speedChange;
        if (windSpeed >= 60.0) {
            windSpeed = 60.0;
            windSpeedIncreasing = false; // Switch to decreasing
        }
    } else {
        windSpeed -= speedChange;
        if (windSpeed <= 0.0) {
            windSpeed = 0.0;
            windSpeedIncreasing = true; // Switch to increasing
        }
    }
    
    // Direction switch logic
    // "360 en sens horaire en 1 minute, puis sens anti-horaire"
    // This means we toggle direction every 60 seconds.
    windTimer += dt;
    if (windTimer >= 60.0) {
        windTimer = 0.0;
        windClockwise = !windClockwise;
    }
}

NavData Simulator::getCurrentData() const {
    std::lock_guard<std::mutex> lock(mutex);
    NavData data;
    data.timestamp = std::chrono::system_clock::now();
    data.sourceId = "SIMULATOR";
    
    if (config.enableGps) {
        data.latitude = currentLat;
        data.longitude = currentLon;
        data.speedOverGround = currentSog;
        data.courseOverGround = currentCog;
        data.heading = currentCog; // Assume heading = COG for sim
        data.isGpsValid = true;
    }
    
    if (config.enableWind) {
        data.windSpeed = windSpeed;
        data.windAngle = windAngle;
    }
    
    return data;
}

std::vector<std::string> Simulator::getNmeaSentences() const {
    std::lock_guard<std::mutex> lock(mutex);
    std::vector<std::string> sentences;
    if (config.enableGps) {
        sentences.push_back(generateRMC());
    }
    if (config.enableWind) {
        sentences.push_back(generateMWV());
    }
    return sentences;
}

// Helpers
std::string Simulator::calculateChecksum(const std::string& sentence) const {
    unsigned char checksum = 0;
    for (char c : sentence) {
        checksum ^= c;
    }
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)checksum;
    return ss.str();
}

std::string Simulator::generateRMC() const {
    // $GPRMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,ddmmyy,,,a*hh
    std::stringstream ss;
    
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::gmtime(&time);
    
    ss << "GPRMC,";
    ss << std::setfill('0') << std::setw(2) << tm.tm_hour
       << std::setw(2) << tm.tm_min
       << std::setw(2) << tm.tm_sec << ".00,";
       
    ss << "A,"; // Active
    
    // Lat
    double latAbs = std::abs(currentLat);
    int latDeg = (int)latAbs;
    double latMin = (latAbs - latDeg) * 60.0;
    ss << std::setw(2) << latDeg << std::fixed << std::setprecision(4) << std::setw(7) << latMin << ",";
    ss << (currentLat >= 0 ? "N" : "S") << ",";
    
    // Lon
    double lonAbs = std::abs(currentLon);
    int lonDeg = (int)lonAbs;
    double lonMin = (lonAbs - lonDeg) * 60.0;
    ss << std::setw(3) << lonDeg << std::fixed << std::setprecision(4) << std::setw(7) << lonMin << ",";
    ss << (currentLon >= 0 ? "E" : "W") << ",";
    
    // Speed
    ss << std::fixed << std::setprecision(1) << currentSog << ",";
    // Course
    ss << std::fixed << std::setprecision(1) << currentCog << ",";
    
    // Date
    ss << std::setw(2) << tm.tm_mday
       << std::setw(2) << (tm.tm_mon + 1)
       << std::setw(2) << (tm.tm_year % 100) << ",,,";
       
    // Mode
    ss << "A"; // Autonomous
    
    std::string content = ss.str();
    return "$" + content + "*" + calculateChecksum(content);
}

std::string Simulator::generateMWV() const {
    // $IIMWV,x.x,R,x.x,N,A*hh
    std::stringstream ss;
    ss << "IIMWV,";
    ss << std::fixed << std::setprecision(1) << windAngle << ",R,";
    ss << std::fixed << std::setprecision(1) << windSpeed << ",N,A";
    
    std::string content = ss.str();
    return "$" + content + "*" + calculateChecksum(content);
}

} // namespace Core
