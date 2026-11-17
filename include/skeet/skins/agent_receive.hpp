#pragma once
#include "skeet/engine/netvars.hpp"
#include <cstdint>

namespace skeet::client {
struct EntityUpdatePatch;
}
namespace skeet::skins {
struct AgentModelCaptureGlobals {
    std::uint32_t& original_handle; //4346D648
    std::int32_t& original_model; //4346D650
};
struct AgentModelReceiveBindings {
    void* const& engine_client; //4346A514
    engine::RecvVarProxy& original; //43476A94, patch+8
};
class AgentModelReceiver {
public:
    AgentModelReceiver(AgentModelCaptureGlobals globals, AgentModelReceiveBindings bindings)
        : globals_(globals), bindings_(bindings) {}
    void receive(const engine::RecvProxyData*, void* entity, void* output); //4335CCD8
    const AgentModelCaptureGlobals& capture_globals() const noexcept {
        return globals_;
    }
    const AgentModelReceiveBindings& receive_bindings() const noexcept {
        return bindings_;
    }

private:
    friend void bind_agent_model_receiver(AgentModelReceiver&, client::EntityUpdatePatch&) noexcept;
    AgentModelCaptureGlobals globals_;
    AgentModelReceiveBindings bindings_;
};
struct AgentModelReceiveDescriptor {
    std::uint32_t class_hash, table_hash, property_hash, native_entry, patch_address;
    engine::RecvVarProxy replacement;
};
const AgentModelReceiveDescriptor& agent_model_receive_descriptor() noexcept;
// The preserved initialization metadata supplies this actual patch node.
// Retain its target, after callback and link order; replace only before.
void bind_agent_model_receiver(AgentModelReceiver&, client::EntityUpdatePatch&) noexcept;
} //namespace skeet::skins
