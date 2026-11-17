#pragma once
#include "command_frame.hpp"
#include "command_pipeline.hpp"
#include "command_prediction_cache.hpp"
#include "command_recapture.hpp"
#include "command_weapon_postprocess.hpp"
#include "player_state.hpp"
#include "skeet/anti_aim/targets.hpp"
#include "skeet/network/packet_control.hpp"
#include <vector>

namespace skeet::client {
struct CommandCaptureOffsets {
    CommandMouseOffsets mouse;
    std::ptrdiff_t rules_guard; // D000A932
    std::ptrdiff_t tick_base; // 3D10B94C
    std::ptrdiff_t vector_48442721;
    std::ptrdiff_t weapon_field_d2563274;
    std::ptrdiff_t team; // 8B5586F6
};
struct CommandCaptureConvars {
    engine::NativeConVarReference target_selector; //43468EA4^FCD2DA7A,+30
    engine::NativeConVarReference optional_angle_override; //43468254^CF54AEB3,+30;nullable
    engine::NativeConVarReference angle_mode; //43467A8C^AC9299CC,+30
    engine::NativeConVarReference angle_secondary; //4346A42C^672DFE69,+30
    engine::NativeConVarReference command_tick_offset; //43468528^F75367FE,+30
    engine::NativeConVarReference interpolation_rate; //4346747C^03B4819C,+2C
    engine::NativeConVarReference interpolation_minimum; //4346A748^7D9DC86F,+2C
    engine::NativeConVarReference interpolation_ratio; //4346A330^BE64D223,+2C
};
struct CommandTargetValidation {
    anti_aim::TargetState& target; // shared 4346DB14..4346DB38
    PlayerStateCache& players; // concrete 433B8A2B cache
    void* const& entity_list; // live4346A890
    const std::int32_t* sequence; // *434678F8
    void* const* clock_owner; // 4346A3A4; nullable
    std::ptrdiff_t clock_offset; // 9F8E7F71
    std::ptrdiff_t rejection_byte; // 9D920E35
    std::ptrdiff_t network_origin; // A9EE6062
    void* const* dormant_owner; // 43468FF0; nullable
    std::ptrdiff_t dormant_present; // 9571D5F5, indexed by handle low word
    std::ptrdiff_t dormant_valid; // D266B537, indexed by handle low word
};
// Complete 433ED5A8. The valid player-handle contract excludes its deliberate
// invalid-instruction branch for a zero low-word entity index.
bool command_target_valid(const CommandTargetValidation&);

struct CommandCaptureContext {
    const CommandRecaptureContext& physical;
    ScriptCommandEnvironment angle_environment;
    CommandCaptureOffsets offsets;
    CommandCaptureConvars convars;
    void* engine_client; // 4346A514; GetViewAngles slot18
    const HotkeyBinding& movement_key; //live RAGE duck_peek_assist, D23ED2A7
    InputBitmap input;
    const std::uint8_t* angle_override; // 434670C0
    const std::int32_t* maximum_angle; // 4346709C
    const float* interpolation_floor; // 43469254
    const network::LatencySamples& latency;
    const SentCommandTiming& timing;
    const CommandTargetValidation& target_validation;
};
struct CommandCapturedState {
    CommandBoneBackup bones;
    float planar_speed; // master stack +29C
    float speed; // master stack +2A0
};
// Complete initial capture 433D355E..433D415B. saved_angles is the master's
// pre-phase2 +318 snapshot. bone_storage is the retained-capacity native
// 43478CC8 vector; its storage must survive prediction and later restoration.
// global_flags refers to 434759C0, separately from frame.flags.
CommandCapturedState capture_initial_command_frame(CommandFrame&, Command&,
                                                   std::uint8_t& send_packet,
                                                   std::uint32_t& global_flags, float sample_time,
                                                   const std::array<float, 3>& saved_angles,
                                                   std::vector<Matrix3x4>& bone_storage,
                                                   const CommandCaptureContext&);
} // namespace skeet::client
