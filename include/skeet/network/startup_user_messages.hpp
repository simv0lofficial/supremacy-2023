#pragma once
#include "skeet/network/client_state_hook_bindings.hpp"
#include "skeet/engine/native_data_image.hpp"
#include <array>
#include <span>

namespace skeet::network {
struct StartupUserMessageDescriptor {
    std::int32_t identifier;
    std::uint32_t native_callback;
    std::uint32_t patch_address;
    void(__cdecl* callback)(const void*);
};
std::span<const StartupUserMessageDescriptor, 4> startup_user_message_descriptors();
std::span<const std::uintptr_t, 5> startup_user_message_wrapper_table();
using StartupUserMessageTargets = std::array<ClientStateScopedPatch**, 4>;

// Actual VClient018 virtual38 identifies its SDK dispatch map. The supported
// client PE and instruction operands are verified before its native tree and
// first binding-vector slots are read. No captured SDK pointer is retained.
StartupUserMessageTargets discover_startup_user_message_targets(engine::NativeDataImage&);

// Devirtualized final F58..F84 recipes, in64/6/69/25 order. The nodes are only
// prepared here. ClientState virtual32 exchanges each SDK target temporarily.
void prepare_startup_user_message_patches(engine::NativeDataImage&,
                                          const StartupUserMessageTargets&);
void prepare_startup_user_message_patches(engine::NativeDataImage&);
} //namespace skeet::network
