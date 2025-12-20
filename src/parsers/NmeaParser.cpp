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

void NmeaParser::parseRMC(const std::vector<std::string>& tokens, Core::NavData& data) {
    // $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
    // 0: ID
    // 1: Time
    // 2: Status (A=Active)
    // 3: Lat
    // 4: N/S
    // 5: Lon
    // 6: E/W
    // 7: Speed (knots)
    // 8: Track angle (True)
    
    if (tokens.size() < 9) return;

    if (tokens[2] == "A") {
        // Speed
        if (!tokens[7].empty()) data.speedOverGround = std::stod(tokens[7]);
        
        // Course / Heading
        if (!tokens[8].empty()) {
            data.courseOverGround = std::stod(tokens[8]);
            data.heading = data.courseOverGround; // Approximation
        }
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
