#pragma once
#include <string>

namespace App {

enum class SourceType { Serial, Udp };

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

} // namespace App
