#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace skeet::network {
inline constexpr std::uint32_t client_state_descriptor_identity = 0x43475a3c;
inline constexpr std::uint32_t client_state_module_identity = 0x43468d20;
inline constexpr std::uint32_t client_state_rtti_hash = 0x747ca973; //.?AVCClientState@@
inline constexpr std::uint32_t client_state_rtti_base_hash = 0;
inline constexpr std::uint32_t client_state_replacement_stream = 0x818;
inline constexpr std::size_t client_state_event_slot = 34;
struct ClientStateNativeReplacement {
    std::uint32_t slot, native_entry, entry_operand;
};
// Captured contexts2060..2085, in actual replacement-loop order.
inline constexpr std::array<ClientStateNativeReplacement, 13> client_state_native_replacements{
    {{2, 0x433c04cc, 0x81c},
     {5, 0x4341efed, 0x824},
     {6, 0x433cf7e4, 0x82c},
     {12, 0x433982bb, 0x834},
     {13, 0x43359ca0, 0x83c},
     {15, 0x433fba1c, 0x844},
     {24, 0x433fd2c4, 0x84c},
     {25, 0x433cfd1a, 0x854},
     {32, 0x433f2918, 0x85c},
     {34, 0x433ae68a, 0x864},
     {37, 0x4333ca14, 0x86c},
     {47, 0x43320766, 0x874},
     {64, 0x433205a0, 0x87c}}};
}
