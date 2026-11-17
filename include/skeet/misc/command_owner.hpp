#pragma once
#include "skeet/misc/automatic_weapons.hpp"
#include "skeet/misc/quick_switch.hpp"
#include "skeet/misc/rebuy_fix.hpp"
#include "skeet/misc/knife_runtime.hpp"
#include "skeet/misc/zeus.hpp"
#include "skeet/movement/blockbot.hpp"
#include <variant>

namespace skeet::misc {
struct AutomaticWeaponsCommandOwner {
    AutomaticWeaponsState& state;
    const AutomaticWeaponsConfig& settings;
    const AutomaticWeaponsBindings& bindings;
};
struct KnifeCommandOwner {
    const KnifeConfig& settings;
    KnifeCommandBindings bindings;
    std::uint32_t& scheduler_flags;
};
struct ZeusCommandOwner {
    ZeusState& state;
    const ZeusConfig& settings;
    const ZeusBindings& bindings;
};
struct BlockbotCommandOwner {
    BlockbotState& state;
    const BooleanConfig& enabled;
    const HotkeyConfig& hotkey;
    const BlockbotBindings& bindings;
};

// One value represents ONE native registration, preserving the caller's actual
// ordered owner vector. This does not impose a cross-constructor default order.
using CommandOwner =
    std::variant<QuickSwitchRuntime*, RebuyFixRuntime*, AutomaticWeaponsCommandOwner,
                 KnifeCommandOwner, ZeusCommandOwner, BlockbotCommandOwner>;

// Native constructor43345EBC registrants: QuickSwitch/Rebuy/Blockbot have their
// bodies at phase2, AutomaticWeapons/Knife/Zeus at phase4. The other phase0..8
// slots are the proven return-only433C95B7/4337D653/43337B12/4334A269 entries.
void dispatch_command_owner(CommandOwner&, std::uint32_t phase, client::CommandFrame*, void* player,
                            void* weapon, client::Command&, std::uint8_t& send_packet,
                            std::array<float, 3>& incoming_angles, InputBitmap);
// All six native phase9 slots point to433D2AAC, the incoming-budget identity.
std::int32_t command_owner_budget(const CommandOwner&, std::int32_t incoming) noexcept;
} //namespace skeet::misc
