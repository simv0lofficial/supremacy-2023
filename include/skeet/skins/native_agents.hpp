#pragma once
#include "skeet/skins/config.hpp"
#include "skeet/skins/native_catalog.hpp"
#include "skeet/skins/model_resources.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/engine/offset_storage.hpp"
#include "skeet/network/data_update_receiver.hpp"
#include <cstddef>
#include <cstdint>

namespace skeet::network {
class DataUpdateObserverRegistry;
}
namespace skeet::skins {
struct NativeAgentBindings {
    using SupportModels = engine::MemberFunction<const std::byte*>;
    void* const& engine_client; // live 4346A514
    void* const& entity_list; // live 4346A890
    const std::int32_t& original_model; // 4346D650
    const std::uint32_t& original_handle; // 4346D648
    SupportModels const& support_models; // actual 4346D64C, ECX=model path
    const engine::NativeOffsetGlobals& offsets;
};
// Concrete43472978 slot1 body registered at secondary receiver+8 by
// 4347DAB7/4337EFD9 in the separate43475760 data-update registry.
class NativeAgentRuntime {
public:
    NativeAgentRuntime(const Settings& settings, NativeCatalog& catalog,
                       NativeAgentBindings bindings, ModelInfo& models, StringTables& tables,
                       network::DataUpdateObserverRegistry& observers);
    void process(void* player, std::int32_t update); // complete433466F7
private:
    friend class network::DataUpdateObserverRegistry;
    const Settings& settings_;
    NativeCatalog& catalog_;
    NativeAgentBindings bindings_;
    ModelInfo& models_;
    StringTables& tables_;
    network::NativeDataUpdateReceiver data_update_receiver_{};
};
} // namespace skeet::skins
