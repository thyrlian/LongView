#include "yaml_config_parser.h"
#include <fstream>
#include <sstream>

namespace LongView {
namespace Config {

Configuration YamlConfigParser::parseFromString(const std::string& content) {
    try {
        YAML::Node node = YAML::Load(content);
        Configuration config;
        
        // Parse version
        if (!node["version"]) {
            throw ConfigException("Missing version field in configuration");
        }
        config.version = node["version"].as<std::string>();
        validateVersion(config.version);
        
        // Parse groups
        if (node["groups"]) {
            config.groups = std::vector<Group>();
            for (const auto& groupNode : node["groups"]) {
                config.groups->push_back(parseGroup(groupNode));
            }
        }
        
        // Parse top-level items
        if (node["items"]) {
            config.items = std::vector<Item>();
            for (const auto& itemNode : node["items"]) {
                config.items->push_back(parseItem(itemNode));
            }
        }
        
        return config;
    } catch (const YAML::Exception& e) {
        throw ConfigParseException(e.what());
    }
}

std::string YamlConfigParser::serializeToString(const Configuration& config) {
    try {
        YAML::Node node;
        
        // Save version
        node["version"] = config.version;
        
        // Save groups
        if (config.groups) {
            for (const auto& group : *config.groups) {
                node["groups"].push_back(serializeGroup(group));
            }
        }
        
        // Save top-level items
        if (config.items) {
            for (const auto& item : *config.items) {
                node["items"].push_back(serializeItem(item));
            }
        }
        
        std::stringstream ss;
        ss << node;
        return ss.str();
    } catch (const YAML::Exception& e) {
        throw ConfigWriteException(e.what());
    }
}

Configuration YamlConfigParser::parseFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw ConfigFileAccessException("Cannot open file: " + filePath);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return parseFromString(buffer.str());
}

void YamlConfigParser::serializeToFile(const std::string& filePath, const Configuration& config) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw ConfigFileAccessException("Cannot open file for writing: " + filePath);
    }
    
    file << serializeToString(config);
}

Group YamlConfigParser::parseGroup(const YAML::Node& node) const {
    Group group;
    
    if (node["name"]) {
        group.name = node["name"].as<std::string>();
    }
    
    if (node["items"]) {
        for (const auto& itemNode : node["items"]) {
            group.items.push_back(parseItem(itemNode));
        }
    }
    
    validateGroup(group);
    return group;
}

YAML::Node YamlConfigParser::serializeGroup(const Group& group) const {
    YAML::Node node;
    
    if (group.name) {
        node["name"] = *group.name;
    }
    
    YAML::Node itemsNode;
    for (const auto& item : group.items) {
        itemsNode.push_back(serializeItem(item));
    }
    
    node["items"] = itemsNode;
    return node;
}

Item YamlConfigParser::parseItem(const YAML::Node& node) const {
    Item item;

    // Parse name
    if (node["name"]) {
        item.name = node["name"].as<std::string>();
    }

    // Parse type
    std::string typeStr = node["type"].as<std::string>();
    auto it = typeMap.find(typeStr);
    if (it == typeMap.end()) {
        throw ConfigException("Invalid type: " + typeStr);
    }
    item.type = it->second;

    // Parse value
    item.value = node["value"].as<std::string>();

    // Parse size
    if (node["size"]) {
        Size size;
        size.width = node["size"]["width"].as<int>();
        size.height = node["size"]["height"].as<int>();
        item.size = size;
    }

    // Parse refresh frequency
    if (node["refresh_frequency"]) {
        item.refresh_frequency = node["refresh_frequency"].as<int>();
    }

    validateItem(item);
    return item;
}

YAML::Node YamlConfigParser::serializeItem(const Item& item) const {
    YAML::Node node;

    // Set name
    if (item.name) {
        node["name"] = *item.name;
    }

    // Set type
    auto it = std::find_if(typeMap.begin(), typeMap.end(),
        [&item](const auto& pair) { return pair.second == item.type; });
    if (it == typeMap.end()) {
        throw ConfigException("Invalid type enum value: " + std::to_string(static_cast<int>(item.type)));
    }
    node["type"] = it->first;

    // Set value
    node["value"] = item.value;

    // Set size
    if (item.size) {
        YAML::Node sizeNode;
        sizeNode["width"] = item.size->width;
        sizeNode["height"] = item.size->height;
        node["size"] = sizeNode;
    }

    // Set refresh frequency
    if (item.refresh_frequency) {
        node["refresh_frequency"] = *item.refresh_frequency;
    }

    return node;
}

void YamlConfigParser::validateItem(const Item& item) const {
    if (item.value.empty()) {
        throw ConfigException("Item value cannot be empty");
    }
    
    if (item.size) {
        if (item.size->width <= 0 || item.size->height <= 0) {
            throw ConfigException("Item size must be positive");
        }
    }

    if (item.refresh_frequency && *item.refresh_frequency <= 0) {
        throw ConfigException("Item refresh frequency must be positive");
    }
}

void YamlConfigParser::validateGroup(const Group& group) const {
    if (group.items.empty()) {
        throw ConfigException("Group must contain at least one item");
    }
    
    for (const auto& item : group.items) {
        validateItem(item);
    }
}

void YamlConfigParser::validateVersion(const std::string& version) const {
    if (version.empty()) {
        throw ConfigException("Version cannot be empty");
    }
}

} // namespace Config
} // namespace LongView 