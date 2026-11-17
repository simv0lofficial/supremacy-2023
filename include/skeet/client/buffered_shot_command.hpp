#pragma once
#include "command_frame.hpp"
#include "command_pipeline.hpp"
#include "command_transform.hpp"
#include "player_state.hpp"
#include "verified_command.hpp"
#include "skeet/animation/entity_update.hpp"
#include "skeet/network/packet_control.hpp"
#include "skeet/prediction/history.hpp"
#include "skeet/rage/pending_shot_publication.hpp"
#include "skeet/rage/shot_diagnostics.hpp"
#include "skeet/rage/spread.hpp"
#include "skeet/rage/targeting_commit.hpp"
#include <variant>

namespace skeet::rage {
struct BufferedShotRepairBindings;
}
namespace skeet::client {

// One physical owner, native 43475968. The existing diagnostic producer writes
// diagnostics directly; phase6 consumes it alongside the retained frame ring.
struct BufferedShotCommandStorage {
    rage::ShotDiagnosticHistory diagnostics;
    std::array<CommandFrame, 150> frames;
    BufferedShotCommandStorage() noexcept; // true constructor 4347FE52
};
static_assert(offsetof(BufferedShotCommandStorage, frames) == 0x1C0);

struct BufferedShotCommandOffsets {
    std::ptrdiff_t simulation_time; // D5FD2E2C
    std::ptrdiff_t local_field_7f5ed2d0;
    std::ptrdiff_t target_rejection; // 9D920E35
    std::ptrdiff_t target_flags; // E2FBE695
    std::ptrdiff_t target_duck; // 3309C0CF
};
struct BufferedShotCommandConvars {
    void* interpolation_rate; // 4346747C XOR 03B4819C
    void* interpolation_minimum; // 4346A748 XOR 7D9DC86F
    void* interpolation_ratio; // 4346A330 XOR BE64D223
    void* recoil_scale; // 4346794C XOR C429E1BF
    void* command_tick_offset; // 43468528 XOR F75367FE
    void* shotgun_patterns; // 43468520 XOR 394BFF56
};
struct BufferedShotCommandTiming {
    void* command_clock; // actual 43475A60, base tick +4
    const std::int32_t* acknowledged; // nullable 43468F9C
    const std::int32_t* sent; // nullable 434670A8
};
struct BufferedShotCommandBindings {
    const std::uint8_t& enabled; // 43467224
    const std::uint8_t& firing_mode_enabled; // 434684E0
    const std::uint8_t& send_mode; // 43468D7C
    const std::int32_t& choked; // *434757DC
    const std::int32_t& last_outgoing; // *43475B2C
    const std::int32_t& maximum_unlag_ms; // 43468230
    const float& clock_time; // *4346A334
    const float& interval; // *4346A338
    const float& interpolation_floor; // 43469254
    float& prediction_time; // *43467EB8
    CommandChecksum const& checksum; // optional actual engine 43467C48
    Command* const& commands; // genuine CInput +F0
    VerifiedCommand* const& verified; // genuine CInput +F4
    EntityCommandOwner* const& history_owner; //live43475898,4336B29A/433952F0
    void* engine_client; // 4346A514, command getter slot205
    void* entity_list; // 4346A890, handle-to-networkable slot1
    PlayerStateCache& players;
    BufferedShotCommandOffsets offsets;
    BufferedShotCommandConvars convars;
    ScriptCommandEnvironment command_environment;
    const network::LatencySamples& latency;
    const BufferedShotCommandTiming& timing;
    const CommandButtonPolicy& button_policy;
    const CommandTrigonometry& trigonometry;
    const rage::SpreadTables& spread_tables;
    const rage::SpreadRandom& random;
    const rage::ShotgunPattern& shotgun_pattern;
    const RageTrigonometry& spread_trigonometry;
    rage::TargetAngleMath angle_math; // genuine CRT atan2
    float (*arctangent)(float); // genuine CRT __libm_sse2_atanf
    std::array<animation::AnimationAngleSample, 2>& animation_angles; // 434759DC/E8
    const rage::PendingShotPublicationBindings& pending_shots;
    const rage::BufferedShotRepairBindings& repair;
};
class BufferedShotCommandSource {
public:
    virtual ~BufferedShotCommandSource() = default;
    virtual void prepare(BufferedShotCommandStorage&) noexcept = 0;
    virtual void repair(BufferedShotCommandStorage&, CommandFrame&, void* player,
                        std::uint8_t send) = 0;
};
struct BufferedShotCommandOwner {
private:
    std::variant<const BufferedShotCommandBindings*, BufferedShotCommandSource*> source_;

public:
    BufferedShotCommandStorage& state;
    BufferedShotCommandOwner(BufferedShotCommandStorage& state,
                             const BufferedShotCommandBindings& bindings) noexcept
        : source_(&bindings), state(state) {}
    BufferedShotCommandOwner(BufferedShotCommandStorage& state,
                             BufferedShotCommandSource& source) noexcept
        : source_(&source), state(state) {}
    const BufferedShotCommandBindings* decoded() const noexcept {
        if (auto* value = std::get_if<const BufferedShotCommandBindings*>(&source_))
            return *value;
        return nullptr;
    }
    BufferedShotCommandSource* live() const noexcept {
        if (auto* value = std::get_if<BufferedShotCommandSource*>(&source_))
            return *value;
        return nullptr;
    }
};

inline constexpr std::uint32_t buffered_shot_command_vtable = 0x4346F01Cu;
// Complete phase2 entry 433E4CB6. Other phase2 arguments are unused.
void prepare_buffered_shot_commands(BufferedShotCommandOwner&) noexcept;
// Complete phase6 entry 43323284. Current command is not a native input;
// historical commands come from the genuine raw/verified CInput rings.
void repair_buffered_shot_commands(BufferedShotCommandOwner&, CommandFrame&, void* player,
                                   std::uint8_t send_packet);

} // namespace skeet::client
