#pragma once

#include <vector>
#include <string>

namespace Utils {

class SerialPortUtils {
public:
    struct PortInfo {
        std::string port; // e.g., "COM3" or "/dev/ttyUSB0"
        std::string description;
    };

    static std::vector<PortInfo> getAvailablePorts();
};

} // namespace Utils
