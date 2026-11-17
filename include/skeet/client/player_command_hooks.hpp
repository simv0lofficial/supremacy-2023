#pragma once
#include "prediction_history_owner.hpp"
#include "skeet/engine/hooks.hpp"
#include "skeet/esp/effects_config.hpp"

namespace skeet::client {
struct PlayerCommandHookFields {
    std::ptrdiff_t current_command_f58e6118;
    std::ptrdiff_t animation_state_3d52a1a7;
};
struct PlayerCommandHookBindings {
    void* const& prediction; //actual43469264; native gate is byte+18
    const esp::EffectsSettings& effects; //same disable_ragdolls43469020
    PredictionHistoryOwner* const& history; //live434759B4,canonical command tags
    PlayerCommandHookFields fields;
};

// Ordinary LOCAL-player callbacks. Original methods are always read from the
// receiver's installed vtable[-2] descriptor, not another saved callback set.
class PlayerCommandHooks {
public:
    explicit PlayerCommandHooks(const PlayerCommandHookBindings& input) noexcept
        : bindings(input) {}
    const PlayerCommandHookBindings& bindings;

    void prediction_gated_callback(void* player); //complete4331E412,slot230
    std::uint32_t ragdoll_result(void* player); //complete433AB4B6,slot298
    // Complete433D18B0,slot350. A current command establishes the native
    // nonnull animation-state/history contract before the two original args.
    void publish_animation_command(void* player, std::uintptr_t first, std::uintptr_t second);

    // Called by the canonical recipe22/10 installer after its replacement
    // table loop and before4331E834. This binds source ABI entries only.
    void bind_native_replacement(engine::VtableHookMetadata&, std::size_t slot,
                                 std::uint32_t native_entry);
    static void* native_replacement_entry(std::size_t slot, std::uint32_t native_entry);
};
} // namespace skeet::client
