#pragma once
#include "skeet/esp/config.hpp"
#include "skeet/esp/other_config.hpp"
#include "skeet/players/state.hpp"
#include <array>
#include <cstddef>

namespace skeet::effects {
class GlowEntityEngine;

// Engine CGlowObjectManager layout. Pointer fields follow host width in the
// independent verification build; the x86 runtime has the native 0x38 stride.
struct GlowObject {
    std::int32_t next_free = -2;
    void* entity = nullptr;
    std::array<float, 4> rgba{};
    std::array<std::byte, 16> reserved_18{};
    std::uint8_t render_occluded = 0;
    std::array<std::byte, 15> reserved_29{};
};
struct GlowRegistry {
    GlowObject* objects = nullptr;
    std::int32_t allocation_count = 0, grow_size = 0, count = 0;
    GlowObject* elements = nullptr;
    std::int32_t first_free = -1;
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(GlowObject) == 0x38 && offsetof(GlowObject, entity) == 4 &&
              offsetof(GlowObject, rgba) == 8 && offsetof(GlowObject, render_occluded) == 0x28);
static_assert(offsetof(GlowRegistry, count) == 0xC && offsetof(GlowRegistry, first_free) == 0x14);
#endif
struct GlowEngine {
    GlowRegistry* const& registry; // live native 0x43467460 cell
    void* const& registration_function; // live native external engine 0x4346832C cell
    // Non-x86 verification ABI. The runtime's x86 call reproduces the native
    // ECX/EDX, XMM0..3, and five stack arguments inside its explicit adapter.
    using HostRegistration = std::int32_t (*)(GlowRegistry*, void* entity, const float* rgb,
                                              float alpha, bool occluded, bool unoccluded,
                                              std::int32_t split_screen);
};
// Complete 0x43403018 and 0x4340BF38. Native updates temporarily select the
// current slot as the free-list head, then call the engine's actual allocator.
void update_glow(GlowEngine&, void* entity, std::uint32_t index, const std::array<float, 4>& rgba);
void clear_glow(GlowEngine&, void* entity, std::uint32_t index);

class GlowRuntime {
public:
    GlowRuntime(const esp::PlayerEspConfig& players, const esp::WorldEspConfig& world,
                GlowEngine& engine, GlowEntityEngine& entities)
        : players(players), world(world), engine(engine), entities(entities) {}
    const esp::PlayerEspConfig& players;
    const esp::WorldEspConfig& world;
    GlowEngine& engine;
    GlowEntityEngine& entities;
    bool weapons_active = false; // owner +0x6B8 (glow subobject +0x6A0)
    std::int32_t bomb_handle = -1; // owner +0x6F4 (subobject +0x6DC)
    void update(); // complete 0x43385D9A
    void clear_bomb(); // complete 0x433B5B76
};

} // namespace skeet::effects
