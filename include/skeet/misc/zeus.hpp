#pragma once
#include "skeet/client/command_frame.hpp"
#include "skeet/misc/config.hpp"
#include "skeet/rage/ballistics.hpp"
#include <cstddef>

namespace skeet::misc {

struct ZeusState {
    // Complete owner +20, filled by primary initialization 434824A7.
    // The actual shared RAGE Enabled descriptor is hash 23060EC5.
    const std::uint8_t* rage_enabled = nullptr;
};

struct ZeusBindings {
    BallisticEngine& ballistics;
    void* const* trace_filter_vtable; // Genuine external table 43475B90, slots 0..4.
    std::ptrdiff_t weapon_definition; // 4346908C / 5C9F0FBB, uint16.
    std::ptrdiff_t target_immunity; // 43467DD0 / AF649DAC, byte.
};

inline constexpr std::uint32_t zeus_constructor = 0x43485D4Cu;
inline constexpr std::uint32_t zeus_command_body = 0x433A12BDu;
inline constexpr std::uint32_t zeus_command_vtable = 0x434701A0u;
inline constexpr std::uint32_t zeus_command_phase = 4;

// Complete 434824A7. The shared boolean descriptor must already be registered.
void bind_zeus_configuration(ZeusState&, ConfigLink& descriptors) noexcept;

// Complete phase4 body 433A12BD; its receiver is the owner +8 interface.
// Direction is the three live floats at frame +174, not command Euler angles.
// Uses the concrete 433EB28B weapon query and 43364CEC direction bullet loop,
// with penetration disabled and the genuine base filter 4346DCEC semantics.
void process_zeus(ZeusState&, const ZeusConfig&, const ZeusBindings&, const client::CommandFrame&,
                  void* player, client::Command&);

} // namespace skeet::misc
