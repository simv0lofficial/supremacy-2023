#pragma once
#include "skeet/engine/owner_registry.hpp"
#include "skeet/engine/decoded_offset.hpp"
#include <cstddef>
#include <cstdint>

namespace skeet::engine {
struct NativeOffsetStorage;
}
namespace skeet::client {
struct EntityCommandOwner;
}
namespace skeet::network {
struct PacketDispatchOffsets {
    engine::DecodedOffset zero_based_player; //B6F85C08
    engine::DecodedOffset weapon_identity; //B3832000
    engine::DecodedOffset weapon_ammunition; //CA5EC698
};
PacketDispatchOffsets packet_dispatch_offsets(const engine::NativeOffsetStorage&);

struct PacketDispatchRuntime {
    engine::OwnerRegistry& callbacks; //4347584C/50/54
    void* const& engine; //4346A514
    void* const& entity_list; //4346A890
    client::EntityCommandOwner* const& prediction_owner; //43475898, existing150-slot history
    PacketDispatchOffsets offsets;
};

// Complete433B06F0. The engine supplies a networkable receiver at packet+4;
// the decoded packet is already an engine-owned object. This body dispatches
// the captured registry then suppresses duplicate local predicted-shot updates.
// The original engine endpoint is installed-vtable metadata slot7.
void dispatch_packet_update(PacketDispatchRuntime&, void* receiver, std::int32_t update_type);
void bind_packet_dispatch_runtime(PacketDispatchRuntime&) noexcept;
#if defined(_M_IX86) || defined(__i386__)
void __fastcall packet_update_hook(void* receiver, void*, std::int32_t update_type);
#else
void packet_update_hook(void* receiver, void*, std::int32_t update_type);
#endif
} // namespace skeet::network
