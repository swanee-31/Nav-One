#include "SerialPortUtils.hpp"
#include <filesystem>
#include <iostream>
#include <vector>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#endif

namespace Utils {

std::vector<SerialPortUtils::PortInfo> SerialPortUtils::getAvailablePorts() {
    std::vector<PortInfo> ports;

#ifdef _WIN32
    // Windows Implementation: Read from Registry
    // HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\SERIALCOMM
    HKEY hKey;
    LONG lRes = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_READ, &hKey);
    
    if (lRes == ERROR_SUCCESS) {
        char valueName[16384];
        DWORD valueNameSize;
        char data[16384];
        DWORD dataSize;
        DWORD type;
        DWORD index = 0;

        while (true) {
            valueNameSize = sizeof(valueName);
            dataSize = sizeof(data);
            
            lRes = RegEnumValueA(hKey, index, valueName, &valueNameSize, nullptr, &type, (LPBYTE)data, &dataSize);
            
            if (lRes != ERROR_SUCCESS) break;

            if (type == REG_SZ) {
                std::string portName(data);
                ports.push_back({portName, "Serial Device"});
            }
            index++;
        }
        RegCloseKey(hKey);
    } else {
        std::cerr << "Failed to open registry key for serial ports. Error: " << lRes << std::endl;
    }
#else
    // Linux/Unix Implementation: Scan /dev
    try {
        for (const auto& entry : std::filesystem::directory_iterator("/dev")) {
            std::string path = entry.path().string();
            std::string filename = entry.path().filename().string();
            
            // Common patterns for serial ports on Linux
            if (filename.find("ttyS") == 0 || 
                filename.find("ttyUSB") == 0 || 
                filename.find("ttyACM") == 0 ||
                filename.find("ttyAMA") == 0) { // Raspberry Pi UART
                
                ports.push_back({path, filename});
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error scanning /dev: " << e.what() << std::endl;
    }
#endif

    return ports;
}

} // namespace Utils
