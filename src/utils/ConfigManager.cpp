#include "ConfigManager.hpp"
#include "tinyxml2.h"
#include <iostream>

using namespace tinyxml2;

namespace Utils {

void ConfigManager::load(const std::string& filename) {
    _sources.clear();
    _outputs.clear();
    XMLDocument doc;
    if (doc.LoadFile(filename.c_str()) != XML_SUCCESS) {
        std::cerr << "Failed to load config file: " << filename << std::endl;
        return;
    }

    XMLElement* root = doc.FirstChildElement("NavOneConfig");
    if (!root) return;

    XMLElement* sourcesElem = root->FirstChildElement("DataSources");
    if (sourcesElem) {
        XMLElement* sourceElem = sourcesElem->FirstChildElement("Source");
        while (sourceElem) {
            App::DataSourceConfig config;
            const char* id = sourceElem->Attribute("id");
            const char* name = sourceElem->Attribute("name");
            const char* typeStr = sourceElem->Attribute("type");
            bool enabled = sourceElem->BoolAttribute("enabled");

            if (id) config.id = id;
            if (name) config.name = name;
            config.enabled = enabled;

            if (typeStr) {
                std::string type = typeStr;
                if (type == "Serial") {
                    config.type = App::SourceType::Serial;
                    const char* port = sourceElem->Attribute("portName");
                    int baud = sourceElem->IntAttribute("baudRate");
                    if (port) config.portName = port;
                    if (baud > 0) config.baudRate = baud;
                } else if (type == "UDP") {
                    config.type = App::SourceType::Udp;
                    int port = sourceElem->IntAttribute("port");
                    if (port > 0) config.port = port;
                }
            }

            _sources.push_back(config);
            sourceElem = sourceElem->NextSiblingElement("Source");
        }
    }

    XMLElement* outputsElem = root->FirstChildElement("DataOutputs");
    if (outputsElem) {
        XMLElement* outputElem = outputsElem->FirstChildElement("Output");
        while (outputElem) {
            App::DataOutputConfig config;
            const char* id = outputElem->Attribute("id");
            const char* name = outputElem->Attribute("name");
            const char* typeStr = outputElem->Attribute("type");
            bool enabled = outputElem->BoolAttribute("enabled");

            if (id) config.id = id;
            if (name) config.name = name;
            config.enabled = enabled;

            if (typeStr) {
                std::string type = typeStr;
                if (type == "Serial") {
                    config.type = App::OutputType::Serial;
                    const char* port = outputElem->Attribute("portName");
                    int baud = outputElem->IntAttribute("baudRate");
                    if (port) config.portName = port;
                    if (baud > 0) config.baudRate = baud;
                } else if (type == "UDP") {
                    config.type = App::OutputType::Udp;
                    const char* addr = outputElem->Attribute("address");
                    int port = outputElem->IntAttribute("port");
                    if (addr) config.address = addr;
                    if (port > 0) config.port = port;
                }
            }

            if (outputElem->Attribute("multiplexAll")) {
                config.multiplexAll = outputElem->BoolAttribute("multiplexAll");
            }

            XMLElement* sourceIdsElem = outputElem->FirstChildElement("SourceIds");
            if (sourceIdsElem) {
                XMLElement* idElem = sourceIdsElem->FirstChildElement("Id");
                while (idElem) {
                    const char* idText = idElem->GetText();
                    if (idText) config.sourceIds.push_back(idText);
                    idElem = idElem->NextSiblingElement("Id");
                }
            }

            _outputs.push_back(config);
            outputElem = outputElem->NextSiblingElement("Output");
        }
    }

    XMLElement* displayElem = root->FirstChildElement("DisplaySettings");
    if (displayElem) {
        _displayConfig.fontScale = displayElem->FloatAttribute("fontScale", 1.0f);
        _displayConfig.theme = displayElem->IntAttribute("theme", 0);
    }

    XMLElement* simElem = root->FirstChildElement("Simulator");
    if (simElem) {
        _simulatorConfig.enableGps = simElem->BoolAttribute("enableGps", true);
        _simulatorConfig.enableWind = simElem->BoolAttribute("enableWind", true);
        _simulatorConfig.enableWater = simElem->BoolAttribute("enableWater", true);
        _simulatorConfig.enableAis = simElem->BoolAttribute("enableAis", true);
        
        _simulatorConfig.startLatitude = simElem->DoubleAttribute("startLatitude", 43.2965);
        _simulatorConfig.startLongitude = simElem->DoubleAttribute("startLongitude", 5.3698);
        _simulatorConfig.baseSpeed = simElem->DoubleAttribute("baseSpeed", 10.0);
        _simulatorConfig.baseCourse = simElem->DoubleAttribute("baseCourse", 90.0);

        _simulatorConfig.minDepth = simElem->DoubleAttribute("minDepth", 5.0);
        _simulatorConfig.maxDepth = simElem->DoubleAttribute("maxDepth", 50.0);
        _simulatorConfig.minWaterTemp = simElem->DoubleAttribute("minWaterTemp", 15.0);
        _simulatorConfig.maxWaterTemp = simElem->DoubleAttribute("maxWaterTemp", 25.0);
    }
}

void ConfigManager::save(const std::string& filename) {
    XMLDocument doc;
    XMLElement* root = doc.NewElement("NavOneConfig");
    doc.InsertEndChild(root);

    XMLElement* displayElem = doc.NewElement("DisplaySettings");
    displayElem->SetAttribute("fontScale", _displayConfig.fontScale);
    displayElem->SetAttribute("theme", _displayConfig.theme);
    root->InsertEndChild(displayElem);

    XMLElement* simElem = doc.NewElement("Simulator");
    simElem->SetAttribute("enableGps", _simulatorConfig.enableGps);
    simElem->SetAttribute("enableWind", _simulatorConfig.enableWind);
    simElem->SetAttribute("enableWater", _simulatorConfig.enableWater);
    simElem->SetAttribute("enableAis", _simulatorConfig.enableAis);
    simElem->SetAttribute("startLatitude", _simulatorConfig.startLatitude);
    simElem->SetAttribute("startLongitude", _simulatorConfig.startLongitude);
    simElem->SetAttribute("baseSpeed", _simulatorConfig.baseSpeed);
    simElem->SetAttribute("baseCourse", _simulatorConfig.baseCourse);
    simElem->SetAttribute("minDepth", _simulatorConfig.minDepth);
    simElem->SetAttribute("maxDepth", _simulatorConfig.maxDepth);
    simElem->SetAttribute("minWaterTemp", _simulatorConfig.minWaterTemp);
    simElem->SetAttribute("maxWaterTemp", _simulatorConfig.maxWaterTemp);
    root->InsertEndChild(simElem);

    XMLElement* sourcesElem = doc.NewElement("DataSources");
    root->InsertEndChild(sourcesElem);

    for (const auto& source : _sources) {
        XMLElement* sourceElem = doc.NewElement("Source");
        sourceElem->SetAttribute("id", source.id.c_str());
        sourceElem->SetAttribute("name", source.name.c_str());
        sourceElem->SetAttribute("enabled", source.enabled);

        if (source.type == App::SourceType::Serial) {
            sourceElem->SetAttribute("type", "Serial");
            sourceElem->SetAttribute("portName", source.portName.c_str());
            sourceElem->SetAttribute("baudRate", source.baudRate);
        } else if (source.type == App::SourceType::Udp) {
            sourceElem->SetAttribute("type", "UDP");
            sourceElem->SetAttribute("port", source.port);
        }

        sourcesElem->InsertEndChild(sourceElem);
    }

    XMLElement* outputsElem = doc.NewElement("DataOutputs");
    root->InsertEndChild(outputsElem);

    for (const auto& output : _outputs) {
        XMLElement* outputElem = doc.NewElement("Output");
        outputElem->SetAttribute("id", output.id.c_str());
        outputElem->SetAttribute("name", output.name.c_str());
        outputElem->SetAttribute("enabled", output.enabled);

        if (output.type == App::OutputType::Serial) {
            outputElem->SetAttribute("type", "Serial");
            outputElem->SetAttribute("portName", output.portName.c_str());
            outputElem->SetAttribute("baudRate", output.baudRate);
        } else if (output.type == App::OutputType::Udp) {
            outputElem->SetAttribute("type", "UDP");
            outputElem->SetAttribute("address", output.address.c_str());
            outputElem->SetAttribute("port", output.port);
        }

        outputElem->SetAttribute("multiplexAll", output.multiplexAll);

        XMLElement* sourceIdsElem = doc.NewElement("SourceIds");
        outputElem->InsertEndChild(sourceIdsElem);
        for (const auto& id : output.sourceIds) {
            XMLElement* idElem = doc.NewElement("Id");
            idElem->SetText(id.c_str());
            sourceIdsElem->InsertEndChild(idElem);
        }

        outputsElem->InsertEndChild(outputElem);
    }

    doc.SaveFile(filename.c_str());
}

} // namespace Utils
