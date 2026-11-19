#pragma once
#include "angle_publication.hpp"
#include "history_activation.hpp"
#include "history_configuration.hpp"
#include "history_animation_publish.hpp"
#include "history_environment.hpp"
#include "history_jobs.hpp"
#include "history_resolver_update.hpp"
#include "history_velocity_capture.hpp"
#include "history_yaw_side.hpp"
#include "update_capture.hpp"
#include "skeet/animation/bone_runtime.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/players/state.hpp"

namespace skeet::ui {
struct SettingsMenuValues;
}
namespace skeet::lagcomp {
struct HistoryRecordingFields {
    std::uint32_t effects_38ca5228;
    std::uint32_t field_043fa575;
    std::uint32_t field_334b460c;
    std::uint32_t weapon_shot_time_0176a381;
    std::uint32_t animation_enabled_8481ea04;
    std::uint32_t angle_interpolation_fac21eaf;
};
struct HistoryRecordingGlobals {
    const std::int32_t& maximum_players; // *43469300
    const std::int32_t& history_clock; // *43475A60
    const float& current_time; // *4346A334
    float& animation_time; // *43467EB8
    float& frame_time; //43467B04+14
    const std::int32_t& animation_frame; // *434678F8
    const ui::SettingsMenuValues& settings_menu;
    const std::uint8_t& shot_angles_active; //43468D7C
    const std::uint8_t& transition_enabled; //4346754C
    std::uint8_t* animation_reset; //optional43467B24
    const RageVector& local_origin; //43475BD4..5BDC
    players::State& adjustments; //canonical4347BE04 records, settings_base()=BE08
};
struct HistoryHypothesisGlobals {
    engine::NativeConVarReference game_type; //43467A8C xorAC9299CC, integer+30
    const std::int32_t& selected_handle; //4346DB1C
    const HotkeyBinding& hotkey;
    const InputBitmap& keys;
    const std::uint32_t& active_mode_mask; //43467030
    const std::uint8_t& physical_override; //43467D70
    const std::uint32_t& input_flags; //4346A8D0
};
struct HistoryTargetRelationships {
    engine::NativeConVarReference game_type, teammates_are_enemies;
    const rage::TargetOffsets& offsets;
};
struct HistoryRecordingBindings {
    client::PlayerStateCache& players;
    RecordPool& records;
    const EntityStateRuntime& entity;
    HistoryRecordingConfiguration& configuration;
    HistoryActivationBindings activation;
    HistoryTargetRelationships targeting;
    HistoryPreludeOffsets prelude;
    HistoryRecordingFields fields;
    HistoryRecordingGlobals globals;
    HistoryHypothesisGlobals hypotheses;
    HistoryYawSideFmod yaw_side_fmod; //actual external _CIfmod ABI adapter
    const animation::BoneRuntime& bones;
    HistoryJobDispatcher& jobs;
    const HistoryEnvironmentEngine& environment;
    const HistoryVelocityCaptureBindings& velocity;
    const HistoryResolverUpdateBindings& resolver;
    const HistoryAnimationPublishBindings& publication;
    const std::int32_t& maximum_unlag_ms; //43468230
};
struct HistoryRecordingScratch {
    //4340C971 automatic locals. No first-use zero/identity direction exists;
    // both regions are retained between players only within that invocation.
    HistoryEnvironmentScratch environment;
    HistoryResolverScratch resolver;
};

class HistoryRecordingRuntime {
public:
    explicit HistoryRecordingRuntime(const HistoryRecordingBindings& bindings) noexcept
        : bindings_(bindings) {}
    // Production entry owns its current native-style automatic workspace.
    void record_players(void* local_player);
    // Complete4340C971 through its real RET4 at43416A07.
    void record_players(void* local_player, HistoryRecordingScratch&);
    // Concrete4336A5B3 worker and the two synchronous record-build branches.
    void build_record(client::PlayerState&, void* player, const float& speed_fraction, LagRecord&,
                      bool deferred);
    void execute_job(const HistoryJobItem&);

private:
    const HistoryRecordingBindings& bindings_;
    bool hypotheses_enabled(void* player) const;
    void build_live_hypotheses(void* player, client::PlayerState&, float speed_fraction,
                               LagRecord&);
    void publish_record(client::PlayerState&, void* player, const CachedHistoryFrame&,
                        bool recording_active, bool enemy, bool forced_yaw, bool layers_changed,
                        std::uint32_t update_flags, float speed_fraction, HistoryJobQueue&);
};
// One process history owner supplies the same canonical globals to deferred
// workers. Bind once during its initialization; no task-local policy or TLS.
void bind_history_recording_runtime(HistoryRecordingRuntime&) noexcept;
} // namespace skeet::lagcomp
