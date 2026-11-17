#pragma once

// Game Events Subsystem Declarations
#include "skeet/esp/player_runtime.hpp"
#include "skeet/skins/config.hpp"
#include "skeet/skins/sequences.hpp"
#include <array>
#include <cstddef>
#include <span>
#include "skeet/engine/owner_registry.hpp"
#include "skeet/engine/hooks.hpp"
#include "skeet/network/client_state_descriptor.hpp"
#include "skeet/misc/native_text.hpp"
#include <memory>
#include <cstdint>
#include <vector>
#include "skeet/misc/logs.hpp"
#include "skeet/misc/config.hpp"
#include "skeet/rage/target_relationships.hpp"
#include "skeet/render_queue.hpp"
#include "skeet/esp/config.hpp"
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/hud/composite_runtime.hpp"
#include "skeet/esp/effects_config.hpp"


// Declarations from deaths.hpp

namespace skeet::events {
struct DeathBindings {
    void* engine = nullptr;
    void* entity_list = nullptr;
    void* local_player = nullptr;
};
struct WeaponDeathOffsets {
    std::ptrdiff_t team = 0;
    std::ptrdiff_t active_weapon = 0;
    std::ptrdiff_t definition = 0;
    std::ptrdiff_t original_owner_low = 0, original_owner_high = 0;
    std::ptrdiff_t stattrak = 0, attributes = 0;
};
struct DeathWeaponIcons {
    DeathWeaponIcons(const skins::Settings& settings,
                     const skins::SequenceRuntime& sequence) noexcept;
    const skins::Settings& settings;
    const skins::SequenceRuntime& sequence; // live native owner+7C producer
    // Constructor4348DFCB copies the canonical knife rows into owner+80.
    // Selection indexes these same definitions; valid selection is required.
    std::span<const skins::KnifeModel> selected_models;
    const char* weapon_key = "weapon"; // native owner +348
    void player_death(MutableGameEvent&, const DeathBindings&,
                      const WeaponDeathOffsets&); // 4341A07D
};
struct StatTrakDeath {
    // The menu, configuration loader and death event share descriptor B6218FEC.
    const skins::WeaponSkinsConfig& weapons;
    void player_death(GameEventReader&, const DeathBindings&,
                      const WeaponDeathOffsets&); // 43336CC1
};
// Native 433E881F, clearing geometry alpha only for a dormant player entity.
void death_clear_dormant_alpha(GameEventReader&, const DeathBindings&, esp::PlayerRuntimeState&);
} // namespace skeet::events

// Declarations from engine_event.hpp

namespace skeet::events {
// GameEvent slot19 consumes this seven-slot interface, without a destructor
// slot. Native typed visitor4346F114 and Lua visitor43470434 agree on argument
// sizes: slot4 pops12 bytes; all other slots pop8 bytes on x86.
struct EngineEventVisitor {
    virtual bool pointer(const char* key, void* value) = 0;
    virtual bool string(const char* key, const char* value) = 0;
    virtual bool floating(const char* key, float value) = 0;
    virtual bool integer(const char* key, std::int32_t value) = 0;
    virtual bool wide_integer(const char* key, std::uint64_t value) = 0;
    virtual bool opaque_word(const char* key, std::uint32_t value) = 0;
    virtual bool boolean(const char* key, bool value) = 0;
};

// The actual engine event boundary; typed listeners and Lua dispatch consume
// this same event object. The engine owns its lifetime.
class EngineGameEventReader final : public MutableGameEvent {
public:
    explicit EngineGameEventReader(void* event) noexcept : event_(event) {}
    void* raw_event() const noexcept {
        return event_;
    }
    const char* name() const; // event slot1
    bool read(std::uint32_t name_hash, FieldType type, FieldValue& value) override;
    void set_string(const char* key, const char* value) override;
    void set_boolean(const char* key, bool value); // event slot13
private:
    void* event_;
};

// Hashed callbacks receive the local entity captured after the pre-event pass.
// Native comparisons use IClientNetworkable::EntIndex at local+8, slot10.
class EngineEventPlayers final : public HitLogPlayerInfo {
public:
    EngineEventPlayers(void* engine, void* local_player) noexcept
        : engine_(engine), local_(local_player) {}
    void* local_entity() const noexcept {
        return local_;
    }
    std::int32_t player_for_user_id(std::uint32_t user_id) override;
    std::int32_t local_player_index() override;
    bool player_info(std::int32_t index, std::array<std::byte, 0x158>& output) override;

private:
    void* engine_;
    void* local_;
};
} // namespace skeet::events

