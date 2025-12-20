#include "ConfigManager.hpp"
#include "tinyxml2.h"
#include <iostream>

using namespace tinyxml2;

namespace Utils {

void ConfigManager::load(const std::string& filename) {
    sources.clear();
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
}

void ConfigManager::save(const std::string& filename) {
    XMLDocument doc;
    XMLElement* root = doc.NewElement("NavOneConfig");
    doc.InsertEndChild(root);

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

    doc.SaveFile(filename.c_str());
}

} // namespace Utils
