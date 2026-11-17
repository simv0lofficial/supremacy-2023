#pragma once
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/esp/grenade_prediction.hpp"
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace skeet::effects {

// Actual owner +94..98 entries. Fields not read by 43380299 are preserved.
struct ProjectileRibbonEntry {
    std::uint32_t handle;
    std::uint32_t reserved_04[4];
    std::int32_t state; // only 0 and 1 are eligible
    esp::GrenadeKind kind;
    std::uint8_t rejected;
    std::uint8_t reserved_1b;
    float creation_time;
    std::uint32_t reserved_20;
};
static_assert(sizeof(ProjectileRibbonEntry) == 0x24);
static_assert(offsetof(ProjectileRibbonEntry, kind) == 0x18);
static_assert(offsetof(ProjectileRibbonEntry, creation_time) == 0x1c);

struct ProjectileRibbonOffsets {
    explicit ProjectileRibbonOffsets(const engine::NativeOffsetGlobals&);
    engine::DecodedOffset inactive; // 01E80986, int32
    engine::DecodedOffset effects; // 38CA5228, uint32; bit 5 suppresses prediction
    engine::DecodedOffset thrower_handle; // DC743513, handle
    engine::DecodedOffset velocity; // 36375C23, Vector3
    engine::DecodedOffset previous_velocity; // 434E9E95, Vector3
    engine::DecodedOffset simulation_time; // D5FD2E2C, float
    engine::DecodedOffset origin; // A9EE6062, Vector3
    engine::DecodedOffset collision_group; // E4E2D1A7, int32
    engine::DecodedOffset projectile_flags; // 7418868F, low uint16
};
struct ProjectileRibbonBindings {
    void* const& entity_list; // live4346A890, entity handle lookup at virtual slot4
    void* const& material_system; // live4346A4E0
    void* const& key_values_system; // actual vstdlib!KeyValuesSystem() result cell
    ProjectileRibbonOffsets offsets;
};
struct ProjectileRibbonFrame {
    bool suppressed; // 43468E58
    bool active; // 43467CD4
    std::int32_t current_tick; // *43468D4C
    std::uint32_t color; // 43468F60, native RGBA bytes
};
// Production bindings borrow the actual global storage. Tick and color are
// read at their native sites after preceding entity/simulation engine calls.
struct ProjectileRibbonLiveFrame {
    const std::uint8_t& suppressed; // 43468E58
    const std::uint8_t& active; // 43467CD4
    const std::int32_t& current_tick; // *43468D4C
    const std::uint32_t& color; // 43468F60
};

// These are the owner's actual decoded strings, not replacement asset names.
struct ProjectileRibbonMaterialNames {
    std::string render_mode; // owner +240, FindVarFast token +220
    std::string texture; // owner +250
    std::string base_texture; // owner +260, FindVarFast token +224
    std::string shader; // owner +270
};

class ProjectileRibbonOwner {
public:
    ProjectileRibbonMaterialNames material_names;
    void* material = nullptr; // owner +A4, lazily created
    std::uint32_t render_mode_token = 0, base_texture_token = 0;

    // Complete 43380299. Uses the real entity/material/trace boundaries;
    // prediction, filtering, endpoint rules and beam geometry remain source.
    void draw(const ProjectileRibbonFrame&, std::span<const ProjectileRibbonEntry>,
              const ProjectileRibbonBindings&, const esp::GrenadeEnvironment&,
              esp::GrenadeTraceProvider);
    // The production owner passes its canonical warning vector and the shared
    // full-owner+D0 world-point scratch used by warning refresh directly.
    void draw(const ProjectileRibbonLiveFrame&, std::span<const esp::GrenadeWarningRecord>,
              const ProjectileRibbonBindings&, const esp::GrenadeEnvironmentBindings&,
              esp::GrenadeTraceProvider, std::vector<esp::Vector3>& world_path);
    void release_material(); // complete4336843F: virtual13, then clear +A4
    std::span<const esp::Vector3> path() const noexcept {
        return path_;
    }

private:
    template <class Frame, class Entry, class Environment>
    void draw_impl(const Frame&, std::span<const Entry>, const ProjectileRibbonBindings&,
                   const Environment&, esp::GrenadeTraceProvider,
                   std::vector<esp::Vector3>& world_path);
    void prepare_material(const ProjectileRibbonBindings&);
    std::vector<esp::Vector3> path_; // fixture storage; production borrows full-owner+D0
};

} // namespace skeet::effects
