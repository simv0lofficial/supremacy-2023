#pragma once
#include "config.hpp"
#include "skeet/client/command_frame.hpp"
#include "skeet/rage/record_candidates.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/lagcomp/extrapolated_record.hpp"
#include <variant>

namespace skeet::misc {
struct KnifeMeleeBindings;
struct KnifeAimBindings;
struct KnifeRuntimeBindings {
    const std::uint8_t* rage_enabled; // 43467224, read again after selection
    const std::int32_t* maximum_clients; // *43469300
    const rage::TargetBindings& targeting;
    std::ptrdiff_t armor; // decoded 47A6F50C
    const rage::RecordCandidateBindings& candidates;
    const lagcomp::RecordSelectionContext& records;
    const lagcomp::ExtrapolatedRecordBindings& extrapolation;
    const KnifeMeleeBindings& melee;
    const KnifeAimBindings& aim;
    const animation::BoneRuntime& bone_runtime;
    const MovementExtrapolationLiveBindings& live_movement;
    const client::CommandBudgetBindings& budget;
};

// Complete4341D871. Eligibility uses43358962, movement type8 is rejected,
// and the strict250000 squared-distance ceiling retains the first tie.
void* nearest_knife_target(const RageVector& origin, const rage::TargetBindings&,
                           std::int32_t maximum_clients);

// Complete433257DA gameplay owner. Uses the canonical command/frame/cache,
// real melee traces and lag-record transactions. The native +8 secondary
// owner is represented by KnifeConfig rather than raw configuration offsets.
void process_knife_command(const KnifeConfig&, client::CommandFrame&, client::Command&,
                           void* player, std::uint8_t& send_packet, std::uint32_t& scheduler_flags,
                           const KnifeRuntimeBindings&);
// Native entry gates precede physical frame construction. Production samples
// no player/model/record until these exact config/weapon/command gates pass.
bool knife_command_enabled(const KnifeConfig&, const client::CommandFrame&, const client::Command&,
                           const std::uint8_t* rage_enabled) noexcept;
void process_enabled_knife_command(const KnifeConfig&, client::CommandFrame&, client::Command&,
                                   void* player, std::uint8_t& send_packet,
                                   std::uint32_t& scheduler_flags, const KnifeRuntimeBindings&);

class KnifeCommandSource {
public:
    virtual ~KnifeCommandSource() = default;
    virtual void process(const KnifeConfig&, client::CommandFrame&, client::Command&, void*,
                         std::uint8_t&, std::uint32_t&) = 0;
};
// The explicit standalone arm borrows authored complete frame views. The
// production arm retains only its source owner and forms views per call.
class KnifeCommandBindings {
public:
    KnifeCommandBindings(const KnifeRuntimeBindings& bindings) noexcept : source_(&bindings) {}
    KnifeCommandBindings(KnifeCommandSource& source) noexcept : source_(&source) {}
    void process(const KnifeConfig&, client::CommandFrame&, client::Command&, void*, std::uint8_t&,
                 std::uint32_t&) const;

private:
    std::variant<const KnifeRuntimeBindings*, KnifeCommandSource*> source_;
};
} // namespace skeet::misc
