#pragma once
#include "config.hpp"
#include "command_owner.hpp"
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/engine/frame_update.hpp"

namespace skeet::misc {
class Runtime;
class RebuyFixOwner;
struct RebuyPrimaryEndpoint {
    void* const* vtable;
    RebuyFixOwner* owner;
};
struct RebuyFrameEndpoint : engine::FrameUpdateEndpoint {
    RebuyFixOwner* owner;
};
struct RebuyFixOwnerBindings {
    ConfigRegistries& configuration;
    RebuyConfig& settings;
    Runtime& miscellaneous;
    events::EventRegistry& events;
    engine::OwnerRegistryBinding owners;
    client::CommandPhaseRegistry& command_owners;
    engine::OwnerRegistryBinding frame_updates;
    void* buy_anywhere_cvar;
    const float* current_time;
    const double* time_4347a500;
    const double* time_4347b118;
    RebuyFixOffsets offsets;
};

// Native4348217D owner: primary cleanup, command phase2, frame-change observer,
// the canonical BooleanConfig and the permanent player_spawn listener.
// The cleanup registry's slot0 destroys and frees this native-header owner; the factory result borrows
// that same object and must not be placed in a second owning smart pointer.
class RebuyFixOwner final {
public:
    explicit RebuyFixOwner(RebuyFixOwnerBindings);
    ~RebuyFixOwner(); //433C1134 ->4332B079, then release native owner storage
    RebuyFixOwner(const RebuyFixOwner&) = delete;
    RebuyFixOwner& operator=(const RebuyFixOwner&) = delete;
    RebuyFixRuntime& runtime() noexcept {
        return runtime_;
    }
    void frame_update(std::uint8_t notify); //complete433C8E40
    void destroy(); //433C1134, then433837D0 on the same process heap
private:
    RebuyFixOwnerBindings bindings_;
    RebuyFixRuntime runtime_;
    CommandOwner command_owner_;
    client::NativeCommandPhaseReceiver command_receiver_{};
    RebuyPrimaryEndpoint primary_;
    RebuyFrameEndpoint frame_;
};
RebuyFixOwner* create_rebuy_fix_owner(RebuyFixOwnerBindings);
} //namespace skeet::misc
