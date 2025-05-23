#pragma once

#include "config.h"
#include <string>
#include <stdexcept>
#include <yaml-cpp/yaml.h>

namespace LongView {
namespace Config {

class ConfigManager {
public:
    // Singleton pattern
    static ConfigManager& getInstance();

    // Delete copy constructor and assignment operator
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    // Load configuration from file
    void loadFromFile(const std::string& filePath);
    
    // Save configuration to file
    void saveToFile(const std::string& filePath) const;
    
    // Get current configuration
    const Configuration& getConfiguration() const;
    
    // Update configuration
    void updateConfiguration(const Configuration& config);
    
    // Validate configuration
    void validateConfiguration() const;

private:
    // Private constructor for singleton
    ConfigManager() = default;
    
    // Current configuration
    Configuration currentConfig_;
    
    // Internal validation methods
    void validateVersion(const std::string& version) const;
    void validateGroup(const Group& group) const;
    void validateItem(const Item& item) const;

    // Internal loading methods
    Item loadAndValidateItem(const YAML::Node& itemNode) const;

    // Internal saving methods
    YAML::Node saveItemToNode(const Item& item) const;
};

// Base exception class for all configuration errors
class ConfigException : public std::runtime_error {
public:
    explicit ConfigException(const std::string& message)
        : std::runtime_error(message) {}
};

// File not found exception
class ConfigFileNotFoundException : public ConfigException {
public:
    explicit ConfigFileNotFoundException(const std::string& filePath)
        : ConfigException("Configuration file does not exist: " + filePath) {}
};

// Empty file exception
class ConfigFileEmptyException : public ConfigException {
public:
    explicit ConfigFileEmptyException(const std::string& filePath)
        : ConfigException("Configuration file is empty: " + filePath) {}
};

// YAML parsing exception
class ConfigParseException : public ConfigException {
public:
    explicit ConfigParseException(const std::string& message)
        : ConfigException("Failed to parse YAML file: " + message) {}
};

// File access exception
class ConfigFileAccessException : public ConfigException {
public:
    explicit ConfigFileAccessException(const std::string& message)
        : ConfigException("Failed to access configuration file: " + message) {}
};

// File writing exception
class ConfigWriteException : public ConfigException {
public:
    explicit ConfigWriteException(const std::string& message)
        : ConfigException("Failed to write file: " + message) {}
};

} // namespace Config
} // namespace LongView