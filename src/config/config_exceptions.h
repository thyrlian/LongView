#pragma once

#include <stdexcept>
#include <string>

namespace LongView {
namespace Config {

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

// Parse exception
class ConfigParseException : public ConfigException {
public:
    explicit ConfigParseException(const std::string& message)
        : ConfigException("Failed to parse configuration file: " + message) {}
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