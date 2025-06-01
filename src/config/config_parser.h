#pragma once

#include "config.h"
#include <string>
#include <memory>

namespace LongView {
namespace Config {

// Abstract interface for configuration parsing
class IConfigParser {
public:
    virtual ~IConfigParser() = default;
    
    // Parse configuration from string
    virtual Configuration parseFromString(const std::string& content) = 0;
    
    // Serialize configuration to string
    virtual std::string serializeToString(const Configuration& config) = 0;
    
    // Parse configuration from file
    virtual Configuration parseFromFile(const std::string& filePath) = 0;
    
    // Serialize configuration to file
    virtual void serializeToFile(const std::string& filePath, const Configuration& config) = 0;
};

// Factory function to create parser
std::unique_ptr<IConfigParser> createConfigParser();

} // namespace Config
} // namespace LongView 