// Declarations from event_dispatch.hpp

namespace skeet::misc {
class PersistentKillFeedRuntime;
}
namespace skeet::script {
class ClientModule;
}
namespace skeet::events {
// The two methods of native observer vector434758E0/E4/E8. Destruction is not
// an interface slot. Constructors append borrowed observers in their own order.
struct EventObserver {
    virtual void before(EngineGameEventReader& event, std::uint32_t name_hash) = 0;
    virtual void after(EngineGameEventReader& event, std::uint32_t name_hash) = 0;
};
void register_event_observer(engine::OwnerRegistry&, EventObserver&, engine::JobMemory);

struct EventMasterBindings {
    void*& engine; //4346A514, aliases EventRegistry::engine_interface
    void*& event_manager; //43468364, aliases EventRegistry::event_manager
    void* const* local_player; //43475808, read after the pre-event pass
};
class EventMaster {
public:
    EventMaster(EventRegistry& registry, engine::OwnerRegistry& observers,
                EventMasterBindings& bindings) noexcept
        : registry_(registry), observers_(observers), bindings_(bindings) {}
    // Complete433AE68A..433AE847. Demo/HLTV invokes the hook metadata's original
    // virtual34; the ordinary path consumes the packet and returns true.
    bool process(void* hook_receiver, void* event_packet);

private:
    EventRegistry& registry_; //43478DD0/DD4/DD8
    engine::OwnerRegistry& observers_;
    EventMasterBindings& bindings_;
};

// Constructor4347E910 appends owner+8 with vtable4346FB0C. Its second method
// really resets the HUD lifetime after every manager dispatch, even if disabled.
class KillFeedEventObserver final : public EventObserver {
public:
    explicit KillFeedEventObserver(misc::PersistentKillFeedRuntime& runtime) noexcept
        : runtime_(runtime) {}
    void before(EngineGameEventReader&, std::uint32_t name_hash) override; //433FA644
    void after(EngineGameEventReader&, std::uint32_t name_hash) override; //4332B844
private:
    misc::PersistentKillFeedRuntime& runtime_;
};

// Constructor4347FFB5 appends owner+40 with vtable4346F6C8. The Lua registry
// is separate from the C++ registry; only its first matching hash is dispatched.
class LuaEventObserver final : public EventObserver {
public:
    LuaEventObserver(script::ClientModule& module, void* const* local_player) noexcept
        : module_(module), local_(local_player) {}
    void before(EngineGameEventReader&, std::uint32_t name_hash) override; //4335B986
    void after(EngineGameEventReader&, std::uint32_t) override {} //433C95B7
private:
    script::ClientModule& module_;
    void* const* local_;
};
} // namespace skeet::events

// Declarations from event_hook.hpp

namespace skeet::events {

// Captured initialization operands0860/0864 bind slot34 of descriptor43475A3C
// to433AE68A. Its thirteen replacements belong to the earlier protected hook
// scheduler;4341F57F does not install this hook.
inline constexpr std::size_t event_packet_slot = network::client_state_event_slot;

// Owns the source dispatcher behind the actual engine-vtable entry. The event
// registry, observer vector and local-player pointer slot are the same objects
// populated by feature constructors. In particular, no engine/manager copy is
// kept apart from the registry used to publish manager+A0 subscription changes.
class EventPacketHookRuntime final {
public:
    EventPacketHookRuntime(EventRegistry& registry, engine::OwnerRegistry& observers,
                           void* const* local_player) noexcept;
    EventPacketHookRuntime(const EventPacketHookRuntime&) = delete;
    EventPacketHookRuntime& operator=(const EventPacketHookRuntime&) = delete;

