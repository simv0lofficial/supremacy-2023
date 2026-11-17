#pragma once
#include "skeet/client/skin_runtime_factories.hpp"
#include "skeet/engine/build_skin_bindings.hpp"
#include "skeet/skins/native_schema.hpp"
#include "skeet/skins/native_sequence_update.hpp"
#include "skeet/skins/sequence_owner.hpp"
#include <memory>

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
}
namespace skeet::engine {
class BuildSkinReceivePatches;
}
namespace skeet::skins {
// Stable production lifetime shared by skin feature constructors and RootMenu.
// Construction creates source adapters and live views only. The existing four
// skin factories and43481F6A retain their actual registration/startup order;
// RootMenu's catalog initializer performs its genuine lazy ItemSystem call.
// Keep this context alive through feature destruction and proxy restoration.
class SkinsRuntimeOwner {
public:
    SkinsRuntimeOwner(client::ClientRuntimeStorage&, client::ConfigurationRuntimeOwner&,
                      engine::BuildSkinBindings);
    ~SkinsRuntimeOwner();
    SkinsRuntimeOwner(const SkinsRuntimeOwner&) = delete;
    SkinsRuntimeOwner& operator=(const SkinsRuntimeOwner&) = delete;
    SkinsRuntimeOwner(SkinsRuntimeOwner&&) = delete;
    SkinsRuntimeOwner& operator=(SkinsRuntimeOwner&&) = delete;
    NativeCatalog& catalog() noexcept;
    WeaponRefresh& refresh() noexcept;
    Settings& settings() noexcept;
    NativeEntityCosmetics& cosmetics() noexcept;
    ModelInfo& models() noexcept;
    StringTables& string_tables() noexcept;
    client::SkinRuntimeFactoryBindings& factory_bindings() noexcept;
    SequenceOwnerBindings& sequence_bindings() noexcept;
    SequenceRuntime& sequence_state() noexcept;
    SequenceReceivers& sequence_receivers() noexcept;
    NativeSequenceRuntime& sequence_runtime() noexcept;
    AgentModelReceiver& agent_receiver() noexcept;
    // The actual patch nodes/targets/order are supplied by their source
    // producer. This binds only the existing source proxy callbacks/originals.
    void bind_receive_patches(std::span<client::EntityUpdatePatch* const, 4>,
                              client::EntityUpdatePatch&) noexcept;
    void bind_receive_patches(const engine::BuildSkinReceivePatches&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::skins
