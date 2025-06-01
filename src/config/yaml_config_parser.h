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
};

} // namespace Config
} // namespace LongView 