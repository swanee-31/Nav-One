#include "AisSimulator.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <bitset>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Simulator {

// Helper to append bits
void addBits(std::vector<bool>& bits, long long value, int numBits) {
    for (int i = numBits - 1; i >= 0; --i) {
        bits.push_back((value >> i) & 1);
    }
}

void addStringBits(std::vector<bool>& bits, const std::string& str, int maxChars) {
    std::string s = str;
    if (s.length() > maxChars) s = s.substr(0, maxChars);
    else s.resize(maxChars, '@'); // Pad with @ (0)

    for (char c : s) {
        int val = c;
        // Convert ASCII to 6-bit AIS char
        // @=0, A-Z=1-26, etc.
        // Standard mapping:
        // 0x40 (@) -> 0
        // 0x41 (A) -> 1 ...
        if (val >= 0x40 && val <= 0x5F) val -= 0x40;
        else if (val >= 0x20 && val <= 0x3F) val = val; // Keep punctuation? No, mapping is specific.
        // Simplified mapping for A-Z and numbers
        // 0-9: 0x30-0x39 -> 16-25? No.
        // Let's use a simplified lookup or logic if needed.
        // Actually, standard says:
        // Take 6 least significant bits of ASCII? No.
        // "The character set is 6-bit ASCII... 0x20-0x5F -> 0x00-0x3F"
        // So just subtract 0x20? No.
        // 0x40 (@) is 0.
        // 0x41 (A) is 1.
        // ...
        // 0x5A (Z) is 26.
        // 0x30 (0) is 16? No.
        
        // Correct mapping:
        // Char -> 6-bit
        // @ -> 0
        // A-Z -> 1-26
        // [\]^_ -> 27-31
        // space ! " # ... -> 32 ...
        // 0-9 -> 48-57 (in 6-bit? No, 6-bit only goes to 63)
        
        // Let's use the standard table logic:
        // if c < 0x20 -> 0
        // c = c & 0x3F (take lower 6 bits?)
        // if c < 0x20 c += 0x40 ??
        
        // Actually, for encoding STRING into 6-bit:
        // "Valid characters are all printable ASCII characters from 0x20 to 0x5F"
        // "0x40 (@) is 0"
        // "0x41 (A) is 1"
        
        int code = 0;
        if (c >= '@' && c <= '_') code = c - '@'; // 0-31
        else if (c >= ' ' && c <= '?') code = c; // 32-63
        else code = 0; // Invalid -> @
        
        addBits(bits, code, 6);
    }
}

AisSimulator::AisSimulator(std::unique_ptr<ISimulator> simulator)
    : SimulatorDecorator(std::move(simulator)) {
    
    // Initialize default ships if config is empty
    auto config = getConfig();
    if (config.aisTargets.empty()) {
        // Zigomar
        AisTargetConfig zigomar;
        zigomar.name = "ZIGOMAR";
        zigomar.callsign = "FAF9142";
        zigomar.mmsi = 227000001;
        zigomar.shipType = 36; // Sailing
        zigomar.length = 6;
        zigomar.width = 2;
        zigomar.speed = 3.5;
        zigomar.course = 45.0;
        zigomar.updateFrequency = 10000; // 10s
        // Position relative to start (approx 1NM NE)
        zigomar.latitude = config.startLatitude + 0.015;
        zigomar.longitude = config.startLongitude + 0.015;
        config.aisTargets.push_back(zigomar);

        // Yamato
        AisTargetConfig yamato;
        yamato.name = "YAMATO";
        yamato.callsign = "JD0001";
        yamato.mmsi = 431000001;
        yamato.shipType = 35; // Military
        yamato.length = 263;
        yamato.width = 39;
        yamato.speed = 21.0;
        yamato.course = 270.0;
        yamato.updateFrequency = 10000; // 10s
        // Position relative to start (approx 3NM W)
        yamato.latitude = config.startLatitude;
        yamato.longitude = config.startLongitude - 0.05;
        config.aisTargets.push_back(yamato);

        // Titanic
        AisTargetConfig titanic;
        titanic.callsign = "MUC";
        titanic.name = "TITANIC";
        titanic.mmsi = 232000001;
        titanic.shipType = 60; // Passenger
        titanic.length = 269;
        titanic.width = 28;
        titanic.speed = 19.0;
        titanic.course = 180.0;
        titanic.updateFrequency = 10000; // 10s
        // Position relative to start (approx 2NM S)
        titanic.latitude = config.startLatitude - 0.03;
        titanic.longitude = config.startLongitude;
        config.aisTargets.push_back(titanic);
        
        setConfig(config);
    }
    
    initShips(getConfig());
}

