#pragma once
#include "screen_overlay.hpp"
#include "skeet/client/commands.hpp"
#include "skeet/legit/runtime.hpp"
#include "skeet/lagcomp/interpolation.hpp"
#include "skeet/prediction/history.hpp"
#include "skeet/rage/spread.hpp"
#include "skeet/rage/settings.hpp"

namespace skeet::hud {
// The Remove recoil descriptor (63660060), looked up at434866A8..434866B8,
// supplies full owner+58. Its lifetime is the canonical settings lifetime.
void bind_screen_overlay_recoil_config(ScreenOverlayState&, const rage::GlobalSettings&) noexcept;
void reset_screen_overlay_state(ScreenOverlayState&) noexcept; // complete433CDDCF
struct ScreenOverlayFields {
    std::ptrdiff_t weapon_definition; // 5C9F0FBB
    std::ptrdiff_t weapon_recoil_index; // 6777C9CE
    std::ptrdiff_t recoil_angles; // 1BAE1168
    std::ptrdiff_t recoil_velocity; // 48442721
    std::ptrdiff_t weapon_time_f71ce331;
    std::ptrdiff_t player_time_0176a381;
    std::ptrdiff_t player_time_de530a8b;
    std::ptrdiff_t weapon_penalty_d2563274;
};
// Proven fields of the shared weapon context consumed by this owner. Spread
// uses the existing canonical +C4/+D0 projection, rather than another model.
struct ScreenOverlayCommandFrame {
    std::uint32_t flags;
    void* weapon;
    std::array<float, 2> recoil_view; // native context +9C/+A0
    rage::SpreadParameters spread;
};
ScreenOverlayCommandFrame screen_overlay_command_frame(const void* native_context);
struct ScreenOverlayUpdateBindings {
    void* engine; // virtual5 screen dimensions
    void* input; // virtual32(-1), native final radius adjustment
    void* cvar; // native linked command list at +30
    const float* current_time;
    const float* interval;
    const float* receive_interval;
    const float* field_of_view;
    engine::ConVarBinding recoil_scale; //ConVar4346794C ^C429E1BF
    engine::ConVarBinding first_decay; //ConVar43467104 ^E974CD65
    engine::ConVarBinding linear_decay;
    engine::ConVarBinding angle_decay;
    engine::ConVarBinding velocity_decay;
    client::EntityCommandOwner* const& history_owner; //live43475898, getter433952F0
    lagcomp::RotationMath rotation;
    // Actual scalar SSE CRT boundaries, not internal algorithm replacements.
    float (*exponential)(float);
    float (*tangent)(float);
};
class ScreenOverlayPublisher {
    bool standalone_initialized_ = false;
    void* standalone_variable_ = nullptr;

public:
    ScreenOverlayPublisher() noexcept
        : prediction_variable_initialized(standalone_initialized_),
          prediction_variable(standalone_variable_) {}
    ScreenOverlayPublisher(bool& initialized, void*& variable) noexcept
        : prediction_variable_initialized(initialized), prediction_variable(variable) {}
    ScreenOverlayPublisher(const ScreenOverlayPublisher&) = delete;
    ScreenOverlayPublisher& operator=(const ScreenOverlayPublisher&) = delete;
    bool& prediction_variable_initialized; // native43475894
    void*& prediction_variable; // native43475A24
    // Complete433361BA..4333692C. Native receiver is full owner+8; this writes
    // the same ScreenOverlayState read by draw_screen_overlay(43358A6A).
    void publish(ScreenOverlayState&, const esp::ScreenOverlayConfig&,
                 const ScreenOverlayCommandFrame&, void* local, const client::Command&,
                 const ScreenOverlayFields&, const ScreenOverlayUpdateBindings&);
};
} // namespace skeet::hud
