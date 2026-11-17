#pragma once
#include "skeet/config.hpp"
#include "skeet/engine/jobs.hpp"
#include "skeet/engine/native_data_image.hpp"
#include <cstddef>

namespace skeet {
inline constexpr std::size_t settings_owner_size = 0x328;

// Complete sparse43387D1A..43387E2E construction in the actual native owner.
// All ten configuration headers and the hotkey node belong to this storage;
// SettingsPersistenceState subsequently borrows these same fields directly.
// Incoming reserved bytes and fields unwritten by the native body survive.
void construct_settings_owner(void* storage, ConfigRegistries&);

// Source allocation/publication for canonical43478C68. The native ordinary
// constructor and328-byte zeroed allocation are established independently;
// its protected VM caller has not been recovered as a native factory entry.
// Requires an empty owner cell. Allocation failure throws bad_alloc without
// publishing a pointer or altering the configuration/hotkey registries.
std::byte* create_settings_owner(engine::NativeDataImage&, ConfigRegistries&, engine::JobMemory);
} //namespace skeet
