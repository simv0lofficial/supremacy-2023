#pragma once
#include "types.hpp"
#include "skeet/engine/startup_convars.hpp"
#include <variant>

namespace skeet {
struct BunnyHopGuardNode {
    const BunnyHopGuardNode* next;
    const BunnyHopGuardNode* release_marker;
};
// A standalone semantic chain and a real encoded ConVar are distinct layouts.
// Production rereads the encoded pointer, parent+1C and integer+30 only after
// the airborne/enabled gates. No synthetic node snapshot represents a ConVar.
class BunnyHopGuard {
public:
    BunnyHopGuard(const BunnyHopGuardNode& node) noexcept : source_(&node) {}
    BunnyHopGuard(engine::NativeConVarReference variable) noexcept : source_(variable) {}
    bool releases_jump() const noexcept;

private:
    std::variant<const BunnyHopGuardNode*, engine::NativeConVarReference> source_;
};
// Native node chain must terminate in a self-linked node.
void suppress_airborne_jump(bool enabled, std::uint32_t player_flags,
                            const BunnyHopGuardNode& guard, UserCommand& command) noexcept;
void suppress_airborne_jump(bool enabled, std::uint32_t player_flags, const BunnyHopGuard&,
                            UserCommand&) noexcept;

struct BunnyHopTraceResult {
    float fraction;
    bool flag_a;
    bool flag_b;
};
// These are the two actual recovery sites, called only when their respective
// surrounding movement-solver branches are reached, not a replacement solver.
void recover_bunny_hop_low_speed(bool enabled, const BunnyHopTraceResult& trace,
                                 MovementVector velocity, bool required_state, bool blocked,
                                 UserCommand& command, float forced_move = 450.0f) noexcept;
void recover_bunny_hop_trace(bool enabled, const BunnyHopTraceResult& trace, bool blocked,
                             UserCommand& command, float forced_move = 450.0f) noexcept;
} // namespace skeet
