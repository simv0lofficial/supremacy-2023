#pragma once
#include "accuracy.hpp"
#include "historical_bones.hpp"
#include "historical_bones_live.hpp"
#include "manual_motion.hpp"
#include "target_selection.hpp"

namespace skeet::legit {
struct RuntimeSettings {
    const Settings& legit;
    const BooleanConfig& rage_enabled;
    const HotkeyBinding& rage_key;
};
struct RuntimeNetvars {
    std::ptrdiff_t player_shots_8cfc633a;
    std::ptrdiff_t player_effects_38ca5228;
    std::ptrdiff_t player_byte_043fa575, player_byte_334b460c;
    std::ptrdiff_t player_punch_1bae1168;
    std::ptrdiff_t player_punch_velocity_48442721;
    std::ptrdiff_t player_next_attack_de530a8b;
    std::ptrdiff_t weapon_next_attack_f71ce331;
    std::ptrdiff_t ammunition_ca5ec698, reloading_b8969256;
    std::ptrdiff_t blindness_1bc6d160;
};
struct RuntimeTimes {
    const float* interval; // *4346A338
    const float* command_time; // *4346A334
    const float* world_time; // *43467EB8
    const float* frame_delta; // [43467B04]+20
    const float* setup_bones_time; // [43467B04]+10
    const float* interpolation; // 43476928
    const std::int32_t* clock_tick; // *43468D4C
    const std::int32_t* history_command_tick; // [43475A60]+4, velocity age clock
    const std::int32_t* maximum_clients; //*43469300
};
struct RuntimeBindings {
    RuntimeNetvars offsets;
    RuntimeTimes times;
    AccuracyEnvironment accuracy;
    HistoricalBoneOffsets historical_offsets;
    std::span<Matrix3x4, 19> target_bones;
    std::span<Matrix3x4, 256> bone_scratch;
    RecoilDecayBindings recoil_decay;
    RandomBindings random;
    InputBitmap input;
    const client::CommandAngleBindings& view_angles;
    std::uint32_t& scheduler_flags;
};
// Complete phase0 owner4332CDEA and4339FB19. Runs before the master player's
// rejection gates, obtains engine-client slot18 and advances both view slots.
void capture_legit_view(RuntimeState&, void* engine_client);
// Complete phase4 owner43399A38. All recovered decisions call concrete LEGIT,
// shared history and ballistic implementations; only genuine engine/CRT/RNG
// boundaries are supplied in bindings. Native receiver is owner+8; the source
// RuntimeState expresses the owner's proven fields independently of x86 ABI.
void process_legit_command(RuntimeState&, client::CommandFrame&, void* local_player,
                           client::Command&, const RuntimeSettings&, RuntimeBindings&);
} // namespace skeet::legit
