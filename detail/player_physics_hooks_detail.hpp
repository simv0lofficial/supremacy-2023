#pragma once
#include "skeet/client/player_physics_hooks.hpp"
#include "skeet/engine/interface_call.hpp"
#include <cstring>

namespace skeet::client::player_physics_detail {
template <class T> T field(const void* object, std::ptrdiff_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
template <class T> void store(void* object, std::ptrdiff_t offset, const T& value) noexcept {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof value);
}
template <class Result, class... Arguments>
Result original(void* receiver, std::size_t slot, Arguments... arguments) {
    auto** installed = *static_cast<void***>(receiver);
    const auto* metadata = static_cast<const engine::VtableHookMetadata*>(installed[-2]);
    return reinterpret_cast<engine::MemberFunction<Result, Arguments...>>(
        metadata->original_table[slot])(receiver, arguments...);
}
void bind_velocity_continuation(const std::uint32_t&) noexcept;
} //namespace skeet::client::player_physics_detail
