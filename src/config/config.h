#pragma once

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

namespace LongView {
namespace Config {

// Type enum
enum class Type {
    Web,    // URL
    IFrame, // IFrame content
    Image   // Image URL
};

// Type mapping
const std::unordered_map<std::string, Type> typeMap = {
    {"web", Type::Web},
    {"iframe", Type::IFrame},
    {"image", Type::Image}
};

// Size structure
struct Size {
    int width;
    int height;
};

// Item structure
struct Item {
    std::optional<std::string> name;
    Type type;
    std::string value;
    std::optional<Size> size;
    std::optional<int> refresh_frequency;  // in seconds
};

// Group structure
struct Group {
    std::optional<std::string> name;
    std::vector<Item> items;
};

// Main configuration structure
struct Configuration {
    std::string version;
    std::optional<std::vector<Group>> groups;
    std::optional<std::vector<Item>> items;
};

} // namespace Config
} // namespace LongView