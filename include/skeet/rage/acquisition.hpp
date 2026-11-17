#pragma once
#include "scanning.hpp"
#include "skeet/client/command_frame.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/rage/shot_feedback.hpp"
#include "skeet/engine/jobs.hpp"
#include "skeet/lagcomp/extrapolated_record.hpp"
#include "skeet/lagcomp/history_selection.hpp"
#include "skeet/animation/bone_runtime.hpp"
#include <optional>
#include <vector>

namespace skeet {
struct MovementExtrapolationLiveBindings;
}
namespace skeet::client {
struct CommandBudgetBindings;
}
namespace skeet::ui {
struct SettingsMenuValues;
}
namespace skeet::rage {
class FeedbackOwner;
// The ten native dwords copied from the preamble's SP178 to evaluation+BC.
// Unnamed option bytes are preserved until their actual consumers read them.
struct AcquisitionTargetSettings {
    std::array<std::uint32_t, 10> words{};
    std::uint8_t option(std::size_t native_offset) const noexcept;
    void option(std::size_t native_offset, std::uint8_t value) noexcept;
    ScanPolicy scan_policy() const noexcept;
    void scan_policy(const ScanPolicy&) noexcept;
};
struct AcquisitionEvaluation {
    // One physical native0x1190-byte owner. Its unwritten prefix/EC padding
    // remains incoming storage; record is an alias of native.record.
    lagcomp::EvaluationState native;
    lagcomp::LagRecord& record;
    AcquisitionEvaluation() noexcept : record(native.record) {}
    AcquisitionEvaluation(const AcquisitionEvaluation&) = delete;
    AcquisitionEvaluation& operator=(const AcquisitionEvaluation&) = delete;
    const lagcomp::LagRecord* source_record = nullptr;
    AcquisitionTargetSettings settings;
    std::int32_t requested_tick = -1; // native candidate+B4
    std::uint32_t body_disabler_reasons = 0; // native candidate+B8
    std::uint32_t ballistic_evaluations = 0; // native candidate+E8
    bool extrapolated = false;
    std::int32_t extrapolation_ticks = 0;
    std::uint32_t target_handle = UINT32_MAX;
};
// Publishes only proven prefix fields into the retained native owner. This
// includes every point word and preserves the independent0xEC padding dword.
const lagcomp::EvaluationState& synchronize_evaluation_state(const TargetCandidate&,
                                                             AcquisitionEvaluation&) noexcept;
// Projects an existing/restored complete owner without synthesizing its holes.
void restore_evaluation_state(const lagcomp::EvaluationState&, TargetCandidate&,
                              AcquisitionEvaluation&) noexcept;
struct AcquisitionWorkItem {
    std::array<std::byte, 36> bytes;
};
static_assert(sizeof(AcquisitionWorkItem) == 36);
struct AcquisitionQueue {
    std::array<AcquisitionWorkItem, 80> items;
    std::uint32_t count = 0;
    // Native4340C789 accepts appends only while begin aliases the in-object
    // 80-item buffer. Preserve this distinct state after an external rebind.
    bool inline_storage = true;
    void clear() noexcept {
        count = 0;
    }
};
// The constructor434803BC allocates its80-record block separately from the
// primary owner;43387719 frees that block and clears its pointer. Candidates,
// requests and the embedded work queue keep their existing canonical owners.
class AcquisitionEvaluations {
public:
    AcquisitionEvaluations() noexcept = default;
    ~AcquisitionEvaluations();
    AcquisitionEvaluations(const AcquisitionEvaluations&) = delete;
    AcquisitionEvaluations& operator=(const AcquisitionEvaluations&) = delete;
    void allocate(engine::JobMemory);
    void release() noexcept;
    AcquisitionEvaluation& operator[](std::size_t index) noexcept {
        return data_[index];
    }
    const AcquisitionEvaluation& operator[](std::size_t index) const noexcept {
        return data_[index];
    }
    AcquisitionEvaluation* data() noexcept {
        return data_;
    }
    const AcquisitionEvaluation* data() const noexcept {
        return data_;
    }
    std::size_t size() const noexcept {
        return data_ ? 80u : 0u;
    }
    AcquisitionEvaluation* begin() noexcept {
        return data_;
    }
    AcquisitionEvaluation* end() noexcept {
        return data_ ? data_ + 80 : nullptr;
    }
    const AcquisitionEvaluation* begin() const noexcept {
        return data_;
    }
    const AcquisitionEvaluation* end() const noexcept {
        return data_ ? data_ + 80 : nullptr;
    }

private:
    AcquisitionEvaluation* data_ = nullptr;
    engine::JobMemory memory_{};
};
struct AcquisitionStorage {
    // The canonical contiguous80-candidate projection is consumed directly by
    // rank_completed_target_candidates; its matching record slot has the same index.
    std::array<TargetCandidate, 80> candidates;
    AcquisitionEvaluations evaluations;
    std::array<std::optional<ScanRequest>, 80> requests;
    AcquisitionQueue queued;
};
struct PreambleState;
struct PreambleResult;
struct AcquisitionRecordBindings {
    client::PlayerStateCache& players;
    const void* lag_compensation;
    float interval;
    std::int32_t remembered_target_handle;
    const std::int32_t& body_disablers; //live IntegerConfig.value4346A8D0; low byte read per record
    FeedbackOwner* const& feedback_owner; //canonical43475A7C slot; resolved after health getter
    const lagcomp::HistorySelectionBindings& history;
    const lagcomp::ExtrapolatedRecordBindings& extrapolation;
    ScanContext& scanner;
    const ui::SettingsMenuValues& settings_menu; // live Multithreading434683FC
    const animation::BoneRuntime& bone_runtime;
    const MovementExtrapolationLiveBindings& live_movement;
    const client::CommandBudgetBindings& live_budget; // live433A13ED global references
    // Buffered-shot433C155F clears the safe-neck bit after record policy.
    bool clear_safe_neck = false;
};
// Shared5182A..51C32 /5430E..5473E. Every record load, capture/extrapolation,
// body/safe policy and scanner invocation is a concrete recovered source call.
void prepare_and_submit_target_records(PreambleState&, PreambleResult&, AcquisitionStorage&,
                                       void* target, client::PlayerState&,
                                       std::span<const std::int32_t> ticks,
                                       const AcquisitionTargetSettings&,
                                       std::uint32_t descriptor_flags,
                                       const AcquisitionRecordBindings&);
// Concrete native enqueue/direct branch43354663..43354703 and its duplicated
// single-target block. Queued worker43406EE0 observes the canonical previous
// target mask before deciding whether to execute the full scanner.
void submit_acquisition_scan(AcquisitionStorage&, PreambleState&, ScanContext&, void* target_player,
                             std::uint32_t slot, std::uint32_t descriptor_flags,
                             const ui::SettingsMenuValues&);
// Complete nonempty-job drain4335473E..43354793, using the recovered36-byte
// dispatcher and this module's actual source worker. The live checkbox is
// read again here; a disabled drain preserves the queued logical end.
void dispatch_acquisition_scans(AcquisitionStorage&, engine::JobDispatcher&,
                                const ui::SettingsMenuValues&);
// Final per-player write43354793..433547C9 after the job drain.
void finish_target_acquisition(PreambleState&, AcquisitionStorage&, engine::JobDispatcher&,
                               client::PlayerStateCache&, std::int32_t maximum_clients,
                               const ui::SettingsMenuValues&);
struct PendingTargetDamage {
    std::int32_t entity_index;
    std::int32_t damage;
};
// Complete4341F16E. False leaves both output fields untouched.
bool pending_target_damage(const ShotFeedbackState*, PendingTargetDamage&) noexcept;
// 4335435C..433543BF, after the actual entity health getter at slot122.
std::int32_t scan_target_health(std::int32_t health, std::uint32_t entity_index,
                                const PendingTargetDamage*) noexcept;
// Complete433318B5. reference_damage is deliberately retained.
void reset_scan_point(TargetPoint&) noexcept;
// 43383EAE..43383F11 after the canonical player cache lookup and ConVar gate.
bool target_history_is_usable(const client::NativePlayerStateLayout&, const lagcomp::RecordRing&,
                              const client::CommandFrame&, std::uint32_t entity_index,
                              float interval) noexcept;
} // namespace skeet::rage
