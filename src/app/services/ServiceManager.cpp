#include "ServiceManager.hpp"
#include "utils/ConfigManager.hpp"
#include "network/SerialService.hpp"
#include "network/UdpService.hpp"
#include "parsers/NmeaParser.hpp"
#include "core/MessageBus.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>

namespace App {

ServiceManager::ServiceManager() {}

ServiceManager::~ServiceManager() {
    stopAll();
}

void ServiceManager::loadConfig() {
    Utils::ConfigManager::instance().load();
    sources = Utils::ConfigManager::instance().getSources();

    if (sources.empty()) {
        DataSourceConfig udpSource;
        udpSource.id = "UDP_DEFAULT";
        udpSource.name = "Default UDP Listener";
        udpSource.type = SourceType::Udp;
        udpSource.port = 10110;
        udpSource.enabled = false;
        sources.push_back(udpSource);
    }

    for (const auto& source : sources) {
        if (source.enabled) {
            updateServiceState(source);
        }
    }
}

void ServiceManager::saveConfig() {
    Utils::ConfigManager::instance().setSources(sources);
    Utils::ConfigManager::instance().save();
}

void ServiceManager::stopAll() {
    for (auto& pair : activeServices) {
        if (pair.second) pair.second->stop();
    }
    activeServices.clear();
}

void ServiceManager::stopService(const std::string& id) {
    auto it = activeServices.find(id);
    if (it != activeServices.end()) {
        if (it->second) {
            it->second->stop();
        }
        activeServices.erase(it);
    }
}

void ServiceManager::updateServiceState(const DataSourceConfig& config) {
    if (!config.enabled) {
        stopService(config.id);
        return;
    }

    stopService(config.id);

    try {
        if (config.type == SourceType::Serial) {
            auto service = std::make_unique<Network::SerialService>(config.portName, config.baudRate, 
                [this, id = config.id](const std::vector<char>& data, const std::string& source) {
                    std::string sentence(data.begin(), data.end());
                    sentence.erase(std::remove(sentence.begin(), sentence.end(), '\n'), sentence.end());
                    sentence.erase(std::remove(sentence.begin(), sentence.end(), '\r'), sentence.end());

                    std::stringstream ss(sentence);
                    std::string segment;
                    while(std::getline(ss, segment, '$')) {
                        if (segment.empty()) continue;
                        std::string fullSentence = "$" + segment;
                        
                        if (logCallback) logCallback("SERIAL:" + id, fullSentence);

                        Core::NavData navData;
                        navData.timestamp = std::chrono::system_clock::now();
                        navData.sourceId = "SERIAL:" + id;
                        
                        if (Parsers::NmeaParser::parse(fullSentence, navData)) {
                            Core::MessageBus::instance().publish(navData);
                        }
                    }
                });
            service->start();
            activeServices[config.id] = std::move(service);
        } else if (config.type == SourceType::Udp) {
            auto service = std::make_unique<Network::UdpService>(config.port, 
                [this, id = config.id](const std::vector<char>& data, const std::string& source) {
                    std::string sentence(data.begin(), data.end());
                    sentence.erase(std::remove(sentence.begin(), sentence.end(), '\n'), sentence.end());
                    sentence.erase(std::remove(sentence.begin(), sentence.end(), '\r'), sentence.end());

                    if (logCallback) logCallback("UDP:" + id, sentence);

                    Core::NavData navData;
                    navData.timestamp = std::chrono::system_clock::now();
                    navData.sourceId = "UDP:" + id;
                    
                    if (Parsers::NmeaParser::parse(sentence, navData)) {
                        Core::MessageBus::instance().publish(navData);
                    }
                });
            service->start();
            activeServices[config.id] = std::move(service);
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to start service " << config.name << ": " << e.what() << std::endl;
    }
}

} // namespace App
