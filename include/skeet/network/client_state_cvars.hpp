#pragma once
#include "skeet/engine/interface_call.hpp"
#include "skeet/script/bridge.hpp"
#include "skeet/events/event_name_storage.hpp"
#include <cstddef>
#include <cstdint>

namespace skeet::network {
struct ServerCvarEventNames {
    explicit ServerCvarEventNames(const events::EventNameStorage& storage)
        : event(storage.alias(events::EventName::server_cvar)),
          name(storage.alias(events::EventName::cvarname)),
          value(storage.alias(events::EventName::cvarvalue)) {}
    events::EventNameRef event; //43479450
    events::EventNameRef name; //43479460, exactly8 bytes
    events::EventNameRef value; //43479470, exactly9 bytes
};
using ServerCvarNameGetter = engine::MemberFunction<const char*>;
struct ClientStateServerCvars {
    script::Bridge& bridge;
    std::byte* const& engine_module; //43468D20, capturedengine.dll base5ECA0000
    ServerCvarNameGetter& name_getter; //4347CA4C, genuine resolved engine code
    std::uint8_t& lookup_attempted; //4347CA50
    ServerCvarEventNames names;
};
// Complete43340140 and concrete4339C702 table marshalling. Malformed/empty
// name or value ends the native traversal instead of skipping that element.
void observe_server_cvars(ClientStateServerCvars&, const void* message);
} //namespace skeet::network
