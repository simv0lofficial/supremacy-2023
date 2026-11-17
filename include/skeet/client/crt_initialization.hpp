#pragma once
#include <cstdint>
#include <span>

namespace skeet::client {
class NativeStartupSources;
struct CrtInitializationSlot {
    std::uint32_t operand;
    std::uint32_t identifier;
};

// Explicit CRT configuration for the supplied build, projected from ctx.h.
// Native43481E78..43481E92 establishes traversal order, while the identifiers
// are observed constants. This is not a reconstruction of43493903 or proof
// that its outputs remain invariant in another execution environment.
// NativeStartupRuntime consumes this fixed source configuration. The explicitly
// separate RetainedNativeStartupRuntime still uses the real live accessor.
std::span<const CrtInitializationSlot, 182> crt_initialization_plan() noexcept;
// The supplied build's observed precheck at operand2DC is zero. Keeping this
// explicit avoids pretending to implement a live protected operand accessor.
CrtInitializationSlot crt_initialization_precheck() noexcept;

// Check every nonempty slot before any CRT execution. This validates source
// binding availability only; callback43484F06 requires an explicit nonempty
// binding to its actual implementation/context. This invokes no body.
void preflight_crt_initialization(const NativeStartupSources&);
} //namespace skeet::client
