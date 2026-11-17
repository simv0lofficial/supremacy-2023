#pragma once
#include "command_owner.hpp"
#include "shot_diagnostics.hpp"

namespace skeet::rage {
class RuntimeOwner;
struct BufferedShotRepairBindings {
    std::variant<CommandOwner*, RuntimeOwner*> source;
    BufferedShotRepairBindings(CommandOwner* fixture) noexcept : source(fixture) {}
    BufferedShotRepairBindings(RuntimeOwner& live) noexcept : source(&live) {}
};
// Native43402BA2 supplies sixteen in-object pointer slots. Candidate grouping
// appends only while the begin pointer still aliases that inline storage.
struct BufferedShotRepairScratch {
    TargetBatch candidates;
    bool inline_storage = true;
    BallisticQuery historical_bullet;
};
// Complete433C155F. The stored frame is the original ECX, local player EDX,
// followed by copied command, six-entry diagnostic record and inline scratch.
// Returns the canonical full0x1190 evaluation in owner.acquisition, whose
// lifetime extends through the caller's pending-shot update. Flags+170 mask1
// is set for reconstruction and cleared on every ordinary exit.
const lagcomp::EvaluationState*
repair_buffered_shot(client::CommandFrame& stored, void* local_player,
                     client::Command& copied_command, const ShotDiagnosticEntry&,
                     BufferedShotRepairScratch&, const BufferedShotRepairBindings&);
} // namespace skeet::rage