    bool process(void* receiver, void* packet);

private:
    EventMasterBindings bindings_;
    EventMaster master_;
};

// Publishes the existing dispatcher only; the complete ClientStateHookFamily
// prepares all thirteen replacements and performs one shared installation.
void bind_event_packet_hook_runtime(EventPacketHookRuntime& runtime) noexcept;

// The caller supplies the actual prepared descriptor43475A3C and receiver.
// Source-global43468D20 is engine.dll;747CA973 is the RTTI CClientState type
// hash, with base-type hash0. The genuine RTTI resolver/descriptor clone and
// the other twelve replacements must already have completed. This per-entry
// adapter invokes complete4331E834 using the canonical cleanup sentinel; the
// whole-family production caller uses the publisher above and installs once.
// Runtime and metadata must outlive restoration by UnloadRuntime::cleanup.
engine::VtableHookNode* install_event_packet_hook(EventPacketHookRuntime& runtime,
                                                  engine::VtableHookRegistry& hooks, void* receiver,
                                                  engine::VtableHookMetadata& metadata);

#if defined(_M_IX86) || defined(__i386__)
bool __fastcall event_packet_hook(void* receiver, void* ignored_edx, void* packet);
#else
bool event_packet_hook(void* receiver, void* ignored_edx, void* packet);
#endif
} // namespace skeet::events

// Declarations from event_name_storage.hpp

namespace skeet::client {
class NativeStartupSources;
struct ExitCallbacks;
}
namespace skeet::engine {
class PreservedInitialization;
class NativeDataImage;
}
namespace skeet::events {
struct EventRegistry;
class StandaloneEventNameStorage;

// Addresses are the actual first words of42 native16-byte string cells.
enum class EventName : std::uint32_t {
    pre_render = 0x43479360,
    pre_render_3d = 0x43479370,
    override_view = 0x43479380,
    string_cmd = 0x43479390,
    text = 0x434793A0,
    pre_config_load = 0x434793B0,
    post_config_load = 0x434793C0,
    pre_config_save = 0x434793D0,
    post_config_save = 0x434793E0,
    send_datagram = 0x434793F0,
    send_net_msg = 0x43479400,
    player_chat = 0x43479410,
    teamonly = 0x43479420,
    entity = 0x43479430,
    name = 0x43479440,
    server_cvar = 0x43479450,
    cvarname = 0x43479460,
    cvarvalue = 0x43479470,
    invalid_event_name = 0x43479480,
    hitgroup = 0x43479490,
    damage = 0x434794A0,
    hit_chance = 0x434794B0,
    boosted = 0x434794C0,
    high_priority = 0x434794D0,
    aim_fire = 0x434794E0,
    aim_hit = 0x434794F0,
    aim_miss = 0x43479500,
    console_input = 0x43479510,
    net_update_start = 0x43479520,
    net_update_end = 0x43479530,
    level_init = 0x43479540,
    run_command = 0x43479550,
    finish_command = 0x43479560,
    chokedcommands = 0x43479570,
    command_number = 0x43479580,
    post_render = 0x43479590,
    paint = 0x434795A0,
    paint_ui = 0x434795B0,
    pre_predict_command = 0x434795C0,
    predict_command = 0x434795D0,
    setup_command = 0x434795E0,
    voice = 0x434795F0
};

// Copies retain a binding to the native row, never its current pointer/string.
// Predefined consumers read get() at their original argument-read points.
class EventNameRef {
public:
    explicit EventNameRef(const misc::NativeText& cell) noexcept : cell_(cell) {}
    const char* get() const noexcept {
        return cell_.begin;
    }
    operator const char*() const noexcept {
        return get();
    }
    const misc::NativeText& native() const noexcept {
        return cell_;
    }

private:
    const misc::NativeText& cell_;
};

class EventNameStorage {
    friend class StandaloneEventNameStorage;
    std::shared_ptr<engine::NativeDataImage> image_;

public:
    static constexpr std::uint32_t native_base = 0x43479360;
    static constexpr std::size_t count = 42;
    using Cells = std::array<misc::NativeText, count>;
    explicit EventNameStorage(engine::PreservedInitialization&);
    explicit EventNameStorage(std::shared_ptr<engine::NativeDataImage>);
    EventNameStorage(const EventNameStorage&) = delete;
    EventNameStorage& operator=(const EventNameStorage&) = delete;
    EventNameStorage(EventNameStorage&&) = delete;
    EventNameStorage& operator=(EventNameStorage&&) = delete;
    Cells& cells; //required actual image-native43479360 cells; never reset here
    const engine::NativeDataImage& image() const;
    std::shared_ptr<engine::NativeDataImage> image_owner() const noexcept {
        return image_;
    }
    misc::NativeText& at(EventName);
    const misc::NativeText& at(EventName) const;
    EventNameRef alias(EventName name) const {
        return EventNameRef(at(name));
    }

private:
    explicit EventNameStorage(Cells& snapshot) noexcept : cells(snapshot) {}
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(EventNameStorage::Cells) == 0x2A0);
#endif

// Explicit isolated fixture ownership. Production cannot obtain this private
// constructor through a default or detached singleton. image() rejects this
// snapshot, so it cannot pass the live startup graph's image identity guard.
class StandaloneEventNameStorage {
    // Longest constructor operand is19 visible bytes. These are explicit
    // consumer-fixture buffers, not allocations or default production names.
    std::array<std::array<char, 20>, EventNameStorage::count> backing_{};
    EventNameStorage::Cells cells_{};
    EventNameStorage view_;

public:
    StandaloneEventNameStorage() noexcept;
    StandaloneEventNameStorage(const StandaloneEventNameStorage&) = delete;
    StandaloneEventNameStorage& operator=(const StandaloneEventNameStorage&) = delete;
    StandaloneEventNameStorage(StandaloneEventNameStorage&&) = delete;
    StandaloneEventNameStorage& operator=(StandaloneEventNameStorage&&) = delete;
    EventNameStorage& storage() noexcept {
        return view_;
    }
    const EventNameStorage& storage() const noexcept {
        return view_;
    }
};
// All production consumers borrow the one retained image view. Allocation
// occurs only at43480770, not when that view or its registry is constructed.
EventNameStorage& initialize_event_name_storage(EventNameStorage&, engine::JobMemory); //43480770
void finalize_event_name_storage(EventNameStorage&, engine::JobMemory); //43419B17 ->4332B67C

class EventNameStartupBindings {
public:
    EventNameStartupBindings(EventRegistry& registry, client::ExitCallbacks& callbacks,
                             engine::JobMemory memory) noexcept
        : registry(registry), callbacks(callbacks), memory(memory) {}
    EventNameStartupBindings(const EventNameStartupBindings&) = delete;
    EventNameStartupBindings& operator=(const EventNameStartupBindings&) = delete;
    EventNameStartupBindings(EventNameStartupBindings&&) = delete;
    EventNameStartupBindings& operator=(EventNameStartupBindings&&) = delete;
    EventRegistry& registry;
    client::ExitCallbacks& callbacks;
    engine::JobMemory memory;
    void initialize();
    void finalize();
    void (*source_finalizer(std::uint32_t native_address))();
};
// Individual4331EDEC/operand1F4: construct all rows, then append43419B17 to
// the SAME existing433837F5 exit registry. Binding does not execute startup.
void bind_event_name_initializer(client::NativeStartupSources&, EventNameStartupBindings&);
} // namespace skeet::events

