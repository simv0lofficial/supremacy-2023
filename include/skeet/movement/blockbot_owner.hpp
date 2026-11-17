#pragma once
#include "blockbot.hpp"
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/esp/projection.hpp"
#include <vector>

namespace skeet {
class BlockbotOwner;
std::size_t blockbot_owner_size() noexcept;
// Whole43480261: primary, phase2, HUD registration; all table assignments;
// canonical descriptors; sparse state initialization. The incoming storage is
// a native43387448 allocation and primary slot0 frees that header exactly.
// settings must be unregistered; state/configuration/bindings remain shared.
BlockbotOwner& construct_blockbot_owner(void* storage, engine::OwnerRegistryBinding lifecycle,
                                        client::CommandPhaseRegistry& commands,
                                        engine::OwnerRegistryBinding hud, ConfigRegistries&,
                                        BlockbotConfig&, BlockbotState&, const BlockbotBindings&,
                                        const esp::Projection&,
                                        const std::uint32_t& life_state_offset);
} //namespace skeet
