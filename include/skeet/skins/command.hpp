#pragma once
#include "skeet/client/command_frame.hpp"
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/skins/refresh.hpp"
#include "skeet/engine/offset_storage.hpp"

namespace skeet::ui {
class SkinsMenu;
class LegitWeaponSelector;
class RageWeaponSelector;
class RootMenu;
}
namespace skeet::skins {
// Native table 43472868, gated by classification 43474580 and the target
// group's canonical Enabled descriptor. Returns -1 when native leaves it alone.
std::int32_t weapon_rage_group(std::int32_t definition, const ui::RageWeaponSelector&,
                               std::span<const std::uint32_t, 3> classification) noexcept;
void sync_rage_weapon_group(std::int32_t definition, ui::RageWeaponSelector&,
                            std::span<const std::uint32_t, 3> classification); // 4334DB07
void sync_legit_weapon_kind(std::int32_t kind, ui::LegitWeaponSelector&); // 43398ECF

// Secondary owner4346F1F4, registered by434902DF's43345EBC call.
// Only phase 4 is substantive; the native cached definition is a signed word.
class CommandRuntime {
public:
    CommandRuntime(ui::RootMenu* const& menu, WeaponRefresh& refresh,
                   const engine::NativeOffsetGlobals& offsets,
                   std::span<const std::uint32_t, 3> classification)
        : menu_(menu), refresh_(refresh), offsets_(offsets), classification_(classification) {}
    void phase4(const client::CommandFrame&, void* local_player); // complete 43344A6C
    std::int16_t previous_weapon = 0; // native owner +18 / phase receiver +10
private:
    ui::RootMenu* const& menu_; // live 43476924, reread across widget callbacks
    WeaponRefresh& refresh_;
    const engine::NativeOffsetGlobals& offsets_;
    std::span<const std::uint32_t, 3> classification_;
};
} // namespace skeet::skins
