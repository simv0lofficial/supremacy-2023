#pragma once
#include "settings.hpp"
#include "spread.hpp"
#include "firing.hpp"
#include "skeet/client/command_frame.hpp"
#include "skeet/anti_aim/targets.hpp"
#include "skeet/lagcomp/entity_state.hpp"
#include <functional>
#include <variant>

namespace skeet::client {
class TargetRefreshRuntimeOwner;
}

namespace skeet::rage {
struct AcquisitionStorage;
// The exact ten copied dwords at stack+178..19F / evaluation+BC..E3:
// mode, regular/safe masks, scale, multipoint, damage, chance, FOV cosine,
// then DC safe, DD body, DE delay, DF body-on-peek, E0 damage override,
// E1 additional hypotheses, E2/E3 zero.
using PreambleScanTemplate = std::array<std::uint32_t, 10>;

struct PreambleAnimationSnapshot {
    std::uint32_t sequence;
    std::array<std::byte, 0x10> fields_04;
    lagcomp::AnimationState animation;
    float absolute_yaw;
    std::array<std::byte, 0x354> fields_1e8;
};
static_assert(sizeof(PreambleAnimationSnapshot) == 0x53C);
static_assert(offsetof(PreambleAnimationSnapshot, animation) == 0x14);
static_assert(offsetof(PreambleAnimationSnapshot, absolute_yaw) == 0x1E4);
class PreambleAnimationHistory {
    const std::byte* records_;

public:
    PreambleAnimationHistory(const std::array<PreambleAnimationSnapshot, 150>* records) noexcept
        : records_(reinterpret_cast<const std::byte*>(records)) {}
    explicit PreambleAnimationHistory(const std::byte* records) noexcept : records_(records) {}
    const std::byte* at(std::uint32_t sequence) const noexcept {
        return records_ ? records_ + (sequence % 150u) * sizeof(PreambleAnimationSnapshot)
                        : nullptr;
    }
};

// Persistent fields of the RAGE gameplay subobject, whose address is the
// configuration owner+8. Acquisition and firing use these same references.
struct PreambleState {
    std::uint32_t flags = 0; // +170
    std::uint32_t weapon_flags = 0; // +174; separate retained owner
    std::array<std::uint32_t, 3> completed{}; // +1AC
    std::uint32_t evaluation_count = 0; // +1B8; prepared candidates
    std::array<std::uint32_t, 2> target_mask{}; // +19C
    std::array<std::uint32_t, 2> previous_targets{}; // +1A4
    SpreadCache spread; // primary+D14, compensated+1114
    std::uint32_t frame_first = 0; // +1514
    std::int32_t frame_count = 0; // +1518
    std::array<client::CommandFrame, 16> frames; // +151C
};
struct PreambleOffsets {
    std::ptrdiff_t weapon_reload_guard; // decoder B8969256
    std::ptrdiff_t ammunition; // CA5EC698
    std::ptrdiff_t reserve_ammunition; // 49C770CA
    std::ptrdiff_t weapon_identifier; // 5C9F0FBB
    std::ptrdiff_t weapon_next_attack; // F71CE331
    std::ptrdiff_t player_next_attack; // DE530A8B
    std::ptrdiff_t taser_timing; // D2563274
    std::ptrdiff_t local_guard; // D720AEA6
    std::ptrdiff_t animation_layers; // B538BBA5
    std::ptrdiff_t pose_parameters; // 1CAAF0A8
};
using PreambleTargetRefresh =
    std::variant<std::reference_wrapper<const anti_aim::TargetRefreshContext>,
                 std::reference_wrapper<const client::TargetRefreshRuntimeOwner>>;
struct PreambleBindings {
    const GlobalSettings& settings;
    const AimbotSettings& weapon_settings;
    InputBitmap input;
    std::uint32_t& scheduler_flags; // native434759C0
    const void* restricted_convar; // resolved43469028
    const void* const* game_rules; // 4346A3A4
    PreambleOffsets offsets;
    std::span<client::Command, 150> command_ring; // actual CInput+F0
    PreambleAnimationHistory animation_history;
    float& prediction_time; // pointed43467EB8
    float interval;
    bool firing_mode_enabled, send_mode;
    bool command_sequence_gate; // native43468D7C
    std::int32_t current_sequence; // *43475B2C
    std::int32_t& selected_marker; // native434759C4
    std::int32_t
        double_tap_chance; // retained ABI snapshot; reads use settings.double_tap_hit_chance
    bool shotgun_patterns; // terminal43468520 ConVar
    const SpreadTables& spread_tables;
    const SpreadRandom& random;
    const ShotgunPattern& pattern;
    RageTrigonometry trigonometry;
    anti_aim::TargetState& yaw_target;
    PreambleTargetRefresh target_refresh;
    // Genuine optional engine434688C8, identical to the animation runtime.
    lagcomp::SetAbsoluteVector set_absolute_angles;
};
enum class PreambleContinuation {
    no_weapon, //43358507
    inactive, //43357733
    ammunition, //433576F0
    single_target, //4335034F
    multiple_targets, //43351C37
};
struct PreambleResult {
    PreambleContinuation continuation = PreambleContinuation::no_weapon;
    client::CommandFrame* frame = nullptr;
    client::Command* command = nullptr;
    PreambleScanTemplate scan_template{};
    FiringBranchState firing{};
    bool original_firing_bit = false; // stack+D8 bit0, frame+0 mask400
    std::int32_t tick_base = 0; // stack+E4, frame+C8
    std::uint32_t command_hash = 0; // stack+11C
    float previous_prediction_time = 0; // observed stack+124, not restored at exit
    bool replaced_frame = false;
    std::uint32_t prediction_clock_marker = 0x7F7FFFFFU; // stack+120
    bool point_scale_is_automatic = false; // owner+C4==24
    float maximum_point_scale = 0.9f; // stack+22C
    std::uint32_t first_entity = 1;
};
// Complete true shared setup4334F55F..43350349. The selected-frame command,
// shared yaw-target refresh, animation copy and spread preparation are concrete
// source operations. No internal prelude phase is supplied as a callback.
// Native433E0A59 resets the acquisition owner's queued-work logical end without
// releasing its allocation or deleting evaluations.
PreambleResult prepare_rage_command(PreambleState&, client::CommandFrame& original,
                                    client::Command&, void* local_player, AcquisitionStorage&,
                                    const PreambleBindings&);
// Shared identical settings block also in action owner43332281..43332630.
PreambleScanTemplate prepare_rage_scan_template(const PreambleState&, const client::CommandFrame&,
                                                const PreambleBindings&);
} // namespace skeet::rage
