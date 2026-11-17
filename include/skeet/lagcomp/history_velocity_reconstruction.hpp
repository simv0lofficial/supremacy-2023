#pragma once
#include "skeet/lagcomp/records.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/engine/startup_convars.hpp"
#include "skeet/movement/collision.hpp"

namespace skeet::client {
struct PlayerState;
class PlayerStateCache;
}
namespace skeet::lagcomp {

struct HistoryVelocityFields {
    std::uint32_t origin; // A9EE6062
    std::uint32_t duck_amount; // 3309C0CF
    std::uint32_t dormant; // D070684C, byte
    std::uint32_t old_simulation_time; // D3D4230D
    std::uint32_t simulation_time; // D5FD2E2C
    std::uint32_t speed_limit; // 3EB71843
    std::uint32_t player_flags; // E2FBE695
    std::uint32_t walking; // 52899DF3, integer
    std::uint32_t walking_override; // 7090D8DD, byte
    std::uint32_t weapon_handle; // CD812654
};
struct HistoryVelocityCvars {
    // Actual ConVar objects: parent at +1C, encoded float at +2C. The
    // accelerate-using-weapon-speed switch additionally reads encoded +30.
    engine::NativeConVarReference gravity; // global43467A54 xorB882FC15
    engine::NativeConVarReference jump_impulse; // global43467990 xorAF1F2C93
    engine::NativeConVarReference stop_speed; // global43467124 xorC5C9925E
    engine::NativeConVarReference friction; // global4346A820 xorEAAEC541
    engine::NativeConVarReference accelerate; // global43467968 xor1632541C
    engine::NativeConVarReference accelerate_weapon_speed; // global43467E28 xorE80C95F9
};
struct HistoryVelocityReconstructionBindings {
    client::PlayerStateCache& players;
    MovementCollisionEngine& trace;
    void* entity_list;
    const float& interval;
    HistoryVelocityFields fields;
    HistoryVelocityCvars cvars;
    // Actual engine functions4346A364/434678C8 and a truth adapter for the
    // genuine imported strstr774FE670. A nonzero result means substring found.
    // Missing sequence_name is the native empty-string path; classify may
    // itself be absent. The comparison string is global43475AC0+4C.
    engine::MemberFunction<std::int32_t, std::int32_t> classify;
    engine::MemberFunction<const char*, std::int32_t> sequence_name;
    int (*compare_sequence_name)(const char*, const char*);
    const char* jump_sequence_name;
    // Actual77509EE0 double atan2 import. Flight and movement arithmetic,
    // the circle fit and all trace decisions remain concrete source code.
    double (*atan2)(double, double);
    float (*sine)(float); // imported7750C6D0
    float (*cosine)(float); // imported7750A770
    std::uint8_t* trace_flag;
    std::uint32_t& update_flags; // enclosing stack+168, may gain8
};

enum class HistoryVelocityReconstructionExit {
    publish, //4340F1BF ->4340F1CB
    continue_capture //43411342 ->4340F1EC
};

//4340F306..43411342, plus the terminal nonpositive-interval arm at11353..11438.
// The caller owns the enclosing433AC5BD guard and restores it on either exit.
// Inputs are the three newest canonical packet samples and live engine state.
// The result stays in velocity; publish also requests its live-player write.
// As in the enclosing native traversal, player has a valid nonzero networkable
// index and every populated canonical history entry addresses a live record.
HistoryVelocityReconstructionExit
reconstruct_history_velocity(client::PlayerState&, void* player, Vector& velocity,
                             const HistoryVelocityReconstructionBindings&);

} // namespace skeet::lagcomp
