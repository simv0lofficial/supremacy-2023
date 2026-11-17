#pragma once
#include "skeet/players/state.hpp"
#include <memory>

namespace skeet::engine {
class PreservedInitialization;
class NativeDataImage;
}
namespace skeet::esp {
class EspOwnerRuntime;
struct UserMessageBindings {
    EspOwnerRuntime* const& active_owner; //434758DC, captured once by each entry
    const std::uint8_t& recording_enabled; //4346754C, reloaded at its native gates
    const players::State& adjustments; //correction_active byte4347BE0E + index*32
};

// The SDK has already decoded these messages. The incoming pointers retain
// their original x86 generated-message layout; no message IDs are inferred or
// parsed here. State belongs to the active ordinary ESP/player-cache owner.
class UserMessageHandlers {
public:
    UserMessageHandlers(engine::PreservedInitialization&, UserMessageBindings);
    UserMessageHandlers(std::shared_ptr<engine::NativeDataImage>, UserMessageBindings);
    UserMessageHandlers(const UserMessageHandlers&) = delete;
    UserMessageHandlers& operator=(const UserMessageHandlers&) = delete;
    void weapon_sound(const void* packet); //433E7234
    void spotted(const void* packet); //433FCBD6
    const engine::NativeDataImage& image() const noexcept {
        return *image_;
    }

private:
    void spotted_entry(EspOwnerRuntime&, const void* entry);
    std::shared_ptr<engine::NativeDataImage> image_;
    UserMessageBindings bindings_;
};

// The ordinary-entry bridge binds this process-lifetime context before it
// installs either packet entry and keeps it alive until those entries retire.
void bind_user_message_handlers(UserMessageHandlers&) noexcept;
void unbind_user_message_handlers(UserMessageHandlers&) noexcept;
// The observed entries use plain RET and one stack argument. The preserved
// registration dispatcher's use of a return register is a separate ABI input.
extern "C" void __cdecl weapon_sound_user_message_hook(const void* packet);
extern "C" void __cdecl spotted_user_message_hook(const void* packet);
} //namespace skeet::esp
