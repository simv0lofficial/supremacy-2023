#pragma once
#include "skeet/engine/frame_update.hpp"
#include "skeet/misc/purchase_log.hpp"

namespace skeet::script {
struct ConsoleInputHook;
}
namespace skeet::misc {
// Actual434819D2 owner+8 endpoint, table4346EF98. The one method4336ADB0
// consumes the frame dispatcher's DL byte and toggles the canonical purchase
// subscription only on a nonzero notification. All listener storage remains
// in the same concrete PurchaseLogRuntime/EventRegistry.
class PurchaseLogFrameHandler final : public engine::FrameUpdateEndpoint {
public:
    PurchaseLogFrameHandler(engine::OwnerRegistryBinding, PurchaseLogRuntime&,
                            events::EventRegistry&);
    PurchaseLogRuntime& runtime;
    events::EventRegistry& events;
};

class PurchaseLogLifecycleOwner;
std::size_t purchase_log_owner_size() noexcept;
// Complete434819D2/4346F750 composition. Fresh native aligned storage owns the
// ONE canonical config and all native text/subscription fields. Do not also
// initialize_event_log_config or register a separate HitLog listener.
// The first x86E4 bytes match the native object passed to ConsoleInputHook.
// Registration order is primary -> frame -> descriptors -> texts -> damage.
PurchaseLogLifecycleOwner& construct_purchase_log(
    void* storage, engine::OwnerRegistryBinding lifecycle, engine::OwnerRegistryBinding frames,
    ConfigRegistries&, events::EventRegistry&, NormalLog&, rage::TargetRelationshipBindings,
    void* entity_list, script::ConsoleInputHook&, void* const& console_container);
PurchaseLogRuntime& purchase_log_runtime(PurchaseLogLifecycleOwner&) noexcept;
EventLogConfig& purchase_log_configuration(PurchaseLogLifecycleOwner&) noexcept;
// Native primary4346F750 slot9,433410B6; its handler uses the same console
// descriptor/Bridge as capture/restore and forwards the actual engine inputs.
void process_purchase_console_input(PurchaseLogLifecycleOwner&, void* engine_object,
                                    const char* text);
} //namespace skeet::misc
