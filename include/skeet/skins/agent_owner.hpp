#pragma once
#include "skeet/skins/agent_receive.hpp"
#include "skeet/skins/native_agents.hpp"
#include "skeet/engine/owner_registry.hpp"

namespace skeet::skins {
struct AgentOwnerBindings {
    Settings& settings;
    NativeCatalog& catalog;
    // The runtime derives both capture references and its engine-client
    // reference from this receiver. No second original-model/handle store.
    AgentModelReceiver& receiver;
    void* const& entity_list;
    NativeAgentBindings::SupportModels const& support_models;
    const engine::NativeOffsetGlobals& offsets;
    ModelInfo& models;
    StringTables& tables;
    network::DataUpdateObserverRegistry& observers;
};
// Whole 4347DAAA constructor (4347DAB7 is its secondary registration), table
// 4346F358. The primary owner registers before the runtime's actual 4337EFD9
// observer append; both precede the four agent descriptor registrations.
// Its Settings links are fresh; production uses initialize_shared rather than
// the separate full Settings::initialize semantic/fixture initializer.
// The receiver and its 4346D648/4346D650 cells outlive the owner's destruction and
// remain available until the real receive-property patch is restored.
std::size_t agent_owner_size() noexcept;
// Requires valid storage returned by allocate_agent_owner_storage.
void construct_agent_owner(void* native_header_storage, engine::OwnerRegistryBinding,
                           ConfigRegistries&, AgentOwnerBindings);
// The 43387448 allocation used by 4348574E, including its native alignment
// header. Returns null on allocation failure; the original factory's failure
// continuation at 43485761 is unresolved and is not implemented here.
void* allocate_agent_owner_storage(engine::JobMemory);
// Wire the existing 4335CCD8 source callback through its preserved metadata
// patch. Neither its target nor its position in the patch list is replaced.
void bind_agent_owner_receiver(void* owner, client::EntityUpdatePatch&) noexcept;
} //namespace skeet::skins
