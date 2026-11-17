#pragma once
#include "model_render.hpp"
#include "model_entity_relationships.hpp"

namespace skeet::effects {
struct EffectsCommandOwner;
struct PlayerOpacityOffsets {
    std::ptrdiff_t field_af649dac;
    std::ptrdiff_t local_player_d070684c;
    std::ptrdiff_t view_offset_c246315e;
    std::ptrdiff_t duck_amount_3309c0cf;
    std::ptrdiff_t smoke_effect_58ae2423;
    std::ptrdiff_t smoke_tick_f267e956;
};
struct PlayerOpacityBindings {
    EffectsCommandOwner* const& active_effects; //actual nullable43475A1C owner
    const esp::ColoredModelsSettings& settings; //same source descriptors as model rendering
    const ModelEntityRelationships& entities;
    const players::State& player_adjustments; //canonical4347BE08, no copy
    void* input; //actual4346A894, virtual32(-1)
    void* const& smoke_registry; //actual434679C0, handles data0/countC
    const std::int32_t* const& smoke_clock; //pointed43468D4C
    const float& interval; //pointed4346A338
    const RageVector& camera_position; //43479CCC
    const std::uint8_t& suspended; //43468E58
    engine::NativeConVarReference lag_compensation; //live43467A78 XOR3AE06513
    PlayerOpacityOffsets offsets;
};
// Complete433CC653: target origin within149 units of an active smoke entity
// whose elapsed effect time is strictly between0 and18 seconds.
bool player_inside_active_smoke(void* player, const PlayerOpacityBindings&);
// Complete4341D06C plus43367C8B: target eye within36 units of the finite
// local-eye/camera segment. A dead/absent local player returnsfalse.
bool player_near_camera_segment(void* player, const PlayerOpacityBindings&);
// Complete4336E8B8. Uses the native live-entity/nonzero-index contract and the
// same colored-model descriptors/PLAYERS table as rendering. No ESPfade copy.
std::uint8_t evaluate_player_opacity(void* player, const PlayerOpacityBindings&);
// Complete43340082 including4339EBB5's genuine entity alpha virtual395.
// Owner absence skips all calls; mode396 receives0/4 and the originaltrue byte.
void apply_player_opacity(void* player, const PlayerOpacityBindings&);
} // namespace skeet::effects
