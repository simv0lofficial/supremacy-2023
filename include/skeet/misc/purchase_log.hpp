#pragma once
#include "config.hpp"
#include "logs.hpp"
#include "native_text.hpp"
#include "skeet/events/engine_event.hpp"
#include "skeet/events/registry.hpp"
#include "skeet/rage/target_relationships.hpp"
#include <array>

namespace skeet::misc {
// Event-log owner 434819DE: the damage listener is permanent; the purchase
// listener at native owner+44 follows the live purchases descriptor 5086BCF5.
class PurchaseLogRuntime {
public:
    PurchaseLogRuntime(const EventLogConfig& settings, NormalLog& output,
                       rage::TargetRelationshipBindings targets,
                       const NativeEncryptedText& purchase_format,
                       const NativeEncryptedText& hit_format,
                       const std::array<NativeText, 9>& hitgroups, std::uint8_t& subscription_state)
        : settings(settings), output(output), targets(targets), format(purchase_format),
          damage(settings, hit_format, hitgroups), purchase_subscribed_(subscription_state) {}
    PurchaseLogRuntime(const PurchaseLogRuntime&) = delete;
    PurchaseLogRuntime& operator=(const PurchaseLogRuntime&) = delete;

    const EventLogConfig& settings;
    NormalLog& output;
    rage::TargetRelationshipBindings targets;
    void* entity_list = nullptr; // native 4346A890, genuine entity-list interface
    const NativeEncryptedText& format; // single native owner+3C/+40 backing
    events::HitLog damage; // borrows the same owner's +34 format and +54 groups

    // Complete handler433A0D37, called through thunk43418477. The actual event
    // master supplies EngineEventPlayers with its captured local entity.
    void item_purchase(events::GameEventReader&, events::EngineEventPlayers&);
    // Constructor registration434819DE adds only the damage listener. Purchase
    // registration waits for the separate frame notification below.
    void bind_events(events::EventRegistry&);
    // Frameendpoint4336ADB0 runs the transition only when its DL byte is nonzero.
    void configuration_changed(bool notify, events::EventRegistry&);
    // Native433E0610 calls4336B697(0) after freeing the nine hitgroup strings
    // and before freeing the two format strings. The damage triple stays put.
    void disable_purchase_subscription(events::EventRegistry&);
    // Explicit full-lifetime cleanup also removes the permanent damage triple.
    void unbind_events(events::EventRegistry&);

private:
    std::uint8_t& purchase_subscribed_; // single native owner+50 raw byte
    static void purchase_listener(void*, events::GameEventReader&, events::EventPlayerIndices&);
    static void damage_listener(void*, events::GameEventReader&, events::EventPlayerIndices&);
};
} // namespace skeet::misc
