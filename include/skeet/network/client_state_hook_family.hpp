#pragma once
#include "client_state_descriptor.hpp"
#include "skeet/engine/hook_descriptors.hpp"
#include "skeet/engine/hook_descriptor_storage.hpp"
#include <span>

namespace skeet::client {
struct EntityUpdateRuntime;
}
namespace skeet::esp {
struct EffectsSettings;
}
namespace skeet::events {
class EventPacketHookRuntime;
}
namespace skeet::network {
class ClientStateHooksRuntime;
struct TickUpdateRuntime;
struct IncomingUpdateRuntime;
struct SharedEspRuntime;

struct ClientStateHookRuntimes {
    ClientStateHooksRuntime& core;
    TickUpdateRuntime& ticks;
    IncomingUpdateRuntime& incoming;
    const esp::EffectsSettings& effects;
    SharedEspRuntime& shared_esp;
    events::EventPacketHookRuntime& events;
    client::EntityUpdateRuntime& entities;
};
struct ClientStateSourceReplacement {
    ClientStateNativeReplacement native;
    void* source_entry;
};
std::span<const ClientStateSourceReplacement, 13> client_state_source_replacements();

// Complete source counterpart of the single captured13-entry descriptor
// recipe. Every replacement is a concrete recovered source ABI entry. The
// external engine/module imports and shared runtimes keep their real owners.
class ClientStateHookFamily {
public:
    ClientStateHookFamily(engine::HookDescriptorStorage& storage, engine::VtableHookRegistry& hooks,
                          engine::NativeImageEnvironment images,
                          engine::HookDescriptorSystem system, ClientStateHookRuntimes runtimes);
    ClientStateHookFamily(const ClientStateHookFamily&) = delete;
    ClientStateHookFamily& operator=(const ClientStateHookFamily&) = delete;

    // Whole SOURCE recipe: actual RTTI resolution, actual descriptor producer,
    // then all13 source entries. Do not follow it with the preserved raw native
    // replacement loop, which would overwrite these source function pointers.
    void prepare();
    // Bind after an externally completed raw loop when that path is used.
    // The installer only performs its native swap; it does not rewrite slots.
    void bind_replacements();
    engine::VtableHookNode* install(void* actual_receiver);
    engine::VtableHookMetadata& metadata() const noexcept {
        return metadata_;
    }

private:
    engine::VtableHookMetadata& metadata_;
    engine::VtableHookRegistry& hooks_;
    std::byte* const& engine_module_;
    engine::NativeImageEnvironment images_;
    engine::HookDescriptorSystem system_;
    ClientStateHookRuntimes runtimes_;
};
}
