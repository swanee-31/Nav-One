#pragma once
#include <string>
#include <vector>

namespace App {

enum class SourceType { Serial, Udp, Simulator };

struct DataSourceConfig {
    std::string id;
    std::string name;
    bool enabled = false;
    SourceType type;
    
    // Serial
    std::string portName;
    int baudRate = 4800;

    // Network
    int port = 10110;
};

enum class OutputType { Serial, Udp };

struct DataOutputConfig {
    std::string id;
    std::string name;
    bool enabled = false;
    OutputType type;
    
    // Serial
    std::string portName;
    int baudRate = 4800;

    // Network
    std::string address = "127.0.0.1";
    int port = 10110;

    // Multiplexing
    bool multiplexAll = true;
    std::vector<std::string> sourceIds;
};

struct DisplayConfig {
    float fontScale = 1.0f;
    int theme = 0; // 0: Dark, 1: Light, 2: Classic
};

} // namespace App
