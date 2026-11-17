#pragma once
#include "history.hpp"
#include "skeet/resolver_runtime/environment_probe.hpp"

namespace skeet::client {
struct PlayerState;
}
namespace skeet::lagcomp {
#if defined(_M_IX86) || defined(__i386__)
using HistoryEnvironmentAtan2 = double(__vectorcall*)(double, double);
#else
using HistoryEnvironmentAtan2 = double (*)(double, double);
#endif
struct HistoryEnvironmentInput {
    RageVector local_origin; // native cached origin43475BD4, before the +64 eye offset
    std::int32_t simulation_ticks;
    // Raw x86 address of the canonical native owner+5D4 ring. It is read as
    // float bits on the original both-blocked path, not dereferenced there.
    std::uint32_t native_history_address;
    // Read only if the final yaw-change gate succeeds, after every trace.
    const float& current_time; // *4346A334
};
struct HistoryEnvironmentScratch {
    // Native caller stack+2EC..2F4, retained between players in4340C971.
    // The native enclosing function owns these automatic words and does not
    // initialize them before first use; the opaque load retains actual bytes.
    RageVector direction;
};
struct HistoryEnvironmentEngine {
    const resolver_runtime::EnvironmentEngine& probe;
    std::uint32_t origin_offset; // decoded A9EE6062 target-origin field
    // Native77509EE0 __libm_sse2_atan2: y in XMM0, x in XMM1, result in XMM0.
    HistoryEnvironmentAtan2 atan2;
};
struct HistoryEnvironmentUpdate {
    bool selected = false;
    bool timestamp_updated = false;
};

// Complete4340D89F..4340E58B; stops before E58B's resolver-index store.
// Caller has entered the preceding resolver-detection branch. Reads the live
// target origin, executes the concrete world/hull and
// penetration probe, publishes owner+F0, and conditionally stamps owner+F4.
// The shared PlayerState owns the only history ring and all published fields.
HistoryEnvironmentUpdate update_history_environment(client::PlayerState&, const void* target,
                                                    HistoryEnvironmentScratch&,
                                                    const HistoryEnvironmentInput&,
                                                    const HistoryEnvironmentEngine&);
} // namespace skeet::lagcomp
