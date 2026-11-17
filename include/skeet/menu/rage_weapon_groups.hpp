#pragma once
#include "skeet/config_storage/storage.hpp"
#include "skeet/config_storage/config_stream_view.hpp"

namespace skeet::ui {
class MenuWindow;

struct RageWeaponGroupState {
    MenuWindow& window;
    ConfigLink& registry;
    std::span<ConfigBlob*, 12> groups;
    ConfigStreamView baseline_a;
    ConfigStreamView baseline_b;
    std::uint32_t last_group = UINT32_MAX;
};

// 0x433ABA07: the selected group is read once, before saving the old group.
// A null setting selects group zero. An unchanged or out-of-range selection
// does not save, load, or refresh widgets.
void switch_rage_weapon_group(RageWeaponGroupState& state, const std::uint32_t* selected_group);
}
