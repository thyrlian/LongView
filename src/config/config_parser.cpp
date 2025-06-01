#include "config_parser.h"
#include "yaml_config_parser.h"

namespace LongView {
namespace Config {

std::unique_ptr<IConfigParser> createConfigParser() {
    return std::make_unique<YamlConfigParser>();
}

} // namespace Config
} // namespace LongView 