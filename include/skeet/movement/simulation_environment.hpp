#pragma once
#include "simulation.hpp"
#include "skeet/engine/startup_convars.hpp"

namespace skeet {
namespace client {
struct Command;
}

// Resolved entity-property offsets. Each native stage reads the current
// binding; engine getters and collision calls may change the live objects.
struct MovementEnvironmentFields {
    std::uint32_t speed_limit_3eb71843;
    std::uint32_t ducking_6b1a59b5;
    std::uint32_t field_a15c3105;
    std::uint32_t weapon_handle_cd812654;
    std::uint32_t player_flags_e2fbe695;
    std::uint32_t duck_amount_3309c0cf;
    std::uint32_t duck_speed_fb7e3404;
};

struct MovementEnvironmentConvars {
    engine::ConVarBinding accelerate_weapon_speed; //43467E28 xorE80C95F9
    engine::ConVarBinding jump_speed_cap; //43467FA0 xor4B52A9BF
    engine::ConVarBinding stamina_land; //43468F4C xorB0CF25E8; presence only
    engine::ConVarBinding stop_speed; //43467124 xorC5C9925E
    engine::ConVarBinding friction; //4346A820 xorEAAEC541
    engine::ConVarBinding accelerate; //43467968 xor1632541C
};

struct MovementEnvironmentBindings {
    MovementEnvironmentFields fields;
    MovementEnvironmentConvars convars;
    void* entity_list; // actual entity-list interface, slot4
    const float* interval; // live4346A338
    std::uint8_t* engine_trace_flag; // live434683D0
};

//433D7624..433D76F2: player450 getter precedes the speed-limit property read.
// Other fields are zero; this result belongs only to the command stage.
MovementPhysics capture_movement_command_physics(void* player, const MovementEnvironmentBindings&);

//433DA894..433DAA31. Call only inside the positive additional-speed branch.
// The raw duck byte follows the command-button short circuit. Weapon lookup
// and virtual6E8/738/6EC calls retain their native guards and call order.
// Duck/walk decisions survive mutations made by those engine methods.
struct MovementGroundWeaponPhysics {
    MovementPhysics physics;
    bool duck;
    bool walk;
};
MovementGroundWeaponPhysics
capture_movement_ground_weapon_physics(void* player, const UserCommand&,
                                       std::uint32_t simulation_player_flags,
                                       const MovementEnvironmentBindings&);

// Direct native reads, called by the corresponding stage after its gates.
bool movement_player_duck_slowdown(void* player, const MovementEnvironmentBindings&) noexcept;
bool movement_jump_speed_cap(const MovementEnvironmentBindings&) noexcept;
bool movement_landing_stamina_present(const MovementEnvironmentBindings&) noexcept;

// Live433D7606. The trace guard encloses command getters, collision, duck,
// weapon getters and finish. Every simulated tick repeats its native reads.
void simulate_live_movement_tick(MovementSimulation&, MovementCollisionEngine&,
                                 const MovementEnvironmentBindings&, void* player,
                                 const UserCommand&, MovementAngles);
// Native command pointer contract: movement is copied before the max-speed
// getter; view angles are read later at the actual ground/air angle-vector call.
void simulate_live_movement_tick(MovementSimulation&, MovementCollisionEngine&,
                                 const MovementEnvironmentBindings&, void* player,
                                 const client::Command&);

} // namespace skeet