// Declarations from feature_listeners.hpp

namespace skeet::events {
// These adapters supply the native callback's captured local entity and borrow
// the actual feature owners. Each bind corresponds to the constructor's one
// unconditional registration; feature enable gates remain in the handlers.
struct HitLogListener {
    HitLog& owner;
    misc::NormalLog& output;
    void bind(EventRegistry&); //43481D24, wrapper4339FA72
    void unbind(EventRegistry&);

private:
    static void dispatch(void*, GameEventReader&, EventPlayerIndices&);
};
struct HitMarkerListener {
    HitMarker& owner;
    HitMarkerBindings& bindings;
    void bind(EventRegistry&); //4348661D, wrapper433BFDF6
    void unbind(EventRegistry&);

private:
    static void dispatch(void*, GameEventReader&, EventPlayerIndices&);
};
struct DeathWeaponIconsListener {
    DeathWeaponIcons& owner;
    DeathBindings& bindings;
    WeaponDeathOffsets& offsets;
    void bind(EventRegistry&); //4348E073, wrapper433ABC62
    void unbind(EventRegistry&);

private:
    static void dispatch(void*, GameEventReader&, EventPlayerIndices&);
};
struct StatTrakDeathListener {
    StatTrakDeath& owner;
    DeathBindings& bindings;
    WeaponDeathOffsets& offsets;
    // This is the single production StatTrak route, calling raw native owner
    // 43336CC1 with the canonical WeaponSkinsConfig borrowed by StatTrakDeath.
    void bind(EventRegistry&); //43490360, wrapper433F9037
    void unbind(EventRegistry&);

private:
    static void dispatch(void*, GameEventReader&, EventPlayerIndices&);
};
struct DormantAlphaDeathListener {
    DeathBindings& bindings;
    esp::PlayerRuntimeState& players; // consumes the canonical PlayerStateCache
    void bind(EventRegistry&); //43483E3F, wrapper433ECACC
    void unbind(EventRegistry&);

private:
    static void dispatch(void*, GameEventReader&, EventPlayerIndices&);
};
} // namespace skeet::events

