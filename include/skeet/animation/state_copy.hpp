#pragma once
#include <cstddef>

namespace skeet::animation {

inline constexpr std::size_t copied_animation_state_size = 0x348;

// Complete433F526C. Only the five native32-bit vector-header words at+13C
// are initialized; the remaining animation state bytes are untouched.
void initialize_animation_state_vector(void* animation_state) noexcept;

// Complete433CF41B ->4331E2CD twice. allocator is the actual43467204 engine
// interface. Owned native32-bit storage is freed through its virtual slot5;
// a negative grow-size keeps external storage and its allocation count.
void release_animation_state_vector(void* animation_state, void* allocator);

} // namespace skeet::animation
