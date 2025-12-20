#include "ServiceManager.hpp"
#include "utils/ConfigManager.hpp"
#include "network/SerialService.hpp"
#include "network/UdpService.hpp"
#include "network/UdpSender.hpp"
#include "network/SimulatorService.hpp"
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
    std::lock_guard<std::recursive_mutex> lock(mutex);
    Utils::ConfigManager::instance().load();
    sources = Utils::ConfigManager::instance().getSources();
    outputs = Utils::ConfigManager::instance().getOutputs();

    if (sources.empty()) {
        DataSourceConfig udpSource;
        udpSource.id = "UDP_DEFAULT";
        udpSource.name = "Default UDP Listener";
        udpSource.type = SourceType::Udp;
        udpSource.port = 10110;
        udpSource.enabled = false;
        sources.push_back(udpSource);
    }

    // Ensure Simulator source exists
    bool simExists = false;
    for (const auto& source : sources) {
        if (source.id == "SIMULATOR") {
            simExists = true;
            break;
        }
    }
    if (!simExists) {
        DataSourceConfig simSource;
        simSource.id = "SIMULATOR";
        simSource.name = "Internal Simulator";
        simSource.type = SourceType::Simulator;
        simSource.enabled = false; // Disabled by default as requested
        sources.push_back(simSource);
    }

    for (const auto& source : sources) {
        if (source.enabled) {
            updateServiceState(source);
        }
    }

    for (const auto& output : outputs) {
        if (output.enabled) {
            updateOutputState(output);
        }
    }
}

void ServiceManager::saveConfig() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    Utils::ConfigManager::instance().setSources(sources);
    Utils::ConfigManager::instance().setOutputs(outputs);
    Utils::ConfigManager::instance().save();
}

void ServiceManager::stopAll() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    for (auto& pair : activeServices) {
        if (pair.second) pair.second->stop();
    }
    activeServices.clear();

    for (auto& pair : activeOutputs) {
        if (pair.second) pair.second->stop();
    }
    activeOutputs.clear();
}

void ServiceManager::stopService(const std::string& id) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto it = activeServices.find(id);
    if (it != activeServices.end()) {
        if (it->second) {
            it->second->stop();
        }
        activeServices.erase(it);
    }
}

void ServiceManager::stopOutput(const std::string& id) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto it = activeOutputs.find(id);
    if (it != activeOutputs.end()) {
        if (it->second) {
            it->second->stop();
        }
        activeOutputs.erase(it);
    }
}

void ServiceManager::broadcast(const std::string& data, const std::string& sourceId) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    for (const auto& outputConfig : outputs) {
        if (!outputConfig.enabled) continue;

        // Check multiplexing rules
        bool shouldSend = outputConfig.multiplexAll;
        if (!shouldSend) {
            for (const auto& allowedId : outputConfig.sourceIds) {
                if (allowedId == sourceId) {
                    shouldSend = true;
                    break;
                }
            }
        }

        if (shouldSend) {
            auto it = activeOutputs.find(outputConfig.id);
            if (it != activeOutputs.end() && it->second && it->second->isRunning()) {
                it->second->send(data);
            }
        }
    }
}

void ServiceManager::updateOutputState(const DataOutputConfig& config) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (!config.enabled) {
        stopOutput(config.id);
        return;
    }

    stopOutput(config.id);

    try {
        if (config.type == OutputType::Serial) {
            // SerialService is bidirectional, so we can use it for output too.
            // We pass a dummy callback or nullptr if we don't care about reading.
            // But SerialService constructor requires a callback.
            // Let's pass an empty lambda.
            auto service = std::make_unique<Network::SerialService>(config.portName, config.baudRate, 
                [](const std::vector<char>&, const std::string&) {});
            service->start();
            activeOutputs[config.id] = std::move(service);
        } else if (config.type == OutputType::Udp) {
            auto service = std::make_unique<Network::UdpSender>(config.address, config.port);
            service->start();
            activeOutputs[config.id] = std::move(service);
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to start output " << config.name << ": " << e.what() << std::endl;
    }
}

void ServiceManager::updateServiceState(const DataSourceConfig& config) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (!config.enabled) {
        stopService(config.id);
        return;
    }

    stopService(config.id);

    try {
        if (config.type == SourceType::Simulator) {
            auto service = std::make_unique<Network::SimulatorService>();
            service->start();
            activeServices[config.id] = std::move(service);
            return;
        } else if (config.type == SourceType::Serial) {
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
                        
                        // Multiplexing: Broadcast raw sentence
                        broadcast(fullSentence + "\r\n", id);

                        {
                            std::lock_guard<std::recursive_mutex> cbLock(mutex);
                            if (logCallback) logCallback("SERIAL:" + id, fullSentence);
                        }

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

                    // Multiplexing: Broadcast raw sentence
                    // Note: UDP packets might contain multiple sentences or partials, but assuming line based for now or packet based.
                    // If packet based, we might want to forward the whole packet.
                    // But here we cleaned it up. Let's forward the cleaned sentence with CRLF.
                    broadcast(sentence + "\r\n", id);

                    {
                        std::lock_guard<std::recursive_mutex> cbLock(mutex);
                        if (logCallback) logCallback("UDP:" + id, sentence);
                    }

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

bool ServiceManager::isSourceEnabled(const std::string& id) const {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    for (const auto& source : sources) {
        if (source.id == id) return source.enabled;
    }
    return false;
}

} // namespace App
