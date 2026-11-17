#pragma once
#include <cstdint>
#include <span>

namespace skeet::client {
class ClientRuntimeStorage;
class PlayerRuntimeOwner;
class EntityCommandRuntimeOwner;
struct EngineRuntimeBindings;
}
namespace skeet::engine {
class HookRuntimeOwner;
class HookDescriptorStorage;
struct VtableHookRegistry;
enum class ClientClassFactoryKind {
    player,
    ragdoll,
    incendiary,
    molotov,
    viewmodel,
    inferno
};
using ClientClassCreate = void* (*)(std::int32_t, std::int32_t);
struct ClientClassFactoryDescriptor {
    std::uint32_t operand;
    const char* class_name;
    std::uint32_t class_hash, native_entry, patch_address;
    ClientClassCreate replacement;
};
// The six native480..4BC recipes. The final startup scheduler captures each
// actual ClientClass.create in patch.after and installs this source entry.
std::span<const ClientClassFactoryDescriptor, 6> client_class_factory_descriptors() noexcept;

// Creation callbacks borrow the same cache, record pool, prediction owner,
// descriptors and installation registry as the existing feature lifecycles.
// This context does not create an entity or install a ClientClass patch.
class ClientClassFactoryRuntime final {
public:
    ClientClassFactoryRuntime(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                              client::PlayerRuntimeOwner&, client::EntityCommandRuntimeOwner&,
                              HookRuntimeOwner&);
    ClientClassFactoryRuntime(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                              client::PlayerRuntimeOwner&, client::EntityCommandRuntimeOwner&,
                              VtableHookRegistry&, HookDescriptorStorage&);
    ~ClientClassFactoryRuntime();
    ClientClassFactoryRuntime(const ClientClassFactoryRuntime&) = delete;
    ClientClassFactoryRuntime& operator=(const ClientClassFactoryRuntime&) = delete;
    void* create(ClientClassFactoryKind, std::int32_t entity_index, std::int32_t serial);

private:
    void install(void*, std::uint32_t descriptor);
    void initialize_player(void* networkable, std::int32_t entity_index);
    void restore_local_movement_flags(void* player);
    client::ClientRuntimeStorage& storage_;
    client::PlayerRuntimeOwner& players_;
    client::EntityCommandRuntimeOwner& entity_;
    VtableHookRegistry& registry_;
    HookDescriptorStorage& descriptors_;
};
// Publish only after the actual family contexts are bound. Keep this runtime
// alive through restoration of the six ClientClass.create patch targets.
void bind_client_class_factory_runtime(ClientClassFactoryRuntime&);
} //namespace skeet::engine
