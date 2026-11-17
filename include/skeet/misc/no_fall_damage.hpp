#pragma once
#include "skeet/config.hpp"
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/client/commands.hpp"
#include "skeet/esp/projection.hpp"
#include "skeet/hud/composite_runtime.hpp"
#include "skeet/movement/simulation_capture.hpp"
#include "skeet/movement/simulation_environment.hpp"

namespace skeet::misc {

struct NoFallDamageConfig {
    BooleanConfig enabled;
    ColorConfig color;
};
void initialize_no_fall_damage_config(ConfigRegistries&, NoFallDamageConfig&) noexcept;

struct NoFallDamageOffsets {
    std::ptrdiff_t movement_type; //7501CB91, byte
    std::ptrdiff_t flags; //E2FBE695, uint32
    std::ptrdiff_t velocity; //36375C23, MovementVector
};
struct NoFallDamageBindings {
    NoFallDamageOffsets offsets;
    void* engine; //actual4346A514, view-angle virtual18
    const void* gravity; //43467A54 xorB882FC15, terminal encoded ConVar
    const void* jump_impulse; //43467990 xorAF1F2C93
    const void* air_accelerate; //434674E4 xorDCEAEAC8
    const float& interval; //*4346A338
    MovementCollisionEngine& collision;
    const MovementCaptureInput& simulation_capture;
    const MovementEnvironmentBindings& movement_environment;
    const esp::Projection& projection;
};

class NoFallDamageRuntime {
public:
    NoFallDamageRuntime(NoFallDamageConfig& settings, NoFallDamageBindings engine)
        : config(settings), bindings(engine) {}
    NoFallDamageConfig& config;
    NoFallDamageBindings bindings;
    // Native complete+3C. Set by lifecycle slot1, hashF2EB89E9.
    const std::uint8_t* air_strafe_enabled = nullptr;
    void bind_configuration(ConfigLink&) noexcept; //434902BA
    void phase2(void* player, client::Command&); //43401BDB..43402018
    void draw(ui::RenderQueue&, void* player); //433953A3..43396CA1
};

struct NoFallDamageHudHandler : hud::HudFrameCallback {
    NoFallDamageRuntime& runtime;
    explicit NoFallDamageHudHandler(NoFallDamageRuntime&);
};

inline constexpr std::uint32_t no_fall_damage_constructor = 0x4348DCB4u;
inline constexpr std::uint32_t no_fall_damage_command_vtable = 0x43472998u;
inline constexpr std::uint32_t no_fall_damage_hud_vtable = 0x43472460u;

// Native order: lifecycle, command interface, HUD interface, then settings.
// Storage comes from the restored aligned native allocator; slot0 frees it.
std::size_t no_fall_damage_owner_size() noexcept;
NoFallDamageRuntime& construct_no_fall_damage(void* storage, engine::OwnerRegistryBinding owners,
                                              client::CommandPhaseRegistry& command_owners,
                                              engine::OwnerRegistryBinding hud_owners,
                                              ConfigRegistries&, NoFallDamageConfig&,
                                              NoFallDamageBindings);

} //namespace skeet::misc
