#pragma once
#include "grenade_environment.hpp"
#include "grenade_auxiliary.hpp"
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/engine/startup_convars.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/movement/simulation_capture.hpp"
#include "skeet/movement/simulation_environment.hpp"

namespace skeet::engine {
class SceneQueries;
struct NativeOffsetGlobals;
}
namespace skeet::ui {
class RendererBackend;
}
namespace skeet::esp {
struct GrenadeLiveOffsets {
    explicit GrenadeLiveOffsets(const engine::NativeOffsetGlobals&);
    engine::DecodedOffset armor; //47A6F50C
    engine::DecodedOffset view_offset; //C246315E
    engine::DecodedOffset life_state; //9D920E35
    engine::DecodedOffset movement_type; //7501CB91
    engine::DecodedOffset observer_mode; //2F4E97C4
    engine::DecodedOffset observer_target; //3B704B41
    engine::DecodedOffset view_angles; //E23A9BE1
    engine::DecodedOffset alternate_eye; //D070684C
    engine::DecodedOffset player_flags; //E2FBE695
    engine::DecodedOffset weapon_definition; //5C9F0FBB
    engine::DecodedOffset weapon_throw_time; //3B93667B
    engine::DecodedOffset weapon_strength; //1B9654D9
    engine::DecodedOffset weapon_state; //9E2078A1
    engine::DecodedOffset recoil; //1BAE1168
    engine::DecodedOffset game_player_override; //D720AEA6
    engine::DecodedOffset teammate_damage_disabled; //F57BEAA9
    engine::DecodedOffset rejection_cde290d2;
    engine::DecodedOffset rejection_af649dac;
    engine::DecodedOffset duck_amount; //3309C0CF
    engine::DecodedOffset active_weapon; //CD812654
};
struct GrenadeLiveConvars {
    engine::NativeConVarReference friendly_fire; //211963A6
    engine::NativeConVarReference self_he_damage; //02081323
    engine::NativeConVarReference teammate_he_damage; //6B6A9C24
    engine::NativeConVarReference teammate_fire_damage; //D5F0540C
    engine::NativeConVarReference recoil_scale; //C429E1BF
    engine::NativeConVarReference game_type; //AC9299CC
    engine::NativeConVarReference teammates_are_enemies; //9190E2E6
};
GrenadeLiveOffsets capture_grenade_live_offsets(const engine::NativeOffsetGlobals&);
// All references bind canonical runtime cells, not copies of player/global
// state. The common query adapters have one process-lifetime physical owner.
struct GrenadeLiveBindings {
    engine::SceneQueries& queries;
    const MovementCaptureInput& movement_capture;
    const MovementEnvironmentBindings& movement_environment;
    const players::State& player_adjustments;
    client::PlayerStateCache& players;
    void* const& engine_client; //4346A514
    void* const& local_player; //43475808
    void* const& game_rules_slot; //4346A3A4, optional pointer-to-rules
    const std::int32_t& maximum_players; //*43469300
    const std::int32_t& tick_count; //*43468D4C
    const std::int32_t& command_count; //*434757DC
    const std::int32_t& last_outgoing; //*43475B2C
    const float& current_time; //*43467EB8
    const float& engine_clock_interval; //43467B04 pointee+20
    const float& interpolation; //43467B04 pointee+24
    const std::uint8_t& grenade_entities_enabled; //43467CD4
    const Projection& projection; //canonical434798E0 matrix/scales
    ui::RendererBackend& renderer; //actual render backend screen/texture storage
    const GrenadeWarningTexture& explosive_texture;
    const GrenadeWarningTexture& fire_texture;
    const GrenadeDamageMath& math;
    double (*arc_tangent)(double, double); //genuine importedatan2
    std::uint8_t* trace_flag; //actualoptional434683D0 byte
    std::uint32_t& command_buttons; //434759C0
    GrenadeLiveConvars convars;
};
} // namespace skeet::esp
