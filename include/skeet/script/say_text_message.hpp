#pragma once
#include "skeet/script/bridge.hpp"
#include "skeet/events/event_name_storage.hpp"

namespace skeet::script {
struct SayTextMessageNames {
    explicit SayTextMessageNames(const events::EventNameStorage& storage)
        : event(storage.alias(events::EventName::player_chat)),
          team_only(storage.alias(events::EventName::teamonly)),
          entity(storage.alias(events::EventName::entity)),
          name(storage.alias(events::EventName::name)),
          text(storage.alias(events::EventName::text)) {}
    events::EventNameRef event; //43479410
    events::EventNameRef team_only; //43479420,8 bytes
    events::EventNameRef entity; //43479430,6 bytes
    events::EventNameRef name; //43479440,4 bytes
    events::EventNameRef text; //434793A0,4 bytes
};
struct SayTextMessageRuntime {
    Bridge& bridge;
    SayTextMessageNames names;
};

// Complete433C05B4 with4332296D ->43327A0C argument conversion. The
// engine owns the decoded SayText2 protobuf object and its string storage.
void dispatch_say_text_message(SayTextMessageRuntime&, const void* packet);
void bind_say_text_message_runtime(SayTextMessageRuntime&) noexcept;
void unbind_say_text_message_runtime(SayTextMessageRuntime&) noexcept;
#if defined(_M_IX86) || defined(__i386__)
void __cdecl say_text_message_hook(const void* packet); //433C05B4,plain RET
#else
void say_text_message_hook(const void* packet);
#endif
} //namespace skeet::script
