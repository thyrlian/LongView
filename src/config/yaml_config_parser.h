#pragma once

#include "config_parser.h"
#include "config_exceptions.h"
#include <yaml-cpp/yaml.h>

namespace LongView {
namespace Config {

class YamlConfigParser : public IConfigParser {
public:
    Configuration parseFromString(const std::string& content) override;
    std::string serializeToString(const Configuration& config) override;
    Configuration parseFromFile(const std::string& filePath) override;
    void serializeToFile(const std::string& filePath, const Configuration& config) override;

private:
    // Internal helper methods
    Item parseItem(const YAML::Node& node) const;
    YAML::Node serializeItem(const Item& item) const;
    Group parseGroup(const YAML::Node& node) const;
    YAML::Node serializeGroup(const Group& group) const;
    void validateItem(const Item& item) const;
    void validateGroup(const Group& group) const;
    void validateVersion(const std::string& version) const;

    // Track last successfully parsed node
    struct LastParsedNode {
        std::string nodeType;  // "item", "group", "version", etc.
        std::string nodeName;  // Name of the node if available
        size_t lineNumber;     // Line number in the YAML file
        std::string content;   // Content of the node
    };
    mutable LastParsedNode lastParsedNode_;

    // Helper methods for tracking and error handling
    void trackNode(const std::string& type, const std::string& name, const YAML::Node& node) const;
    void handleParseError(const std::string& context, const YAML::Node& node, const std::exception& e, bool addLastParsedInfo = true) const;
};

} // namespace Config
} // namespace LongView 