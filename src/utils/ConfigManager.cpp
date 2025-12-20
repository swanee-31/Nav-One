#include "ConfigManager.hpp"
#include "tinyxml2.h"
#include <iostream>

using namespace tinyxml2;

namespace Utils {

void ConfigManager::load(const std::string& filename) {
    sources.clear();
    outputs.clear();
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

            sources.push_back(config);
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

            outputs.push_back(config);
            outputElem = outputElem->NextSiblingElement("Output");
        }
    }

    XMLElement* displayElem = root->FirstChildElement("DisplaySettings");
    if (displayElem) {
        displayConfig.fontScale = displayElem->FloatAttribute("fontScale", 1.0f);
        displayConfig.theme = displayElem->IntAttribute("theme", 0);
    }
}

void ConfigManager::save(const std::string& filename) {
    XMLDocument doc;
    XMLElement* root = doc.NewElement("NavOneConfig");
    doc.InsertEndChild(root);

    XMLElement* displayElem = doc.NewElement("DisplaySettings");
    displayElem->SetAttribute("fontScale", displayConfig.fontScale);
    displayElem->SetAttribute("theme", displayConfig.theme);
    root->InsertEndChild(displayElem);

    XMLElement* sourcesElem = doc.NewElement("DataSources");
    root->InsertEndChild(sourcesElem);

    for (const auto& source : sources) {
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

    for (const auto& output : outputs) {
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
