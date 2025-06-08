#include "yaml_config_parser.h"
#include <fstream>
#include <sstream>

namespace LongView {
namespace Config {

void YamlConfigParser::trackNode(const std::string& type, const std::string& name, const YAML::Node& node) const {
    lastParsedNode_ = {
        type,
        name,
        static_cast<size_t>(node.Mark().line + 1),  // YAML line numbers are 0-based
        node.as<std::string>()
    };
}

void YamlConfigParser::handleParseError(const std::string& context, const YAML::Node& node, const std::exception& e, bool addLastParsedInfo) const {
    std::stringstream ss;
    
    // Get the original error message without any redundant prefixes
    std::string errorMsg = e.what();
    const std::string prefix = "Failed to parse configuration file: ";
    size_t prefixPos = errorMsg.find(prefix);
    while (prefixPos != std::string::npos) {
        errorMsg = errorMsg.substr(prefixPos + prefix.length());
        prefixPos = errorMsg.find(prefix);
    }
    
    // Handle line number display
    int lineNumber = node.Mark().line + 1;
    if (lineNumber > 0) {
        // If the error message already contains line information, just use it
        if (errorMsg.find("at line") != std::string::npos) {
            ss << errorMsg;
        } else {
            ss << "Error parsing " << context << " at line " << lineNumber << ": " << errorMsg;
        }
    } else {
        ss << "Error parsing " << context << ": " << errorMsg;
    }
    
    // Add last successfully parsed node info if available and requested
    if (addLastParsedInfo && lastParsedNode_.lineNumber > 0) {
        // Only show last parsed info if it's different from the current error location
        if (lastParsedNode_.lineNumber != static_cast<size_t>(lineNumber)) {
            ss << "\nLast successfully parsed: " << lastParsedNode_.nodeType;
            if (!lastParsedNode_.nodeName.empty()) {
                ss << " '" << lastParsedNode_.nodeName << "'";
            }
            ss << " at line " << lastParsedNode_.lineNumber;
        }
    }
    
    throw ConfigParseException(ss.str());
}

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
        trackNode("version", "version", node["version"]);
        
        // Parse groups
        if (node["groups"]) {
            config.groups = std::vector<Group>();
            for (const auto& groupNode : node["groups"]) {
                try {
                    config.groups->push_back(parseGroup(groupNode));
                } catch (const ConfigException& e) {
                    handleParseError("group", groupNode, e, false);  // Don't add last parsed info here
                }
            }
        }
        
        // Parse top-level items
        if (node["items"]) {
            config.items = std::vector<Item>();
            for (const auto& itemNode : node["items"]) {
                try {
                    config.items->push_back(parseItem(itemNode));
                } catch (const ConfigException& e) {
                    handleParseError("item", itemNode, e, false);  // Don't add last parsed info here
                }
            }
        }
        
        return config;
    } catch (const YAML::Exception& e) {
        handleParseError("configuration", YAML::Node(), e, true);  // Add last parsed info for YAML errors
        return Configuration();  // This line will never be reached due to the throw in handleParseError
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
        trackNode("group", *group.name, node["name"]);
    }
    
    if (node["items"]) {
        for (const auto& itemNode : node["items"]) {
            try {
                group.items.push_back(parseItem(itemNode));
            } catch (const ConfigException& e) {
                std::string context = "item in group '" + (group.name ? *group.name : "unnamed") + "'";
                handleParseError(context, itemNode, e, true);  // Add last parsed info at the innermost level
            }
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
        trackNode("item", *item.name, node["name"]);
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