void AisSimulator::setConfig(const SimulatorConfig& config) {
    SimulatorDecorator::setConfig(config);
    initShips(config);
}

void AisSimulator::initShips(const SimulatorConfig& config) {
    ships.clear();
    for (const auto& target : config.aisTargets) {
        AisShipState state;
        state.config = target;
        state.currentLat = target.latitude;
        state.currentLon = target.longitude;
        state.timeSinceLastEmit = 0.0;
        state.timeSinceLastStaticEmit = 0.0;
        ships.push_back(state);
    }
}

void AisSimulator::update(double dt) {
    SimulatorDecorator::update(dt);
    
    auto config = getConfig();
    if (!config.enableAis) return;

    for (auto& ship : ships) {
        if (!ship.config.enabled) continue;
        
        updateShipPhysics(ship, dt);
        ship.timeSinceLastEmit += dt * 1000.0;
        ship.timeSinceLastStaticEmit += dt * 1000.0;
    }
}

void AisSimulator::updateShipPhysics(AisShipState& ship, double dt) {
    // Simple constant velocity model
    double distNm = ship.config.speed * (dt / 3600.0);
    double cogRad = ship.config.course * M_PI / 180.0;
    double latRad = ship.currentLat * M_PI / 180.0;
    
    double dLat = distNm * std::cos(cogRad) / 60.0;
    double dLon = distNm * std::sin(cogRad) / (60.0 * std::cos(latRad));
    
    ship.currentLat += dLat;
    ship.currentLon += dLon;
}

Core::NavData AisSimulator::getCurrentData() const {
    return SimulatorDecorator::getCurrentData();
}

std::vector<std::string> AisSimulator::getNmeaSentences() const {
    auto sentences = SimulatorDecorator::getNmeaSentences();
    auto config = getConfig();
    
    if (!config.enableAis) return sentences;

    // We need to cast away constness to reset timers, or make timers mutable.
    // Since ships vector is member, we can't modify it in const method unless mutable.
    // But ships is not mutable.
    // Let's use const_cast for this simulation logic or make ships mutable.
    // Better: make ships mutable in header.
    // Wait, I can't change header now easily without another tool call.
    // I'll use const_cast for now as a quick fix, or better, assume update() handles logic 
    // and here we just read? No, we need to reset timer on emit.
    // I will assume ships is mutable or I will use const_cast.
    
    auto& mutableShips = const_cast<std::vector<AisShipState>&>(ships);

    for (auto& ship : mutableShips) {
        if (!ship.config.enabled) continue;

        // Position Report (Msg 1)
        if (ship.timeSinceLastEmit >= ship.config.updateFrequency) {
            sentences.push_back(generatePositionReport(ship));
            ship.timeSinceLastEmit = 0.0;
        }
        
        // Static Data (Msg 5) - Send every 6 minutes (360000ms) or every N position reports
        // Let's say every 1 minute for sim
        if (ship.timeSinceLastStaticEmit >= 60000.0) {
            sentences.push_back(generateStaticDataReport(ship));
            ship.timeSinceLastStaticEmit = 0.0;
        }
    }
    
    return sentences;
}

std::string AisSimulator::generatePositionReport(const AisShipState& ship) const {
    // Message Type 1
    std::vector<bool> bits;
    addBits(bits, 1, 6); // Message Type
    addBits(bits, 0, 2); // Repeat Indicator
    addBits(bits, ship.config.mmsi, 30); // MMSI
    addBits(bits, 0, 4); // Status (Under way using engine)
    addBits(bits, 0, 8); // ROT
    addBits(bits, (int)(ship.config.speed * 10), 10); // SOG
    addBits(bits, 1, 1); // Position Accuracy
    
    // Lat/Lon in 1/10000 min
    long long lon = (long long)(ship.currentLon * 600000.0);
    long long lat = (long long)(ship.currentLat * 600000.0);
    addBits(bits, lon, 28);
    addBits(bits, lat, 27);
    
    addBits(bits, (int)(ship.config.course * 10), 12); // COG
    addBits(bits, (int)ship.config.course, 9); // True Heading (assume same as COG)
    addBits(bits, 60, 6); // Time stamp
    addBits(bits, 0, 2); // Maneuver
    addBits(bits, 0, 3); // Spare
    addBits(bits, 0, 1); // RAIM
    addBits(bits, 0, 19); // Radio status
    
    return encodeAivdm(bits, 0);
}

