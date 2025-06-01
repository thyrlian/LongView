#pragma once

#include "config.h"
#include "config_parser.h"
#include "config_exceptions.h"
#include <string>
#include <memory>

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

private:
    // Private constructor for singleton
    ConfigManager();
    
    // Configuration
    Configuration config_;
    
    // Parser instance
    std::unique_ptr<IConfigParser> parser_;
};

} // namespace Config
} // namespace LongView