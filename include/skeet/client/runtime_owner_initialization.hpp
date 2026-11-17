#pragma once
#include <cstdint>
#include <span>

namespace skeet::client {
class NativeStartupSources;

// Source identifiers in the constructor order observed in the working client.
// These integers are never converted into executable addresses.
std::span<const std::uint32_t, 57> runtime_owner_initializers() noexcept;
void preflight_runtime_owner_initialization(const NativeStartupSources&);
void initialize_runtime_owners(const NativeStartupSources& sources);
}
