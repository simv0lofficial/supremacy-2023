#pragma once
#include "skeet/skins/sequences.hpp"
#include "skeet/client/entity_update_dispatch.hpp"
#include "skeet/engine/netvars.hpp"
#include "skeet/engine/offset_storage.hpp"

namespace skeet::skins {
struct SequenceReceiveBindings {
    const Settings& settings;
    void* const& local_player; //43475808
    void* const& engine;
    const engine::NativeOffsetStorage& offsets;
    // The original callbacks are patch+8, not the installed property slot.
    engine::RecvVarProxy sequence, model, parity, weapon_handle;
};
struct SequenceReceivers {
    SequenceRuntime& state;
    SequenceReceiveBindings& bindings;
    std::uint32_t previous_handle = 0xFFFFFFFF; //owner43475B88+320
    std::int32_t previous_sequence = -1; //+324
    void receive_sequence(const engine::RecvProxyData*, void*, void*); //43365510
    void receive_model(const engine::RecvProxyData*, void*, void*); //433B1669
    void receive_parity(const engine::RecvProxyData*, void*, void*); //433FD58A
    void receive_weapon_handle(const engine::RecvProxyData*, void*, void*); //4334B4A0
    // Complete434199D2 for the canonical19knife rows and sequence queue.
    // The weapon-handle queue is deliberately retained by this native reset.
    void reset() noexcept;
};
struct SequenceReceiveDescriptor {
    std::uint32_t operand, class_hash, table_hash, property_hash, native_entry, patch_address;
    engine::RecvVarProxy replacement;
};
std::span<const SequenceReceiveDescriptor, 4> sequence_receive_descriptors() noexcept;
// Native43481F83 publication after construction, before later patch setup.
void bind_sequence_receive_runtime(SequenceReceivers&) noexcept;
// Bind the one native43475B88 receiver. Patch nodes are the actual four nodes
// created by the preserved initialization metadata; retain their link order,
// targets and original after words, and replace only their before callbacks.
void bind_sequence_receive_runtime(SequenceReceivers&,
                                   std::span<client::EntityUpdatePatch* const, 4> patches) noexcept;
void unbind_sequence_receive_runtime(SequenceReceivers&) noexcept;
} //namespace skeet::skins
