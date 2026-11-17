#pragma once
#include "player_engine.hpp"
#include "sound_overlay.hpp"

namespace skeet::client {
class ClientRuntimeStorage;
struct EngineRuntimeBindings;
}
namespace skeet::engine {
class SceneQueryRuntimeOwner;
}
namespace skeet::esp {
// Canonical live fields for the EnginePlayers constructed inside the complete
// ESP owner. This supplies no identity cache, decoded entity snapshot or second
// ESP state. Encoded ConVar words retain their actual native XOR view.
PlayerEngineBindings native_player_engine_bindings(client::ClientRuntimeStorage&,
                                                   const client::EngineRuntimeBindings&);
// Shares the actual trace adapter and nullable debug/temp-entity interfaces.
// Sound state and SoundOverlayContext still belong to EspOwnerRuntime itself.
SoundOverlayBindings native_sound_overlay_bindings(client::ClientRuntimeStorage&,
                                                   const client::EngineRuntimeBindings&,
                                                   engine::SceneQueryRuntimeOwner&);
} // namespace skeet::esp
