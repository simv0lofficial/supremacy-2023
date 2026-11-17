#pragma once
#include "skeet/config.hpp"
#include <span>

namespace skeet::ui::menu_detail {
struct Requirement {
    std::uint32_t hash;
    ConfigType type;
};
ConfigDescriptor& descriptor(ConfigRegistries& settings, std::uint32_t hash);
void require_settings(ConfigRegistries& settings, std::span<const Requirement> requirements);
template <class Config> auto* value(ConfigRegistries& settings, std::uint32_t hash) {
    return &static_cast<Config&>(descriptor(settings, hash)).value;
}
}
