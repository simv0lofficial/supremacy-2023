#pragma once
#include "skeet/network/packet_control.hpp"
#include <memory>

namespace skeet::network {
struct PacketCommandOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class MovementHudRuntimeOwner;
class TargetRefreshRuntimeOwner;
class NativeStartupSources;
struct EngineRuntimeBindings;
struct PacketRuntimeFactoryBindings;
// Genuine external vstdlib boundaries. The explicit overload permits authored
// standalone spies; production resolves the already loaded vstdlib exports.
struct PacketRuntimeImports {
    using Seed = void(__cdecl*)(std::int32_t);
    using Integer = std::int32_t(__cdecl*)(std::int32_t, std::int32_t);
    Seed seed;
    Integer integer;
};
PacketRuntimeImports native_packet_runtime_imports(ClientRuntimeStorage&,
                                                   const EngineRuntimeBindings&);
class PacketRuntimeOwner final {
public:
    PacketRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                       ConfigurationRuntimeOwner&, HistoryRuntimeOwner&, MovementHudRuntimeOwner&,
                       TargetRefreshRuntimeOwner&);
    // Requires an already published genuine/authored4346A7CC boundary. This
    // explicit surface performs no module discovery and executes no query.
    PacketRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                       ConfigurationRuntimeOwner&, HistoryRuntimeOwner&, MovementHudRuntimeOwner&,
                       TargetRefreshRuntimeOwner&, PacketRuntimeImports);
    ~PacketRuntimeOwner();
    PacketRuntimeOwner(const PacketRuntimeOwner&) = delete;
    PacketRuntimeOwner& operator=(const PacketRuntimeOwner&) = delete;
    PacketRuntimeOwner(PacketRuntimeOwner&&) = delete;
    PacketRuntimeOwner& operator=(PacketRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    network::PacketCommandOwner& command() noexcept;
    network::FakeLagState& state() noexcept;
    PacketRuntimeFactoryBindings& factory_bindings() noexcept;
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client
