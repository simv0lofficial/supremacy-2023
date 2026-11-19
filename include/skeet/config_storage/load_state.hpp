#pragma once
#include <cstdint>

namespace skeet {

// Native 43475A98 is one shared byte. Category loading sets it to one for
// its value passes. Scoped tab loads also keep it set during widget refresh.
inline bool& config_loading_flag() noexcept {
    static bool loading = false;
    return loading;
}

// The unload bindings retain the native byte-pointer interface to that same
// state; no second value or synchronization step is introduced.
inline const std::uint8_t* config_loading_byte() noexcept {
    static_assert(sizeof(bool) == sizeof(std::uint8_t));
    return reinterpret_cast<const std::uint8_t*>(&config_loading_flag());
}

} // namespace skeet
