#pragma once
#include "skeet/rage/owner.hpp"
namespace skeet::rage::owner_detail {
void initialize_settings(ConfigRegistries&, AimbotSettings&,
                         esp::AimbotVisualizationConfig&) noexcept;
}
