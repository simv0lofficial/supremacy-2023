#pragma once
#include "scanning.hpp"

namespace skeet::ui {
struct SettingsMenuValues;
}
namespace skeet::client {
struct CommandFrame;
}
namespace skeet::lagcomp {
struct HistorySelectionBindings;
}
namespace skeet::rage {
struct PreambleState;
struct PreambleBindings;
struct AcquisitionEvaluation;
struct FinalTargetContextBindings {
    ScanContext& scan;
    const std::uint32_t* current_bone_counter;
    const ui::SettingsMenuValues& settings_menu; // live Multithreading434683FC
    engine::JobDispatcher* jobs;
    const lagcomp::HistorySelectionBindings* history = nullptr;
    const PreambleBindings* preamble = nullptr;
    bool validates_records() const noexcept;
    bool precise_damage() const noexcept;
    bool full_seed_hitchance() const noexcept;
    FinalTargetContextBindings with_scan(ScanContext& replacement) const noexcept {
        return {replacement, current_bone_counter, settings_menu, jobs, history, preamble};
    }
};
// Complete engine-input owner4341B4C2. Call lazily for each attempted point,
// including promotions: bypass precedes entity lookup, then the bullet/basis,
// teammate policy/rays, studio reads, record transaction and sample workers.
// Failure updates the same player cache and context_flags used by acquisition.
bool evaluate_final_target(client::CommandFrame&, PreambleState&, const TargetCandidate&,
                           const AcquisitionEvaluation&, std::uint32_t& context_flags,
                           const FinalTargetContextBindings&, float& chance);
} // namespace skeet::rage
