#include "NmeaParser.hpp"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

namespace Parsers {

bool NmeaParser::parse(const std::string& sentence, Core::NavData& data) {
    if (sentence.empty() || sentence[0] != '$') return false;

    // Checksum validation
    if (!verifyChecksum(sentence)) {
        return false;
    }

    // Extract content between '$' and '*'
    size_t starPos = sentence.find('*');
    std::string cleanSentence = sentence.substr(1, starPos - 1);
    
    auto tokens = split(cleanSentence, ',');
    if (tokens.empty()) return false;

    // Get Talker ID and Sentence Type (e.g., "GPRMC" -> "RMC")
    std::string header = tokens[0];
    if (header.length() < 3) return false;
    
    std::string type = header.substr(header.length() - 3);

    try {
        if (type == "RMC") {
            parseRMC(tokens, data);
            return true;
        } else if (type == "GGA") {
            parseGGA(tokens, data);
            return true;
        } else if (type == "MWV") {
            parseMWV(tokens, data);
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Parse Error: " << e.what() << std::endl;
        return false;
    }

    return false;
}

std::vector<std::string> NmeaParser::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    // Handle case where last token is empty (e.g., "data,")
    if (!s.empty() && s.back() == delimiter) {
        tokens.push_back("");
    }
    return tokens;
}

bool NmeaParser::verifyChecksum(const std::string& sentence) {
    size_t starPos = sentence.find('*');
    if (starPos == std::string::npos || starPos + 3 > sentence.length()) return false;

    unsigned char calculated = 0;
    for (size_t i = 1; i < starPos; ++i) {
        calculated ^= sentence[i];
    }

    std::string checksumStr = sentence.substr(starPos + 1, 2);
    unsigned int provided;
    std::stringstream ss;
    ss << std::hex << checksumStr;
    ss >> provided;

    return calculated == provided;
}

double convertNmeaCoord(const std::string& val, const std::string& dir) {
    if (val.empty()) return 0.0;
    double raw = std::stod(val);
    int degrees = static_cast<int>(raw / 100);
    double minutes = raw - (degrees * 100);
    double decimal = degrees + (minutes / 60.0);
    if (dir == "S" || dir == "W") decimal = -decimal;
    return decimal;
}

void NmeaParser::parseRMC(const std::vector<std::string>& tokens, Core::NavData& data) {
    // $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
    // 0: ID
    // 1: Time (HHMMSS)
    // 2: Status (A=Active)
    // 3: Lat
    // 4: N/S
    // 5: Lon
    // 6: E/W
    // 7: Speed (knots)
    // 8: Track angle (True)
    // 9: Date (DDMMYY)
    
    if (tokens.size() < 10) return;

    // Parse regardless of status (A or V) to allow debugging / partial data
    // if (tokens[2] == "A") {
    {
        data.isGpsValid = (tokens[2] == "A");

        // Position
        if (!tokens[3].empty() && !tokens[4].empty()) {
            data.latitude = convertNmeaCoord(tokens[3], tokens[4]);
        }
        if (!tokens[5].empty() && !tokens[6].empty()) {
            data.longitude = convertNmeaCoord(tokens[5], tokens[6]);
        }

        // Speed
        if (!tokens[7].empty()) data.speedOverGround = std::stod(tokens[7]);
        
        // Course / Heading
        if (!tokens[8].empty()) {
            data.courseOverGround = std::stod(tokens[8]);
            data.heading = data.courseOverGround; // Approximation
        }

        // Date & Time
        if (!tokens[1].empty() && !tokens[9].empty() && tokens[1].length() >= 6 && tokens[9].length() == 6) {
            std::tm tm = {};
            try {
                // Time: HHMMSS
                tm.tm_hour = std::stoi(tokens[1].substr(0, 2));
                tm.tm_min = std::stoi(tokens[1].substr(2, 2));
                tm.tm_sec = std::stoi(tokens[1].substr(4, 2));
                
                // Date: DDMMYY
                tm.tm_mday = std::stoi(tokens[9].substr(0, 2));
                tm.tm_mon = std::stoi(tokens[9].substr(2, 2)) - 1; // 0-11
                int year = std::stoi(tokens[9].substr(4, 2));
                tm.tm_year = (year < 80 ? 2000 + year : 1900 + year) - 1900; // Years since 1900

                time_t time = _mkgmtime(&tm); // Use _mkgmtime for UTC on Windows
                if (time != -1) {
                    data.timestamp = std::chrono::system_clock::from_time_t(time);
                }
            } catch (...) {
                // Ignore parsing errors
            }
        }
    }
}

void NmeaParser::parseGGA(const std::vector<std::string>& tokens, Core::NavData& data) {
    // $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
    // 0: ID
    // 1: Time
    // 2: Lat
    // 3: N/S
    // 4: Lon
    // 5: E/W
    // 6: Quality
    // 7: Satellites
    // 8: HDOP
    // 9: Altitude
    // 10: Altitude Unit (M)
    
    if (tokens.size() < 10) return;

    // Quality indicator: 0 = Invalid, 1 = GPS fix, 2 = DGPS fix, etc.
    try {
        int quality = std::stoi(tokens[6]);
        data.isGpsValid = (quality > 0);
    } catch (...) {
        data.isGpsValid = false;
    }

    // Position
    if (!tokens[2].empty() && !tokens[3].empty()) {
        data.latitude = convertNmeaCoord(tokens[2], tokens[3]);
    }
    if (!tokens[4].empty() && !tokens[5].empty()) {
        data.longitude = convertNmeaCoord(tokens[4], tokens[5]);
    }
    
    // Altitude
    if (!tokens[9].empty()) {
        try {
            data.altitude = std::stod(tokens[9]);
        } catch (...) {}
    }
}

void NmeaParser::parseMWV(const std::vector<std::string>& tokens, Core::NavData& data) {
    // $IIMWV,084.0,R,10.4,N,A*04
    // 1: Wind Angle
    // 2: Reference (R/T)
    // 3: Wind Speed
    // 4: Unit (N/K/M)
    // 5: Status
    
    if (tokens.size() < 6) return;
    
    if (tokens[5] == "A") {
        if (!tokens[1].empty()) data.windAngle = std::stod(tokens[1]);
        if (!tokens[3].empty()) data.windSpeed = std::stod(tokens[3]);
    }
}

} // namespace Parsers
