#pragma once

#include "core/NavData.hpp"
#include <string>
#include <vector>

namespace Parsers {

class NmeaParser {
public:
    // Parses a raw NMEA sentence and updates the provided NavData structure.
    // Returns true if parsing was successful.
    static bool parse(const std::string& sentence, Core::NavData& data);

private:
    static std::vector<std::string> split(const std::string& s, char delimiter);
    static bool verifyChecksum(const std::string& sentence);
    
    // Parsers for specific sentences
    static void parseRMC(const std::vector<std::string>& tokens, Core::NavData& data);
    static void parseGGA(const std::vector<std::string>& tokens, Core::NavData& data);
    static void parseMWV(const std::vector<std::string>& tokens, Core::NavData& data);
};

} // namespace Parsers