// Declarations from game_events.hpp

namespace skeet::events {

// 0x43359C70: FNV-1a followed by the build's final XOR. Null is accepted.
std::uint32_t event_hash(const char* text) noexcept;

inline constexpr std::uint32_t weapon_fire_hash = 0xEA688D0F;
inline constexpr std::uint32_t bullet_impact_hash = 0x19459119;
inline constexpr std::uint32_t player_hurt_hash = 0x89FA8001;
inline constexpr std::uint32_t player_death_hash = 0x3461B584;

enum class FieldType : std::uint32_t {
    floating = 1,
    string = 2,
    integer = 5,
    boolean = 6
};
union FieldValue {
    std::uint32_t integer;
    float floating;
    const char* string;
    bool boolean;
    constexpr FieldValue() : string(nullptr) {}
};

// The listener's typed field visitor, native vtable 4346F114. Failed matches
// leave the previous value untouched; false stops the engine's key scan.
struct FieldQuery {
    std::uint32_t name_hash;
    FieldType type;
    FieldValue value;
    bool integer(const char* name, std::uint32_t input) noexcept; // 4339C769
    bool floating(const char* name, float input) noexcept; // 433B21D7
    bool string(const char* name, const char* input) noexcept; // 4341848A
    bool boolean(const char*, bool) noexcept; // 43337B89
    bool wide_integer(const char*, std::uint64_t) const noexcept {
        return true;
    } // 433ADABB
};

// Engine event vtable +0x4C. True means the requested field could not be read;
// the original listeners return immediately on that result.
struct GameEventReader {
    virtual ~GameEventReader() = default;
    virtual bool read(std::uint32_t name_hash, FieldType type, FieldValue& value) = 0;
};
struct MutableGameEvent : GameEventReader {
    virtual void set_string(const char* key, const char* value) = 0; // event slot16
};
struct EventPlayerIndices {
    virtual ~EventPlayerIndices() = default;
    virtual std::int32_t player_for_user_id(std::uint32_t user_id) = 0;
    virtual std::int32_t local_player_index() = 0;
};

enum class ShotEventKind : std::uint32_t {
    fire,
    impact,
    hurt,
    death
};
// Native queue elements occupy 0x20 bytes. The listeners write payload_words()
// payload values; the remaining native append bytes are stack residue.
struct ShotEvent {
    ShotEventKind kind{};
    std::array<std::uint32_t, 7> payload{};
    std::uint32_t payload_words() const noexcept;
};
static_assert(sizeof(ShotEvent) == 0x20);

struct GameEventState {
    bool enabled = false; // native 0x4346754C
    bool has_pending_shots = false; // owner +0x4C != owner +0x50
    std::vector<ShotEvent> feedback; // owner +0xE0
};

// Complete native listener owners: 4338A387, 43400F91, 433B6725, 4341ADCB.
void weapon_fire(GameEventState&, GameEventReader&, EventPlayerIndices&);
void bullet_impact(GameEventState&, GameEventReader&, EventPlayerIndices&);
void player_hurt(GameEventState&, GameEventReader&, EventPlayerIndices&);
void player_death(GameEventState&, GameEventReader&, EventPlayerIndices&);

} // namespace skeet::events

// Declarations from hit_log.hpp