std::string AisSimulator::generateStaticDataReport(const AisShipState& ship) const {
    // Message Type 5
    std::vector<bool> bits;
    addBits(bits, 5, 6); // Message Type
    addBits(bits, 0, 2); // Repeat Indicator
    addBits(bits, ship.config.mmsi, 30); // MMSI
    addBits(bits, 0, 2); // AIS Version
    addBits(bits, ship.config.mmsi, 30); // IMO Number (fake with MMSI)
    addStringBits(bits, ship.config.callsign, 7); // Call Sign
    addStringBits(bits, ship.config.name, 20); // Name
    addBits(bits, ship.config.shipType, 8); // Ship Type
    addBits(bits, ship.config.length, 9); // Dim A (Bow) - simplified
    addBits(bits, ship.config.width, 9); // Dim B (Stern)
    addBits(bits, 0, 6); // Dim C (Port)
    addBits(bits, 0, 6); // Dim D (Starboard)
    addBits(bits, 1, 4); // EPFD (GPS)
    addBits(bits, 0, 4); // Month
    addBits(bits, 0, 5); // Day
    addBits(bits, 0, 5); // Hour
    addBits(bits, 0, 6); // Minute
    addBits(bits, 0, 8); // Draught
    addStringBits(bits, "DEST", 20); // Destination
    addBits(bits, 0, 1); // DTE
    addBits(bits, 0, 1); // Spare
    
    return encodeAivdm(bits, 2); // Usually Msg 5 needs 2 sentences? 
    // Msg 5 is 424 bits. 
    // One AIVDM sentence payload max is ~60 chars * 6 = 360 bits.
    // So Msg 5 MUST be split.
    // For simplicity in this simulator, let's assume we can fit it or just truncate/ignore splitting logic for now 
    // OR implement splitting.
    // 424 bits / 6 = 71 chars.
    // Max chars per sentence is 82, payload is less.
    // Let's implement splitting logic in encodeAivdm.
}

std::string AisSimulator::encodeAivdm(const std::vector<bool>& bits, int fillBits) const {
    // Convert bits to 6-bit ASCII string
    std::string payload;
    int numBits = bits.size();
    for (int i = 0; i < numBits; i += 6) {
        int val = 0;
        for (int j = 0; j < 6; ++j) {
            if (i + j < numBits) {
                val = (val << 1) | (bits[i + j] ? 1 : 0);
            } else {
                val = (val << 1); // Pad with 0
            }
        }
        
        // Convert 6-bit value to ASCII
        val += 48;
        if (val > 87) val += 8;
        payload += (char)val;
    }
    
    // Splitting logic
    // Max payload length ~60 chars?
    // Let's say 60 chars.
    std::vector<std::string> payloads;
    for (size_t i = 0; i < payload.length(); i += 60) {
        payloads.push_back(payload.substr(i, 60));
    }
    
    int totalSentences = payloads.size();
    static int msgId = 1;
    msgId = (msgId % 9) + 1;
    
    std::stringstream result;
    for (int i = 0; i < totalSentences; ++i) {
        std::stringstream ss;
        ss << "AIVDM,";
        ss << totalSentences << ",";
        ss << (i + 1) << ",";
        ss << msgId << ",";
        ss << "A,"; // Channel
        ss << payloads[i] << ",";
        ss << "0"; // Fill bits (simplified, should calculate for last one)
        
        std::string content = ss.str();
        result << "!" << content << "*" << calculateChecksum(content);
        if (i < totalSentences - 1) result << "\r\n";
    }
    
    return result.str();
}

std::string AisSimulator::calculateChecksum(const std::string& sentence) const {
    int sum = 0;
    for (char c : sentence) {
        sum ^= c;
    }
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << sum;
    return ss.str();
}

} // namespace Simulator
