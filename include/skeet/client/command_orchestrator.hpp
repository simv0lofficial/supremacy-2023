#pragma once
#include "command_angle_retention.hpp"
#include "command_capture.hpp"
#include "command_packet_angle_guard.hpp"
#include "command_packet_transition.hpp"
#include "command_weapon_postprocess.hpp"
#include "command_owner_dispatch.hpp"
#include "command_eye_adjustment.hpp"
#include "restricted_player_command.hpp"
#include "command_empty_tick.hpp"
#include "skeet/anti_aim/config.hpp"

namespace skeet::effects {
class FreeLookOwner;
}
namespace skeet::client {
struct PredictionHistoryOwner;
class CommandCaptureRuntimeOwner;
class TargetRefreshRuntimeOwner;
struct CommandPlayerGates {
    std::array<std::ptrdiff_t, 2> rejection_bytes; //9D920E35,CDE290D2
    std::ptrdiff_t restricted_state; //2F4E97C4, int32
    std::ptrdiff_t movement_type; //7501CB91, byte
    std::ptrdiff_t player_flags; //E2FBE695, uint32
    std::ptrdiff_t tick_base; //3D10B94C, int32
    std::ptrdiff_t active_weapon; //CD812654, handle
};
bool command_player_rejected(const void*, const CommandPlayerGates&) noexcept;
bool command_player_restricted(const void*, const CommandPlayerGates&) noexcept;

struct CommandPacketCatchupBindings {
    void* const& engine_client; //4346A514
    void* const& network_state; //43468E44; double clock at+108
    const float& interval; //*4346A338
};
//433D300D..433D3075. Only the channel clock is an engine boundary.
void catch_up_command_packet(std::uint8_t&, const std::uint8_t& packet_context,
                             const CommandPacketCatchupBindings&);

struct CommandBudgetBindings {
    void** const& game_rules; //4346A3A4
    std::ptrdiff_t rule_bf567619, rule_dda99ab5, rule_b7ea8fda;
    const std::int32_t& configured; //43469274
};
//Complete433A13ED as read by the master before ordered phase9 reductions.
std::int32_t query_command_budget(const CommandBudgetBindings&) noexcept;

//433D2F15..2F63/433D308D..30A1: forward the original CreateMove result's
//angles directly. The empty-tick path additionally updates prediction slot13.
void forward_original_command_angles(void* const& engine_client, void* const& prediction, Command&,
                                     bool empty_tick);

//433D524E..433D5368. Phase6 follows this concrete movement/angle/shot block.
void finalize_command_movement(CommandFrame&, Command&, std::uint8_t& send_packet,
                               const CommandButtonPolicy&, const CommandTrigonometry&,
                               const ScriptCommandEnvironment&, std::int32_t last_outgoing,
                               std::int32_t& last_shot);

//433D5868..433D59DB, after all active time scopes have been restored.
void finish_command_transaction(Command&, std::uint8_t& engine_send_packet,
                                std::uint8_t requested_packet,
                                const CommandPacketTransitionBindings&,
                                std::uint32_t& scheduler_flags, const CommandEntityBindings&,
                                const CommandEntitySnapshot&);

struct CommandMasterGlobals {
    void* const& engine_client;
    void* const& entity_list;
    void* const& prediction;
    void* const& engine_globals;
    float* const& interval;
    float* const& current_time;
    std::int32_t* const& choked;
    std::int32_t* const& last_outgoing;
    std::uint32_t& scheduler_flags;
    std::int32_t& last_shot;
};
struct CommandSentTimingBindings {
    void* const& command_clock; //43475A60, base tick+4
    std::int32_t* const& acknowledged; //43468F9C
    std::int32_t* const& sent; //434670A8
};
struct CommandAcceptedBindings {
    CommandMasterGlobals globals;
    const CommandPlayerGates& player_gates;
    const CommandEntityBindings& entities;
    const CommandPacketCatchupBindings& catchup;
    effects::FreeLookOwner* const& free_look; //same live43475A64 owner used by view capture
    const HotkeyBinding& free_look_key;
    const CommandAngleBindings& view_angles;
    const CommandBudgetBindings& budget;
    //Each phase forms a fresh span, as the native owner rereads begin/end.
    const CommandPhaseRegistry& order;
    const CommandCaptureRuntimeOwner& capture;
    const CommandEyeAdjustmentSourceContext& eye;
    std::vector<Matrix3x4>& bone_storage;
    const CommandMouseOffsets& mouse;
    void* const& input; //live4346A894
    engine::NativeConVarReference copy_gate_convar; //43468E94^2DC3CC20
    std::uint8_t* const& transform_override;
    std::span<const std::uint32_t, 150> inverted_commands;
    const anti_aim::OtherGlobalSettings& anti_aim_other;
    const CommandTrigonometry& trigonometry;
    const ScriptCommandEnvironment& environment;
    const prediction::CommandBindings& prediction_bindings;
    const prediction::CommandPlayerOffsets& prediction_player_offsets;
    prediction::SeedGenerator seed_generator;
    const CommandPacketAngleBindings& packet_angles;
    SentCommandHistoryView sent_history;
    CommandSentTimingBindings sent_timing;
    const CommandWeaponPostprocessBindings& postprocess;
    const CommandPacketTransitionBindings& packet_transition;
    PredictionHistoryOwner* const& restricted_owner; //live434759B4, shared view resolved at call
    const RestrictedPlayerCommandBindings& restricted;
    const TargetRefreshRuntimeOwner& sentinel_refresh;
};
//433D2FFA..433D59DB, after phase0 and the two active-player rejection gates.
//All phases1..9 use the supplied actual registration order. Each custom owner
//must have a concrete source binding; unsupported owners are rejected up front.
//The caller retains frame and bone capacity; send_packet addresses the outer
//saved EBX word, including its adjacent context byte. No payload callback runs.
std::uint8_t run_accepted_command_pipeline(const CommandAcceptedBindings&, CommandFrame&,
                                           void* player, float sample_time, Command&,
                                           std::uint8_t& send_packet,
                                           std::uint8_t original_create_move_result);
struct RejectedPlayerUpdateContext;
struct CommandCreateMoveBindings {
    const CommandEntryBindings& entry;
    const CommandAcceptedBindings& accepted;
    const CommandEmptyTickRegistry& empty_tick_order;
    const RejectedPlayerUpdateContext& rejected_update;
};
//Complete native433D2E40 framing and branches, followed by the concrete
//accepted pipeline. Both independently registered owner orders are explicit.
std::uint8_t process_create_move_command(const CommandCreateMoveBindings&, CommandFrame&,
                                         void* player, float sample_time, Command&,
                                         std::uint8_t& send_packet);
struct CommandCreateMoveRuntime {
    const CommandCreateMoveBindings& bindings;
    std::uint8_t process(void* player, std::uint8_t* send_packet, float sample_time, Command*);
};
void bind_create_move_runtime(CommandCreateMoveRuntime&) noexcept;
void unbind_create_move_runtime(CommandCreateMoveRuntime&) noexcept;
#if defined(_M_IX86) || defined(__i386__)
std::uint8_t __fastcall create_move_command_hook(void* player, std::uint8_t* send_packet,
                                                 float sample_time, Command*);
//Actual player virtual289 replacement4334F53D. EDX at engine entry is not
//the packet pointer; this wrapper passes the saved EBX word to the inner master
//and pops the modified word back into EBX before returning RET8.
std::uint8_t __fastcall player_create_move_hook(void* player, void* register_edx, float sample_time,
                                                Command*);
#else
std::uint8_t create_move_command_hook(void* player, std::uint8_t* send_packet, float sample_time,
                                      Command*);
#endif
} //namespace skeet::client
