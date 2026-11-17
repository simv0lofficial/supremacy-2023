#pragma once
#include "skeet/legit/config.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/engine/startup_convars.hpp"

namespace skeet::legit {
using Vector = std::array<float, 3>;
struct ProfileState {
    std::int32_t profile = 0; // owner + 0x804
    std::int32_t shots = 0; // 0x810
    std::uint32_t previous_buttons = 0; // 0x814
    std::uint8_t enabled = 0; // 0x818
    float speed = 0, speed_scale_fov = 0; // 0x81C, 0x820
    float maximum_lock_time = 0; // 0x824
    bool limit_lock_time = false; // 0x828
    float reaction_time = 0, maximum_fov = 0; // 0x82C, 0x830
    float recoil_pitch = 0, recoil_yaw = 0; // 0x834, 0x838
    std::uint8_t flags = 0, secondary_flags = 0; // 0x83C, 0x83D
    std::uint32_t aim_hitgroups = 0; // 0x840
    float first_shot_range = 0; // 0x844
    std::int32_t minimum_hit_chance = 0; // 0x848
    float trigger_reaction = 0, burst_time = 0; // 0x84C, 0x850
    std::int32_t minimum_damage = 0; // 0x854
    std::uint32_t trigger_hitgroups = 0; // 0x858
    std::int32_t accuracy_boost = 0; // 0x85C
    std::int32_t accuracy_range = 0; // 0x860
    std::array<Vector, 3> recoil_history{}; // 0x898..0x8B8
    float burst_until = 0; // 0x8BC
};
// 43399B2F..43399E4D: complete per-weapon settings projection. Valid engine
// weapon-profile values are nonnegative signed bytes. Values >= 9 read profile
// zero while retaining the previously selected state.profile, as in x86.
bool select_weapon_profile(const Settings&, ProfileState&, std::int32_t weapon_profile,
                           std::uint32_t command_buttons);
// Complete433EA238. A missing LEGIT owner admits recoil compensation. Valid
// engine categories are0..8; the aim hotkey is evaluated only when enabled.
bool recoil_compensation_enabled(const Settings*, std::uint32_t category, InputBitmap);
// Complete native 4340C87E, shared by the disabled-profile and reset paths.
void clear_recoil_history(ProfileState&) noexcept;

struct FirstShotFrame {
    std::uint32_t prediction_buttons;
    Vector eye_position;
    Vector forward;
    std::int32_t maximum_clients;
};
// Complete 433EE27B. Entity enumeration and origin/bounds access use actual
// engine virtuals; candidate rejection reuses the recovered 43358962 chain.
void apply_first_shot_delay(const Settings&, ProfileState&, const FirstShotFrame&,
                            const rage::TargetBindings&, InputBitmap,
                            std::uint32_t& command_buttons);

struct RecoilDecayBindings {
    // Parent-chained native ConVars: parent +0x1C, encoded float +0x2C.
    engine::ConVarBinding linear_decay;
    engine::ConVarBinding angle_decay;
    engine::ConVarBinding velocity_decay;
    float interval;
    // Native msvcrt!__libm_sse2_expf boundary, called twice per update.
    float (*exponential)(float);
};
// Complete 4336B421: exponential and linear angle decay, with the native
// two half-step velocity integrations and in-place velocity update.
void decay_recoil(Vector& velocity, Vector& angle, const RecoilDecayBindings&);
struct AimStepInput {
    float pitch_delta, yaw_delta;
    float target_fov; // native caller stack +0x38
    float target_distance; // native caller stack +0x140
    float time_scale; // owner +0x8D8
    float curve; // owner +0x8EC
    std::int32_t command_number;
};
struct RandomBindings {
    void* context;
    void (*seed)(void*, std::int32_t);
    float (*uniform)(void*, float minimum, float maximum);
};
// 4339BB12..4339BD66, the complete angular step calculation after the
// target/manual-movement gates. Keeps FOV scaling, range scaling, seeded native
// random variation and the final length cap in their original operation order.
Vector calculate_aim_step(const ProfileState&, const AimStepInput&, RandomBindings);
struct TriggerFrame {
    std::uint32_t context_flags; // native weapon context +0
    std::uint32_t weapon_id; // +0xD8
    float time; // native global 43467EB8
    float blind_amount;
};
struct TriggerCommand {
    std::uint32_t buttons;
    std::int32_t tick;
};
struct TriggerHit {
    float damage;
    std::uint32_t hitgroup;
    void* entity;
};
struct TargetTrack;
// Complete 4331FBDB. target_times contains the first float of each native
// 0x28-byte record at owner +0x910, indexed by (networkable index & 63).
bool trigger_hit_allowed(const ProfileState&, const TriggerHit&,
                         std::span<const float, 64> target_times);
// Whole4331FBDB live-owner form: the age is read from the original strided
// target record after the genuine networkable index call has returned.
bool trigger_hit_allowed_live(const ProfileState&, const TriggerHit&, const TargetTrack* targets);
// 4339BFBD..4339C074: activation, pending-burst and blind-player gates before
// tracing. True enters the native ballistic/hitchance path.
bool begin_trigger(const Settings&, ProfileState&, const TriggerFrame&, InputBitmap,
                   TriggerCommand&);
// 4339C408..4339C491: apply the actual trace result/tick, then advance the
// burst timer. RandomFloat is called only on a successful non-burst result.
void finish_trigger(ProfileState&, const TriggerFrame&, bool hit, std::int32_t selected_tick,
                    TriggerCommand&, RandomBindings);
// Whole4339C408..4339C491 continuation. Frame flags are read after tracing,
// and the world clock is read after RandomFloat when random burst is selected.
void finish_trigger_live(ProfileState&, const std::uint32_t* context_flags, const float* time,
                         bool hit, std::int32_t selected_tick, TriggerCommand&, RandomBindings);
} // namespace skeet::legit
