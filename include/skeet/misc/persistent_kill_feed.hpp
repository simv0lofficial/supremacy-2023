#pragma once
#include "skeet/misc/config.hpp"
#include "skeet/events/game_events.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/engine/frame_update.hpp"
#include "skeet/misc/native_text.hpp"

namespace skeet::misc {
struct PersistentKillFeedOffsets {
    std::ptrdiff_t rules_d000a932 = 0; // byte, 4346746C ^43467D50 ^D000A932
    std::ptrdiff_t rules_972bc124 = 0; // integer, 43468BE4 ^4346A378 ^972BC124
    std::ptrdiff_t rules_9f8e7f71 = 0; // float, 43468D6C ^43468428 ^9F8E7F71
};
struct PersistentKillFeedBindings {
    void* const& engine; //4346A514
    void* const& hud_context; //live43468994
    // Actual engine FindHudElement4346A374; its return points at owner+14.
    const engine::MemberFunction<void*, const char*>& find_hud;
    void* const* const& game_rules; //live4346A3A4, may itself be null
    void* const& global_vars; //live43467B04, current time at +10
    PersistentKillFeedOffsets offsets;
};

// Native owner4347E910 registers owner+8 in the pre-event observer vector
// 434758E0 and owner+10 in frame observer vector43475AD8. The menu and this
// runtime read the same descriptor245CEDB5; changing it also enables cleanup.
class PersistentKillFeedRuntime {
public:
    PersistentKillFeedRuntime(const PersistentKillFeedConfig& settings,
                              PersistentKillFeedBindings& bindings, const NativeText& hud_name)
        : settings_(settings), bindings_(bindings), hud_name_(hud_name) {}
    std::uint32_t pending_count = 0; //native owner+28
    void on_event(events::GameEventReader&, std::uint32_t name_hash); //433FA644
    void reset(); //4332B844, restore HUD lifetime1.5
    void update(); //4341783B, clear accumulated notices when disabled
    void release(); //43394306, lifecycle slot4, connected HUD cleanup
    void reset_state() noexcept; //4335C6B7, lifecycle slot5
private:
    void* death_notice();
    void clear(void* owner);
    const PersistentKillFeedConfig& settings_;
    PersistentKillFeedBindings& bindings_;
    const NativeText& hud_name_; //one constructor-owned native433B2213 vector
};
// Constructor4347E910's separate owner+10 frame endpoint, whose one entry is
// 4341783B. Its lifetime is the same as the concrete runtime it references.
class PersistentKillFeedFrameHandler final : public engine::FrameUpdateEndpoint {
public:
    PersistentKillFeedFrameHandler(engine::OwnerRegistryBinding, PersistentKillFeedRuntime&);
    PersistentKillFeedRuntime& runtime;
};
std::size_t persistent_kill_feed_owner_size() noexcept;
// Storage has the native43387448 allocation header. Registers primary owner,
// pre/post event endpoint and frame endpoint before initializing the previously
// unregistered canonical245CEDB5 descriptor. Returns the owned runtime.
PersistentKillFeedRuntime&
construct_persistent_kill_feed_owner(void* storage, engine::OwnerRegistryBinding owners,
                                     engine::OwnerRegistryBinding events,
                                     engine::OwnerRegistryBinding frames, ConfigRegistries&,
                                     PersistentKillFeedConfig&, PersistentKillFeedBindings&);
} //namespace skeet::misc
