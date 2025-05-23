#include "config_manager.h"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace LongView {
namespace Config {

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

void ConfigManager::loadFromFile(const std::string& filePath) {
    try {
        // Check if file exists
        if (!std::filesystem::exists(filePath)) {
            throw ConfigFileNotFoundException(filePath);
        }

        // Check if file is empty
        if (std::filesystem::is_empty(filePath)) {
            throw ConfigFileEmptyException(filePath);
        }

        // Try to load the file
        YAML::Node config;
        try {
            config = YAML::LoadFile(filePath);
        } catch (const YAML::ParserException& e) {
            throw ConfigParseException(e.what());
        } catch (const YAML::BadFile& e) {
            throw ConfigFileAccessException(e.what());
        }
        
        // Clear existing data
        currentConfig_.groups.reset();
        currentConfig_.items.reset();
        
        // Load and validate version
        if (!config["version"]) {
            throw ConfigException("Missing version field in configuration");
        }
        currentConfig_.version = config["version"].as<std::string>();
        validateVersion(currentConfig_.version);
        
        // Load and validate groups
        if (config["groups"]) {
            currentConfig_.groups = std::vector<Group>();
            for (const auto& groupNode : config["groups"]) {
                Group group;
                
                if (groupNode["name"]) {
                    group.name = groupNode["name"].as<std::string>();
                }
                
                if (groupNode["items"]) {
                    for (const auto& itemNode : groupNode["items"]) {
                        group.items.push_back(loadAndValidateItem(itemNode));
                    }
                }
                
                validateGroup(group);
                currentConfig_.groups->push_back(group);
            }
        }
        
        // Load and validate top-level items
        if (config["items"]) {
            currentConfig_.items = std::vector<Item>();
            for (const auto& itemNode : config["items"]) {
                currentConfig_.items->push_back(loadAndValidateItem(itemNode));
            }
        }
        
        validateConfiguration();
    } catch (const YAML::Exception& e) {
        throw ConfigParseException(e.what());
    }
}

void ConfigManager::saveToFile(const std::string& filePath) const {
    try {
        YAML::Node config;
        
        // Save version
        config["version"] = currentConfig_.version;
        
        // Save groups
        if (currentConfig_.groups) {
            for (const auto& group : *currentConfig_.groups) {
                YAML::Node groupNode;
                
                if (group.name) {
                    groupNode["name"] = *group.name;
                }
                
                YAML::Node itemsNode;
                for (const auto& item : group.items) {
                    itemsNode.push_back(saveItemToNode(item));
                }
                
                groupNode["items"] = itemsNode;
                config["groups"].push_back(groupNode);
            }
        }
        
        // Save top-level items
        if (currentConfig_.items) {
            for (const auto& item : *currentConfig_.items) {
                config["items"].push_back(saveItemToNode(item));
            }
        }
        
        std::ofstream fout(filePath);
        fout << config;
    } catch (const YAML::Exception& e) {
        throw ConfigWriteException(e.what());
    }
}

const Configuration& ConfigManager::getConfiguration() const {
    return currentConfig_;
}

void ConfigManager::updateConfiguration(const Configuration& config) {
    currentConfig_ = config;
    validateConfiguration();
}

void ConfigManager::validateConfiguration() const {
    try {
        validateVersion(currentConfig_.version);
        
        if (currentConfig_.groups) {
            for (const auto& group : *currentConfig_.groups) {
                validateGroup(group);
            }
        }
        
        if (currentConfig_.items) {
            for (const auto& item : *currentConfig_.items) {
                validateItem(item);
            }
        }
    } catch (const ConfigException& e) {
        throw;  // Re-throw the exception
    }
}

void ConfigManager::validateVersion(const std::string& version) const {
    if (version.empty()) {
        throw ConfigException("Version cannot be empty");
    }
}

void ConfigManager::validateGroup(const Group& group) const {
    if (group.items.empty()) {
        throw ConfigException("Group must contain at least one item");
    }
    
    for (const auto& item : group.items) {
        validateItem(item);
    }
}

void ConfigManager::validateItem(const Item& item) const {
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

Item ConfigManager::loadAndValidateItem(const YAML::Node& itemNode) const {
    Item item;

    // Load name
    if (itemNode["name"]) {
        item.name = itemNode["name"].as<std::string>();
    }

    // Load type
    std::string typeStr = itemNode["type"].as<std::string>();
    auto it = typeMap.find(typeStr);
    if (it == typeMap.end()) {
        throw ConfigException("Invalid type: " + typeStr);
    }
    item.type = it->second;

    // Load value
    item.value = itemNode["value"].as<std::string>();

    // Load size
    if (itemNode["size"]) {
        Size size;
        size.width = itemNode["size"]["width"].as<int>();
        size.height = itemNode["size"]["height"].as<int>();
        item.size = size;
    }

    // Load refresh frequency
    if (itemNode["refresh_frequency"]) {
        item.refresh_frequency = itemNode["refresh_frequency"].as<int>();
    }

    validateItem(item);
    return item;
}

YAML::Node ConfigManager::saveItemToNode(const Item& item) const {
    YAML::Node itemNode;

    // Set name
    if (item.name) {
        itemNode["name"] = *item.name;
    }

    // Set type
    // Find the string value for the type by searching typeMap
    auto it = std::find_if(typeMap.begin(), typeMap.end(),
        [&item](const auto& pair) { return pair.second == item.type; });
    if (it == typeMap.end()) {
        throw ConfigException("Invalid type enum value: " + std::to_string(static_cast<int>(item.type)));
    }
    itemNode["type"] = it->first;

    // Set value
    itemNode["value"] = item.value;

    // Set size
    if (item.size) {
        YAML::Node sizeNode;
        sizeNode["width"] = item.size->width;
        sizeNode["height"] = item.size->height;
        itemNode["size"] = sizeNode;
    }

    // Set refresh frequency
    if (item.refresh_frequency) {
        itemNode["refresh_frequency"] = *item.refresh_frequency;
    }

    return itemNode;
}

} // namespace Config
} // namespace LongView