#pragma once
#include "config.hpp"
#include "skeet/client/command_frame.hpp"
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/esp/grenade_auxiliary.hpp"
#include "skeet/esp/grenade_warning_owner.hpp"
#include "skeet/esp/grenade_live.hpp"
#include "skeet/movement/release_command.hpp"
#include "skeet/movement/release_binding.hpp"
#include "skeet/movement/simulation_capture.hpp"
#include "skeet/movement/simulation_environment.hpp"

namespace skeet::engine {
struct NativeOffsetGlobals;
}
namespace skeet::misc {

struct GrenadeCommandOffsets {
    engine::DecodedOffset active_weapon; // CD812654, uint32 handle
    engine::DecodedOffset weapon_definition; // 5C9F0FBB, uint16
    engine::DecodedOffset grenade_pin_pulled; // E1DF7E9B, byte
    engine::DecodedOffset grenade_throw_time; // 3B93667B, float
    engine::DecodedOffset grenade_strength; // 1B9654D9, float
    engine::DecodedOffset player_life_state; // 9D920E35, byte
    engine::DecodedOffset player_flags; // E2FBE695, uint32
    engine::DecodedOffset player_velocity; // 36375C23, Vector3
    engine::DecodedOffset player_tick_base; // 3D10B94C, int32
    engine::DecodedOffset player_maximum_speed; // 3EB71843, float
    engine::DecodedOffset player_recoil; // 1BAE1168, three floats
    engine::DecodedOffset player_alternate_eye; // D070684C, byte
};
// Bind the actual encrypted offset operands; each use reads their live words.
GrenadeCommandOffsets capture_grenade_command_offsets(const engine::NativeOffsetGlobals&);

struct GrenadeCommandMovement {
    const MovementCaptureInput& capture;
    const MovementEnvironmentBindings& environment;
};

// Production movement binds the actual player/ConVars/engine interfaces. Each
// future tick keeps the original full Command pointer and repeats live reads.
struct GrenadeCommandScene {
    esp::GrenadeLiveScene& live;
    const GrenadeCommandMovement& movement;
    // Both are the actual optional global434759B0 owner. A present owner
    // requires its+1AC feedback; absence is the native433A1975 early return.
    MovementReleaseBinding release_movement;
    MovementFeedbackBinding movement_feedback;
};

struct GrenadeCommandBindings {
    void* const& entity_list; // actual4346A890 cell, handle lookup slot4
    void* const& input; // actual4346A894 cell, live command pointer +F0
    engine::NativeConVarReference
        recoil_scale_convar; // live4346794C^C429E1BF, terminal encoded float
    GrenadeCommandOffsets offsets;
    const std::int32_t* tick_count; // *43475A60 +4
    const float* tick_interval; // *4346A338
    const float* current_time; // *43467EB8, read before release preroll
    const std::uint8_t* release_choke_enabled; // actual43467560
    const std::uint8_t* observer_suppressed; // actual434684E0
    std::uint32_t* command_buttons; // actual434759C0
    client::CommandAngleBindings view_angles;
    const esp::GrenadeEspConfig* esp_config;
    esp::GrenadeWarningOwner* warning_owner;
    const GrenadeCommandScene& scene;
};

struct GrenadeCommandState {
    bool phase4_eligible = false; // complete owner+50
    std::int32_t release_command_number = 0; // complete owner+54
    client::Command released_command{}; // complete+178, tick overwritten +180
    client::Command captured_command{}; // complete+1E4, phase1 full command
    esp::Vector3 previous_origin{}; // complete+10C
    esp::Vector3 live_velocity{}; // complete+118
    esp::Vector3 trajectory_velocity{}; // complete+100, reset after >100-unit move
    // Native+208 aliases the first three movement floats inside+1E4.
    esp::Vector3 captured_movement() const noexcept {
        return {captured_command.movement.forward_move, captured_command.movement.side_move,
                captured_command.movement.up_move};
    }
};

inline constexpr std::uint32_t grenade_command_constructor = 0x4348675Eu;
inline constexpr std::uint32_t grenade_command_vtable = 0x4346F0ACu;
inline constexpr std::uint32_t grenade_empty_tick_vtable = 0x434704BCu;

class GrenadeCommandRuntime {
public:
    GrenadeCommandRuntime(GrenadePredictionConfig& settings, GrenadeCommandBindings engine)
        : config(settings), bindings(engine) {}
    GrenadePredictionConfig& config;
    GrenadeCommandBindings bindings;
    GrenadeCommandState state;

    // Actual secondary+8 slots1/2/4/7. The whole GrenadeOwner registers its
    // physical command receiver; slot9 returns its input limit.
    void phase1(void* player, const client::Command&); //433B7F7E
    void phase2(void* player, void* weapon, client::Command&, std::array<float, 3>& angles,
                InputBitmap); //43383139
    void phase4(client::CommandFrame&, void* player, client::Command&, InputBitmap); //4338AD80
    void phase7(const client::CommandFrame&, void* player); //4339C567
    static std::int32_t phase9(std::int32_t limit) noexcept {
        return limit;
    } //433D2AAC
    // Actual complete owner+10 EMPTY-TICK CreateMove interface, slot0.
    // Constructor433F10DB registers it in43475978/7C. Native433D2F6C also
    // supplies original-result/sample-time arguments, unused by4337DA8B.
    // This interface is distinct from the owner's+20 BA8 history observer.
    void observer0(void* player, client::Command&, InputBitmap); //4337DA8B
    // Actual+20 BA8 history/entity observer, slot1, at43337BC0.
    void update_remote_warnings(void* player, std::uintptr_t incoming);

private:
    bool release_enabled(InputBitmap) const noexcept;
    bool release_is_recent() const noexcept;
    void* active_weapon(void* player) const;
    const client::Command* previous_command(const client::Command&) const noexcept;
    void reassess_warnings(void* player);
    void process_automatic_release(client::CommandFrame&, void* player, void* weapon,
                                   client::Command&, InputBitmap);
    void process_throw_release(client::CommandFrame&, void* player, void* weapon, client::Command&,
                               InputBitmap);
};

} // namespace skeet::misc
