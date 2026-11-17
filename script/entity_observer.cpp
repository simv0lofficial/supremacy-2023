#include "skeet/script/entity_observer.hpp"

namespace skeet::script {
namespace {
int marshal_no_arguments(Bridge&, void*) {
    return 0;
} //43365AE1 ->43336942
}

ScriptEntityObserver::ScriptEntityObserver(Bridge& bridge, client::EntityObserverRegistry& registry)
    : bridge_(bridge) {
    registry.append_scripts(receiver_, *this); //4347FF7A, complete owner+18
}

void ScriptEntityObserver::publish_table(client::EntityObserverRegistry& registry) noexcept {
    registry.publish_scripts(receiver_); //43480004
}

int ScriptEntityObserver::update_entities(void*, std::uintptr_t) {
    return bridge_.state
               ? dispatch_callbacks(bridge_,
                                    bridge_.name(events::EventName::net_update_start).get(),
                                    marshal_no_arguments, nullptr)
               : 0;
}

int ScriptEntityObserver::incoming_complete(void*, std::int32_t) {
    return bridge_.state
               ? dispatch_callbacks(bridge_, bridge_.name(events::EventName::net_update_end).get(),
                                    marshal_no_arguments, nullptr)
               : 0;
}

} // namespace skeet::script
