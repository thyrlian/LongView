#include "config_manager.h"
#include <filesystem>

namespace LongView {
namespace Config {

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager() : parser_(createConfigParser()) {
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

        // Load configuration using parser
        config_ = parser_->parseFromFile(filePath);
    } catch (const std::exception& e) {
        throw ConfigParseException(e.what());
    }
}

void ConfigManager::saveToFile(const std::string& filePath) const {
    try {
        parser_->serializeToFile(filePath, config_);
    } catch (const std::exception& e) {
        throw ConfigWriteException(e.what());
    }
}

const Configuration& ConfigManager::getConfiguration() const {
    return config_;
}

void ConfigManager::updateConfiguration(const Configuration& config) {
    config_ = config;
}

} // namespace Config
} // namespace LongView