namespace skeet::events {
struct HitLogPlayerInfo : EventPlayerIndices {
    // Engine vtable +0x20, the native 0x158-byte player_info_t output.
    virtual bool player_info(std::int32_t index, std::array<std::byte, 0x158>& output) = 0;
};
// Complete 4339FF69: sanitize only bell/tab/LF/CR, truncate successful engine
// names at byte 20, or format the player index when the engine query fails.
std::array<char, 128> player_log_name(std::int32_t index, HitLogPlayerInfo&);

struct HitLog {
    HitLog(const misc::EventLogConfig& settings, const misc::NativeEncryptedText& format,
           const std::array<misc::NativeText, 9>& hitgroups)
        : settings(settings), format(format), hitgroups(hitgroups) {}
    const misc::EventLogConfig& settings; // live descriptor0CE14F5A, owner+30
    const misc::NativeEncryptedText& format; // same native owner+34/+38 backing
    const std::array<misc::NativeText, 9>& hitgroups; // same native owner+54..D4
    // 433E0D36, registered for player_hurt. Defaults recovered from 434819D2.
    void player_hurt(GameEventReader&, HitLogPlayerInfo&, misc::NormalLog&);
};
} // namespace skeet::events

// Declarations from hit_marker.hpp

namespace skeet::events {
struct HitMarkerView {
    const void* player = nullptr;
    std::ptrdiff_t rejection_byte = 0; // decoder 43468E78 ^434684E4 ^9D920E35
    float frame_time = 0; // globals +14
    std::int32_t width = 0, height = 0; // renderer context +F4 / +F0
    float font_scale = 1; // renderer context +104
};
struct HitMarkerBindings {
    void* const& engine;
    void* const& entity_list;
    void* const& surface; // live434689C0, distinct from constructor's captured+C
    rage::TargetRelationshipBindings enemy;
    const std::uint8_t& sound_blocked; // live43468E58
    // Live4346D558 pointer; a null value is the native no-list case.
    std::int32_t* const& sound_players;
};
struct HitMarker {
    HitMarker(const esp::HitMarkerConfig& settings, float& remaining,
              const misc::NativeText& sound) noexcept
        : settings(settings), remaining(remaining), sound(sound) {}
    const esp::HitMarkerConfig& settings; // canonical9C345DFA /69C9D9F2
    float& remaining; // one native owner+38 cell
    const misc::NativeText& sound; // one native owner+3C string
    void player_hurt(GameEventReader&, HitMarkerBindings&, void* local_player); // 4337F176
    void draw(ui::RenderQueue&, const HitMarkerView&); // complete 433D066C
    // Production entry: clock, rejection offset and supplied queue dimensions
    // are read inside their actual native gates, never at owner construction.
    void draw_live(ui::RenderQueue&, void* player, void* const& global_variables,
                   const engine::DecodedOffset& rejection_byte);
    void reset() noexcept {
        remaining = 0;
    } // 4336897C
};
} // namespace skeet::events

// Declarations from hit_marker_owner.hpp

namespace skeet::events {
class HitMarkerLifecycleOwner;
struct HitMarkerOwnerBindings {
    esp::HitMarkerConfig& settings;
    float& remaining; // one canonical native+38 cell
    misc::NativeText& sound; // one canonical native+3C text backing
    HitMarkerBindings& events;
    void* const& global_variables; // live43467B04, frame time+14
    engine::DecodedOffset rejection_byte; //43468E78 ^434684E4 ^9D920E35
};
inline constexpr std::size_t native_hit_marker_allocation_size = 0x4C;
std::size_t hit_marker_owner_size() noexcept;

// Whole43486513 constructor and primary4347253C. Caller supplies native-header
// storage large enough for hit_marker_owner_size(). The canonical settings,
// remaining/text cells, event bindings and all registries outlive this owner.
// Native order: primary -> HUD -> capture surface -> descriptors -> remaining0
// -> sound text -> one permanent hurt listener. Do not separately initialize
// these descriptors or register another HitMarkerListener/HUD handler.
HitMarkerLifecycleOwner& construct_hit_marker(void* storage, engine::OwnerRegistryBinding lifecycle,
                                              engine::OwnerRegistryBinding hud, ConfigRegistries&,
                                              EventRegistry&, HitMarkerOwnerBindings);
HitMarker& hit_marker_runtime(HitMarkerLifecycleOwner&) noexcept;

// Factory43484954 requests0x4C and tail-calls43486513 on success. Its failure
// target43484967 is not recovered by the supplied instruction records; this
// constructor entry leaves that protected edge and startup order unchanged.
} // namespace skeet::events

