#pragma once

namespace skeet::engine {
class ClientClassFactoryRuntime;
class HookRuntimeOwner;
class HookDescriptorStorage;
struct UnloadState;
}
namespace skeet::effects {
class VisualReceiveOwner;
}
namespace skeet::skins {
class SkinsRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class PlayerReceiveOwner;

// The final initializer's first complete lifecycle pass calls primary1 on
// the registered57 owners. Its connected pass calls primary5, independently
// of the later4341F57F primary2/primary7 passes.
void initialize_final_feature_callbacks(ClientRuntimeStorage&);

// Devirtualized native final capture order: six ClientClass.create slots,
// fifteen player/layer proxies, four viewmodel proxies, five visual proxies,
// one agent-model proxy. Each node is prepended to canonical43476908. Slots
// keep their originals until the existing entity-update dispatcher exchanges
// them around the actual engine call. No vtable or target is installed here.
void capture_final_startup_patches(ClientRuntimeStorage&, PlayerReceiveOwner&,
                                   effects::VisualReceiveOwner&, skins::SkinsRuntimeOwner&,
                                   engine::ClientClassFactoryRuntime&);

// Actual final phase after the four user-message recipes: publish the client
// feedback fields, capture/link CTEFireBullets into4347644C, install overlay,
// ClientState and the source float decoder, then call all57 primary3 entries.
// CTEFireBullets is exchanged by incoming-tick/update, never left installed.
// The original final initializer then returns AL1 to its outer caller.
void finish_final_engine_startup(ClientRuntimeStorage&, engine::HookRuntimeOwner&);
// Same operation on its three canonical views, also usable by isolated source
// contracts without constructing unrelated animation/render context owners.
void finish_final_engine_startup(ClientRuntimeStorage&, engine::HookDescriptorStorage&,
                                 engine::UnloadState&);
} //namespace skeet::client