// Declarations from registry.hpp

namespace skeet::events {
using EventCallback = void (*)(void* owner, GameEventReader&, EventPlayerIndices&);
struct EventListener {
    std::uint32_t name_hash;
    void* owner;
    EventCallback callback;
};
struct EventRegistry {
private:
    void* standalone_engine_ = nullptr;
    void* standalone_manager_ = nullptr;

public:
    explicit EventRegistry(EventNameStorage& names) noexcept
        : name_storage(names), engine_interface(standalone_engine_),
          event_manager(standalone_manager_) {}
    EventRegistry(EventNameStorage& names, void*& engine, void*& manager) noexcept
        : name_storage(names), engine_interface(engine), event_manager(manager) {}
    EventRegistry(const EventRegistry&) = delete;
    EventRegistry& operator=(const EventRegistry&) = delete;
    EventRegistry(EventRegistry&&) = delete;
    EventRegistry& operator=(EventRegistry&&) = delete;
    EventNameStorage& name_storage;
    std::vector<EventListener> listeners;
    bool subscriptions_dirty = false;
    void*& engine_interface; // live4346A514, IsConnected virtual26
    void*& event_manager; // live43468364, dirty byte+A0

    // Native 4336BAAD / 4341B241. Exact triple equality, stable insertion/erase.
    bool add(EventListener listener, bool engine_connected);
    bool remove(EventListener listener, bool engine_connected);
    // Complete engine-bound entries: the connection query happens only after
    // an actual insert/erase, then publishes the real event-manager dirty byte.
    bool add(EventListener listener);
    bool remove(EventListener listener);
    // 433AE7A5..433AE7E4, after the engine's pre-event observers have run.
    void dispatch(std::uint32_t name_hash, GameEventReader&, EventPlayerIndices* local_player);
    bool contains(std::uint32_t name_hash) const noexcept;
    EventNameRef name(EventName id) const {
        return name_storage.alias(id);
    }
};

// Constructor 4347F89E..4347F8E4 registers these three event types. The impact
// triple is stored separately at owner+100, with subscription state at+10C.
void bind_shot_events(EventRegistry&, GameEventState&, bool engine_connected);
void unbind_shot_events(EventRegistry&, GameEventState&, bool engine_connected);
void bind_shot_events(EventRegistry&, GameEventState&);
void unbind_shot_events(EventRegistry&, GameEventState&);
void dispatch_shot_event(GameEventState&, std::uint32_t name_hash, GameEventReader&,
                         EventPlayerIndices&);

struct SubscriptionDescriptor {
    std::int32_t identifier;
    const char* name;
};
// Native 4336B72D..4336B820: OR interested engine events into existing bits.
// Engine descriptors supply valid nonnegative identifiers within mask storage.
void enable_event_subscriptions(const EventRegistry&, std::span<const std::uint32_t> lua_names,
                                std::span<const SubscriptionDescriptor>,
                                std::span<std::uint32_t> mask);
// Complete4336B72D raw-engine owner walk. Manager+4/+10 is an array of0x34-byte
// descriptors; manager+78 resolves descriptor+4 names through0x18-byte entries.
// The native mask object stores its writable word pointer at+C.
void enable_engine_event_subscriptions(const EventRegistry&,
                                       std::span<const std::uint32_t> lua_names,
                                       const void* event_manager, void* mask_object);
} // namespace skeet::events

// Declarations from server_impacts.hpp

namespace skeet::events {
struct ServerImpactBindings {
    void* const& debug_overlay; // live43468260, genuine AddBoxOverlay virtual1
    const std::uint8_t& suspended; // live43468E58
};
struct ServerImpactRuntime {
    const esp::EffectsSettings& settings; // same canonical4348E6E2 descriptors
    const ServerImpactBindings& bindings;
};
// Complete43387294, reached by event-listener thunk43359E08. The enclosing
// effects owner registers19459119 and supplies the captured local player.
void process_server_impact(ServerImpactRuntime&, GameEventReader&, EventPlayerIndices&);
} // namespace skeet::events
