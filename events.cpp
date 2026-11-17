// Game Events Subsystem Implementation
#include "skeet/events/deaths.hpp"
#include "skeet/engine/interface_call.hpp"
#include <bit>
#include <cstring>
#include "skeet/events/engine_event.hpp"
#include "skeet/events/event_dispatch.hpp"
#include "skeet/engine/unload.hpp"
#include "skeet/misc/persistent_kill_feed.hpp"
#include "skeet/script/client_api.hpp"
#include <algorithm>
#include "skeet/events/event_hook.hpp"
#include "skeet/events/event_name_storage.hpp"
#include "skeet/events/registry.hpp"
#include "skeet/client/native_startup.hpp"
#include "skeet/client/shutdown.hpp"
#include "skeet/engine/preserved_initialization.hpp"
#include <stdexcept>
#include <utility>
#include "skeet/events/feature_listeners.hpp"
#include "skeet/events/game_events.hpp"
#include "skeet/events/hit_log.hpp"
#include <cstdio>
#include "skeet/events/hit_marker.hpp"
#include "skeet/core.hpp"
#include <xmmintrin.h>
#include "skeet/events/hit_marker_owner.hpp"
#include <array>
#include <new>
#include "skeet/events/server_impacts.hpp"
#include <immintrin.h>


// ============================================================================
// Subcomponent: deaths.cpp
// ============================================================================

namespace skeet::events {
namespace {
template <class T> T field(const void* object, std::ptrdiff_t offset) {
    std::remove_const_t<T> value{};
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
template <class T> void assign(void* object, std::ptrdiff_t offset, T value) {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof value);
}
void* active_weapon(const DeathBindings& bindings, const WeaponDeathOffsets& offsets) {
    const auto handle = field<std::uint32_t>(bindings.local_player, offsets.active_weapon);
    if (handle == 0xFFFFFFFF)
        return nullptr;
    return engine::invoke<void*>(bindings.entity_list, 4, handle);
}
}
void death_clear_dormant_alpha(GameEventReader& event, const DeathBindings& bindings,
                               esp::PlayerRuntimeState& state) {
    FieldValue victim;
    if (event.read(0x8361150E, FieldType::integer, victim))
        return;
    const auto index = engine::invoke<std::int32_t>(bindings.engine, 9, victim.integer);
    if (static_cast<std::uint32_t>(index) - 1 > 63)
        return;
    const auto entity = engine::invoke<void*>(bindings.entity_list, 3, index);
    if (!entity || !engine::invoke<bool>(entity, 158))
        return;
    if (!engine::invoke<bool>(static_cast<std::byte*>(entity) + 8, 9))
        return;
    state.geometry(static_cast<std::uint32_t>(index) & 63).alpha = 0;
}
DeathWeaponIcons::DeathWeaponIcons(const skins::Settings& settings,
                                   const skins::SequenceRuntime& sequence) noexcept
    : settings(settings), sequence(sequence), selected_models(skins::knife_models()) {}
void DeathWeaponIcons::player_death(MutableGameEvent& event, const DeathBindings& bindings,
                                    const WeaponDeathOffsets& offsets) {
    if (!bindings.local_player)
        return;
    if (engine::invoke<bool>(bindings.engine, 82))
        return;
    if (engine::invoke<bool>(bindings.engine, 93))
        return;
    const auto team = field<std::int32_t>(bindings.local_player, offsets.team);
    if (!(team == 3 ? settings.knife_ct.value : settings.knife_t.value))
        return;
    // 4341A0E8 captures this live producer value once, before reading fields.
    const auto original_definition = sequence.original_definition;
    if (original_definition == -1)
        return;
    FieldValue attacker, weapon;
    if (event.read(0x455B4DBF, FieldType::integer, attacker))
        return;
    if (event.read(0xFDF97DB0, FieldType::string, weapon))
        return;
    const auto source = engine::invoke<std::int32_t>(bindings.engine, 9, attacker.integer);
    if (source !=
        engine::invoke<std::int32_t>(static_cast<std::byte*>(bindings.local_player) + 8, 10))
        return;
    const auto item = active_weapon(bindings, offsets);
    if (!item)
        return;
    const auto selection = field<std::int32_t>(bindings.local_player, offsets.team) == 3
                               ? settings.knife_model_ct.value
                               : settings.knife_model_t.value;
    const auto definition = field<std::uint16_t>(item, offsets.definition);
    if (selected_models[selection].definition != definition || !weapon.string || !*weapon.string)
        return;
    const auto expected = event_hash(weapon.string);
    assign(item, offsets.definition, static_cast<std::uint16_t>(original_definition));
    const auto previous_name = engine::invoke<const char*>(item, 459);
    assign(item, offsets.definition, definition);
    if (expected != event_hash(previous_name + 7))
        return;
    const auto replacement = engine::invoke<const char*>(item, 459);
    if (replacement)
        event.set_string(weapon_key, replacement + 7);
}
void StatTrakDeath::player_death(GameEventReader& event, const DeathBindings& bindings,
                                 const WeaponDeathOffsets& offsets) {
    FieldValue attacker;
    if (event.read(0x455B4DBF, FieldType::integer, attacker))
        return;
    const auto source = engine::invoke<std::int32_t>(bindings.engine, 9, attacker.integer);
    if (source !=
        engine::invoke<std::int32_t>(static_cast<std::byte*>(bindings.local_player) + 8, 10))
        return;
    const auto item = active_weapon(bindings, offsets);
    if (!item)
        return;
    const auto owner_high = field<std::uint32_t>(item, offsets.original_owner_high);
    const auto owner_low = field<std::uint32_t>(item, offsets.original_owner_low);
    std::array<std::byte, 0x158> info{};
    const auto index = engine::invoke<std::int32_t>(bindings.engine, 12);
    if (!engine::invoke<bool>(bindings.engine, 8, index, info.data()))
        return;
    const auto id_low = field<std::uint32_t>(info.data(), 8),
               id_high = field<std::uint32_t>(info.data(), 12);
    if ((!id_low && !id_high) || owner_low != id_low || owner_high != id_high)
        return;
    auto definition = static_cast<std::uint32_t>(field<std::uint16_t>(item, offsets.definition));
    if (!definition)
        return;
    if (definition - 500 <= 25) {
        definition = 42;
        const auto team = field<std::int32_t>(bindings.local_player, offsets.team);
        if (team != 3 && field<std::int32_t>(bindings.local_player, offsets.team) == 2)
            definition = 59;
    } else if (definition >= 65)
        return;
    const auto& settings = weapons.value[definition];
    if (!settings.enabled || !settings.stattrak)
        return;
    assign(item, offsets.stattrak, field<std::uint32_t>(item, offsets.stattrak) + 1);
    const auto count = field<std::int32_t>(item, offsets.attributes + 0x10);
    if (count <= 0)
        return;
    auto attribute = field<std::byte*>(item, offsets.attributes + 4);
    for (std::int32_t entry = 0; entry < count; ++entry, attribute += 0x18)
        if (field<std::uint32_t>(attribute, 4) == 80) {
            assign(attribute, 8, field<std::uint32_t>(attribute, 8) + 1);
            return;
        }
}
} // namespace skeet::events

// ============================================================================
// Subcomponent: engine_event.cpp
// ============================================================================

namespace skeet::events {
namespace {
class QueryVisitor final : public EngineEventVisitor {
public:
    QueryVisitor(std::uint32_t hash, FieldType type, FieldValue value) : query{hash, type, value} {}
    FieldQuery query;
    bool pointer(const char*, void*) override {
        return true;
    } //433F1111
    bool string(const char* key, const char* value) override {
        return query.string(key, value);
    }
    bool floating(const char* key, float value) override {
        return query.floating(key, value);
    }
    bool integer(const char* key, std::int32_t value) override {
        return query.integer(key, static_cast<std::uint32_t>(value));
    }
    bool wide_integer(const char* key, std::uint64_t value) override {
        return query.wide_integer(key, value); //433ADABB
    }
    bool opaque_word(const char*, std::uint32_t) override {
        return true;
    } //433F1111
    bool boolean(const char* key, bool value) override {
        return query.boolean(key, value);
    }
};
}
const char* EngineGameEventReader::name() const {
    return engine::invoke<const char*>(event_, 1);
}
bool EngineGameEventReader::read(std::uint32_t hash, FieldType type, FieldValue& value) {
    QueryVisitor visitor(hash, type, value);
    const auto result = engine::invoke<bool, EngineEventVisitor*>(event_, 19, &visitor);
    value = visitor.query.value;
    return result;
}
void EngineGameEventReader::set_string(const char* key, const char* value) {
    engine::invoke<void, const char*, const char*>(event_, 16, key, value);
}
void EngineGameEventReader::set_boolean(const char* key, bool value) {
    engine::invoke<void, const char*, bool>(event_, 13, key, value);
}
std::int32_t EngineEventPlayers::player_for_user_id(std::uint32_t user_id) {
    return engine::invoke<std::int32_t>(engine_, 9, user_id);
}
std::int32_t EngineEventPlayers::local_player_index() {
    return engine::invoke<std::int32_t>(static_cast<std::byte*>(local_) + 8, 10);
}
bool EngineEventPlayers::player_info(std::int32_t index, std::array<std::byte, 0x158>& output) {
    return engine::invoke<bool>(engine_, 8, index, output.data());
}
} // namespace skeet::events

// ============================================================================
// Subcomponent: event_dispatch.cpp
// ============================================================================

namespace skeet::events {
void register_event_observer(engine::OwnerRegistry& registry, EventObserver& observer,
                             engine::JobMemory memory) {
    void* pointer = &observer;
    engine::append_owner(registry, &pointer, memory);
}
bool EventMaster::process(void* receiver, void* packet) {
    if (engine::invoke<bool>(bindings_.engine, 82) || engine::invoke<bool>(bindings_.engine, 93)) {
        const auto table = *static_cast<void***>(receiver);
        const auto* metadata = static_cast<const engine::VtableHookMetadata*>(table[-2]);
        // The native encoded byte displacement resolves to0x88: virtual34.
        const auto original =
            reinterpret_cast<engine::MemberFunction<bool, void*>>(metadata->original_table[34]);
        return original(receiver, packet);
    }
    void* raw_event = engine::invoke<void*, void*>(bindings_.event_manager, 13, packet);
    if (!raw_event)
        return true;
    EngineGameEventReader event(raw_event);
    std::uint32_t passthrough;
    std::memcpy(&passthrough, static_cast<const std::byte*>(packet) + 0x20, sizeof passthrough);
    if (passthrough == 1)
        event.set_boolean("realtime_passthrough", true);
    const auto hash = event_hash(event.name());

    // Native captures both pointer bounds before calling any pre-event method.
    auto** current = observers_.begin;
    auto** const end = observers_.end;
    for (; current != end; ++current)
        static_cast<EventObserver*>(*current)->before(event, hash);

    void* const local = *bindings_.local_player;
    if (local) {
        EngineEventPlayers players(bindings_.engine, local);
        registry_.dispatch(hash, event, &players);
    }
    engine::invoke<void, void*>(bindings_.event_manager, 9, raw_event);

    // The manager call can change the observer vector. Native reloads both
    // bounds and traverses the resulting sequence in reverse order.
    auto** const begin = observers_.begin;
    current = observers_.end;
    while (current != begin) {
        --current;
        static_cast<EventObserver*>(*current)->after(event, hash);
    }
    return true;
}
void KillFeedEventObserver::before(EngineGameEventReader& event, std::uint32_t hash) {
    runtime_.on_event(event, hash);
}
void KillFeedEventObserver::after(EngineGameEventReader&, std::uint32_t) {
    runtime_.reset();
}
void LuaEventObserver::before(EngineGameEventReader& event, std::uint32_t hash) {
    if (!*local_)
        return;
    const auto subscriptions = module_.event_subscriptions();
    if (std::find(subscriptions.begin(), subscriptions.end(), hash) != subscriptions.end())
        module_.dispatch_engine_event(event.raw_event());
}
} // namespace skeet::events

// ============================================================================
// Subcomponent: event_hook.cpp
// ============================================================================

namespace skeet::events {
namespace {
EventPacketHookRuntime* active_runtime = nullptr;
}

EventPacketHookRuntime::EventPacketHookRuntime(EventRegistry& registry,
                                               engine::OwnerRegistry& observers,
                                               void* const* local_player) noexcept
    : bindings_{registry.engine_interface, registry.event_manager, local_player},
      master_(registry, observers, bindings_) {}

bool EventPacketHookRuntime::process(void* receiver, void* packet) {
    return master_.process(receiver, packet);
}

void bind_event_packet_hook_runtime(EventPacketHookRuntime& runtime) noexcept {
    active_runtime = &runtime;
}

engine::VtableHookNode* install_event_packet_hook(EventPacketHookRuntime& runtime,
                                                  engine::VtableHookRegistry& hooks, void* receiver,
                                                  engine::VtableHookMetadata& metadata) {
    // Publish the dispatcher before4331E834 swaps the receiver's table. Native
    // can install the table even when allocation of its cleanup node fails.
    bind_event_packet_hook_runtime(runtime);
    metadata.replacement_table[event_packet_slot] = reinterpret_cast<void*>(&event_packet_hook);
    return engine::install_vtable_hook(hooks, receiver, metadata);
}

#if defined(_M_IX86) || defined(__i386__)
bool __fastcall event_packet_hook(void* receiver, void*, void* packet) {
#else
bool event_packet_hook(void* receiver, void*, void* packet) {
#endif
    // Native433AE68A receives ECX=receiver and one stack argument (RET4).
    // The spare fastcall EDX parameter preserves that exact x86 entry ABI.
    return active_runtime->process(receiver, packet);
}
} // namespace skeet::events

// ============================================================================
// Subcomponent: event_name_storage.cpp
// ============================================================================

namespace skeet::events {
namespace {
EventNameStartupBindings* active_names = nullptr;
// Constant source operands reconstruct the stack plaintext of43480770. They
// are used only by the real allocating constructors, never as consumer names.
constexpr std::array<const char*, EventNameStorage::count> initial_names{"pre_render",
                                                                         "pre_render_3d",
                                                                         "override_view",
                                                                         "string_cmd",
                                                                         "text",
                                                                         "pre_config_load",
                                                                         "post_config_load",
                                                                         "pre_config_save",
                                                                         "post_config_save",
                                                                         "send_datagram",
                                                                         "send_net_msg",
                                                                         "player_chat",
                                                                         "teamonly",
                                                                         "entity",
                                                                         "name",
                                                                         "server_cvar",
                                                                         "cvarname",
                                                                         "cvarvalue",
                                                                         "invalid event name",
                                                                         "hitgroup",
                                                                         "damage",
                                                                         "hit_chance",
                                                                         "boosted",
                                                                         "high_priority",
                                                                         "aim_fire",
                                                                         "aim_hit",
                                                                         "aim_miss",
                                                                         "console_input",
                                                                         "net_update_start",
                                                                         "net_update_end",
                                                                         "level_init",
                                                                         "run_command",
                                                                         "finish_command",
                                                                         "chokedcommands",
                                                                         "command_number",
                                                                         "post_render",
                                                                         "paint",
                                                                         "paint_ui",
                                                                         "pre_predict_command",
                                                                         "predict_command",
                                                                         "setup_command",
                                                                         "voice"};
std::size_t row_index(EventName name) {
    const auto address = static_cast<std::uint32_t>(name);
    const auto offset = address - EventNameStorage::native_base;
    if ((offset & 15U) || offset >= EventNameStorage::count * 16U)
        throw std::out_of_range("Name is not a row of native43479360 storage");
    return offset / 16U;
}
void finalize_names() {
    active_names->finalize();
}
void initialize_names(void* owner) { //4331EDEC
    auto& source = *static_cast<EventNameStartupBindings*>(owner);
    source.initialize(); //CALL43480770, before the finalizer registration
    (void)source.callbacks.append(source.source_finalizer(0x43419B17u)); //JMP433837F5
}
EventNameStorage::Cells& image_cells(const std::shared_ptr<engine::NativeDataImage>& image) {
    if (!image)
        throw std::invalid_argument("Event names require their native data owner");
    return image->native_cell<EventNameStorage::Cells>(EventNameStorage::native_base);
}
}
EventNameStorage::EventNameStorage(engine::PreservedInitialization& source)
    : EventNameStorage(source.image_owner()) {}
EventNameStorage::EventNameStorage(std::shared_ptr<engine::NativeDataImage> image)
    : image_(std::move(image)), cells(image_cells(image_)) {}
const engine::NativeDataImage& EventNameStorage::image() const {
    if (!image_)
        throw std::logic_error("Snapshot event names have no native data owner");
    return *image_;
}
misc::NativeText& EventNameStorage::at(EventName name) {
    return cells[row_index(name)];
}
const misc::NativeText& EventNameStorage::at(EventName name) const {
    return cells[row_index(name)];
}
StandaloneEventNameStorage::StandaloneEventNameStorage() noexcept : view_(cells_) {
    // Isolated consumer adapters own their mutable backing explicitly. This
    // never runs for the production image view or passes its startup guard.
    for (std::size_t row = 0; row < cells_.size(); ++row) {
        const auto length = std::strlen(initial_names[row]);
        auto* const bytes = backing_[row].data();
        std::memcpy(bytes, initial_names[row], length + 1);
        cells_[row] = {bytes, bytes + length, bytes + length + 1, 0};
    }
}
EventNameStorage& initialize_event_name_storage(EventNameStorage& names, engine::JobMemory memory) {
    //433B2213 clears only begin/end/limit, preserves each fourth word, scans
    // the exact literal and allocates/copies its own writable native backing.
    for (std::size_t row = 0; row < names.cells.size(); ++row)
        misc::construct_native_text(names.cells[row], initial_names[row], memory);
    return names; //4348199A returns the first native row,43479360
}
void finalize_event_name_storage(EventNameStorage& names, engine::JobMemory memory) {
    //4332B67C starts at+290 and ends with a tail call for+0. It neither clears
    // the freed headers nor edits the listener registry or its dirty byte.
    for (std::size_t row = names.cells.size(); row != 0; --row)
        misc::destroy_native_text(names.cells[row - 1], memory);
}
void EventNameStartupBindings::initialize() {
    initialize_event_name_storage(registry.name_storage, memory);
}
void EventNameStartupBindings::finalize() {
    finalize_event_name_storage(registry.name_storage, memory);
}
void (*EventNameStartupBindings::source_finalizer(std::uint32_t address))() {
    if (address != 0x43419B17u)
        throw std::out_of_range("Finalizer is not the native event-name cleanup entry");
    if (active_names && active_names != this)
        throw std::logic_error("Event-name exit callback already has another stable context");
    active_names = this;
    return finalize_names;
}
void bind_event_name_initializer(client::NativeStartupSources& sources,
                                 EventNameStartupBindings& bindings) {
    if (&bindings.registry.name_storage.image() != &sources.data())
        throw std::invalid_argument(
            "Event-name CRT and registry require the same canonical native data owner");
    sources.bind_initializer({0x4331EDECu, &bindings, initialize_names});
}
} // namespace skeet::events

// ============================================================================
// Subcomponent: feature_listeners.cpp
// ============================================================================

namespace skeet::events {
void HitLogListener::dispatch(void* context, GameEventReader& event, EventPlayerIndices& players) {
    auto& self = *static_cast<HitLogListener*>(context);
    self.owner.player_hurt(event, static_cast<HitLogPlayerInfo&>(players), self.output);
}
void HitLogListener::bind(EventRegistry& registry) {
    registry.add({player_hurt_hash, this, dispatch});
}
void HitLogListener::unbind(EventRegistry& registry) {
    registry.remove({player_hurt_hash, this, dispatch});
}
void HitMarkerListener::dispatch(void* context, GameEventReader& event,
                                 EventPlayerIndices& players) {
    auto& self = *static_cast<HitMarkerListener*>(context);
    self.owner.player_hurt(event, self.bindings,
                           static_cast<EngineEventPlayers&>(players).local_entity());
}
void HitMarkerListener::bind(EventRegistry& registry) {
    registry.add({player_hurt_hash, this, dispatch});
}
void HitMarkerListener::unbind(EventRegistry& registry) {
    registry.remove({player_hurt_hash, this, dispatch});
}
void DeathWeaponIconsListener::dispatch(void* context, GameEventReader& event,
                                        EventPlayerIndices& players) {
    auto& self = *static_cast<DeathWeaponIconsListener*>(context);
    auto bindings = self.bindings;
    bindings.local_player = static_cast<EngineEventPlayers&>(players).local_entity();
    self.owner.player_death(static_cast<MutableGameEvent&>(event), bindings, self.offsets);
}
void DeathWeaponIconsListener::bind(EventRegistry& registry) {
    registry.add({player_death_hash, this, dispatch});
}
void DeathWeaponIconsListener::unbind(EventRegistry& registry) {
    registry.remove({player_death_hash, this, dispatch});
}
void StatTrakDeathListener::dispatch(void* context, GameEventReader& event,
                                     EventPlayerIndices& players) {
    auto& self = *static_cast<StatTrakDeathListener*>(context);
    auto bindings = self.bindings;
    bindings.local_player = static_cast<EngineEventPlayers&>(players).local_entity();
    self.owner.player_death(event, bindings, self.offsets);
}
void StatTrakDeathListener::bind(EventRegistry& registry) {
    registry.add({player_death_hash, this, dispatch});
}
void StatTrakDeathListener::unbind(EventRegistry& registry) {
    registry.remove({player_death_hash, this, dispatch});
}
void DormantAlphaDeathListener::dispatch(void* context, GameEventReader& event,
                                         EventPlayerIndices&) {
    auto& self = *static_cast<DormantAlphaDeathListener*>(context);
    death_clear_dormant_alpha(event, self.bindings, self.players);
}
void DormantAlphaDeathListener::bind(EventRegistry& registry) {
    registry.add({player_death_hash, this, dispatch});
}
void DormantAlphaDeathListener::unbind(EventRegistry& registry) {
    registry.remove({player_death_hash, this, dispatch});
}
} // namespace skeet::events

// ============================================================================
// Subcomponent: game_events.cpp
// ============================================================================

namespace skeet::events {
namespace {
constexpr std::uint32_t userid = 0x8361150E, attacker = 0x455B4DBF;
constexpr std::uint32_t weapon = 0xFDF97DB0, damage = 0x32130FCF, hitgroup = 0xB8B40BAC;
constexpr std::uint32_t coordinates[] = {0x6FC60D76, 0x6EC61305, 0x6DC60E5C};
bool active(const GameEventState& state) {
    return state.enabled && state.has_pending_shots;
}
bool contains_fire(const GameEventState& state) {
    for (const auto& event : state.feedback)
        if (event.kind == ShotEventKind::fire)
            return true;
    return false;
}
}

std::uint32_t event_hash(const char* text) noexcept {
    std::uint32_t result = 0x811C9DC5;
    if (text)
        for (auto cursor = reinterpret_cast<const unsigned char*>(text); *cursor; ++cursor)
            result = (result ^ *cursor) * 0x1000193;
    return result ^ 0x92CA5DF1;
}

bool FieldQuery::integer(const char* name, std::uint32_t input) noexcept {
    if (type != FieldType::integer || event_hash(name) != name_hash)
        return true;
    value.integer = input;
    return false;
}
bool FieldQuery::floating(const char* name, float input) noexcept {
    if (type != FieldType::floating || event_hash(name) != name_hash)
        return true;
    value.floating = input;
    return false;
}
bool FieldQuery::string(const char* name, const char* input) noexcept {
    if (type != FieldType::string || event_hash(name) != name_hash)
        return true;
    value.string = input;
    return false;
}
bool FieldQuery::boolean(const char* name, bool input) noexcept {
    if (type != FieldType::boolean || event_hash(name) != name_hash)
        return true;
    value.boolean = input;
    return false;
}

std::uint32_t ShotEvent::payload_words() const noexcept {
    switch (kind) {
    case ShotEventKind::fire:
    case ShotEventKind::death:
        return 1;
    case ShotEventKind::impact:
        return 3;
    case ShotEventKind::hurt:
        return 4;
    }
    return 0;
}

void weapon_fire(GameEventState& state, GameEventReader& event, EventPlayerIndices& players) {
    if (!active(state))
        return;
    FieldValue user, item;
    if (event.read(userid, FieldType::integer, user))
        return;
    if (event.read(weapon, FieldType::string, item))
        return;
    const auto index = players.player_for_user_id(user.integer);
    if (index != players.local_player_index())
        return;
    ShotEvent output;
    output.kind = ShotEventKind::fire;
    output.payload[0] = event_hash(item.string);
    state.feedback.push_back(output);
}

void bullet_impact(GameEventState& state, GameEventReader& event, EventPlayerIndices& players) {
    if (!active(state) || !contains_fire(state))
        return;
    FieldValue user;
    std::array<FieldValue, 3> position;
    if (event.read(userid, FieldType::integer, user))
        return;
    for (unsigned axis = 0; axis != 3; ++axis)
        if (event.read(coordinates[axis], FieldType::floating, position[axis]))
            return;
    const auto index = players.player_for_user_id(user.integer);
    if (index != players.local_player_index())
        return;
    ShotEvent output;
    output.kind = ShotEventKind::impact;
    for (unsigned axis = 0; axis != 3; ++axis)
        output.payload[axis] = std::bit_cast<std::uint32_t>(position[axis].floating);
    state.feedback.push_back(output);
}

void player_hurt(GameEventState& state, GameEventReader& event, EventPlayerIndices& players) {
    if (!active(state) || !contains_fire(state))
        return;
    FieldValue user, source, item, amount, group;
    if (event.read(userid, FieldType::integer, user))
        return;
    if (event.read(attacker, FieldType::integer, source))
        return;
    if (event.read(weapon, FieldType::string, item))
        return;
    if (event.read(damage, FieldType::integer, amount))
        return;
    if (event.read(hitgroup, FieldType::integer, group))
        return;
    const auto victim = players.player_for_user_id(user.integer);
    if (static_cast<std::uint32_t>(victim) - 1 > 63)
        return;
    const auto local = players.local_player_index();
    if (victim == local)
        return;
    if (players.player_for_user_id(source.integer) != local)
        return;
    ShotEvent output;
    output.kind = ShotEventKind::hurt;
    output.payload[0] = static_cast<std::uint32_t>(victim);
    output.payload[1] = group.integer;
    output.payload[2] = amount.integer;
    output.payload[3] = event_hash(item.string);
    state.feedback.push_back(output);
}

void player_death(GameEventState& state, GameEventReader& event, EventPlayerIndices& players) {
    if (!active(state))
        return;
    FieldValue user;
    if (event.read(userid, FieldType::integer, user))
        return;
    ShotEvent output;
    output.kind = ShotEventKind::death;
    output.payload[0] = static_cast<std::uint32_t>(players.player_for_user_id(user.integer));
    state.feedback.push_back(output);
}
} // namespace skeet::events

// ============================================================================
// Subcomponent: hit_log.cpp
// ============================================================================

namespace skeet::events {
std::array<char, 128> player_log_name(std::int32_t index, HitLogPlayerInfo& engine) {
    std::array<char, 128> result{};
    std::array<std::byte, 0x158> info{};
    if (engine.player_info(index, info)) {
        std::memcpy(result.data(), info.data() + 0x10, result.size());
        for (auto& character : result) {
            if (!character)
                break;
            if (character == '\a' || character == '\t' || character == '\n' || character == '\r')
                character = ' ';
        }
        result[20] = 0;
    } else {
        ::sprintf_s(result.data(), result.size(), "%d", index);
    }
    return result;
}
void HitLog::player_hurt(GameEventReader& event, HitLogPlayerInfo& players, misc::NormalLog& log) {
    if (!settings.damage.value)
        return;
    FieldValue attacker, victim, group, damage, health;
    if (event.read(0x455B4DBF, FieldType::integer, attacker))
        return;
    if (event.read(0x8361150E, FieldType::integer, victim))
        return;
    if (event.read(0xB8B40BAC, FieldType::integer, group))
        return;
    if (event.read(0x32130FCF, FieldType::integer, damage))
        return;
    if (event.read(0xF952B07E, FieldType::integer, health))
        return;
    const auto source = players.player_for_user_id(attacker.integer);
    if (source != players.local_player_index())
        return;
    const auto target = players.player_for_user_id(victim.integer);
    if (static_cast<std::uint32_t>(target) - 1 > 63)
        return;
    if (target == players.local_player_index())
        return;
    if (group.integer > 8)
        return;
    const auto name = player_log_name(target, players);
    const auto* const hitgroup = hitgroups[group.integer].begin;
    std::array<char, 54> decoded_format; // native ESP+62..+97
    misc::decode_native_text(format, decoded_format.data());
    log.format(decoded_format.data(), name.data(), hitgroup,
               std::bit_cast<std::int32_t>(damage.integer),
               std::bit_cast<std::int32_t>(health.integer));
}
} // namespace skeet::events

// ============================================================================
// Subcomponent: hit_marker.cpp
// ============================================================================

namespace skeet::events {
void HitMarker::player_hurt(GameEventReader& event, HitMarkerBindings& bindings,
                            void* local_player) {
    if (!settings.sound.value && !settings.enabled.value)
        return;
    FieldValue attacker, victim;
    if (event.read(0x455B4DBF, FieldType::integer, attacker))
        return;
    if (event.read(0x8361150E, FieldType::integer, victim))
        return;
    const auto local_index = engine::invoke<std::int32_t>(bindings.engine, 12);
    if (engine::invoke<std::int32_t>(bindings.engine, 9, attacker.integer) != local_index)
        return;
    const auto target_index = engine::invoke<std::int32_t>(bindings.engine, 9, victim.integer);
    if (static_cast<std::uint32_t>(target_index) - 1 > 63)
        return;
    const auto target = engine::invoke<void*>(bindings.entity_list, 3, target_index);
    if (!target)
        return;
    const auto enemy = bindings.enemy.capture(local_player);
    if (!rage::target_is_enemy(enemy, target))
        return;
    if (settings.sound.value && !bindings.sound_blocked) {
        auto* surface = bindings.surface;
        if (surface) {
            auto* sources = bindings.sound_players;
            if (sources)
                for (std::size_t index = 0; index < 64 && sources[index]; ++index) {
                    if (sources[index] == local_index) {
                        sources[index] = 0x7FFFFFFF;
                        surface = bindings.surface; //native4337F28C reload after matching write
                        break;
                    }
                }
            engine::invoke<void>(surface, 82, sound.begin);
        }
    }
    if (settings.enabled.value)
        remaining = 0.4f;
}
namespace {
std::uint32_t marker_alpha(float remaining) {
    auto ratio = remaining / 0.4f;
    ratio = ratio > 0 ? ratio : 0;
    ratio = ratio < 0.9f ? ratio : 0.9f;
    const auto rounded = ratio * 255.0f + 8388608.0f;
    return std::bit_cast<std::uint32_t>(rounded) & 255;
}
void emit_marker(ui::RenderQueue& queue, std::int32_t width, std::int32_t height, float scale,
                 std::uint32_t alpha) {
    const auto size = _mm_cvtss_si32(_mm_set_ss(scale * 10.0f));
    const auto half = size / 2;
    const auto add = [](std::int32_t a, std::int32_t b) {
        return std::bit_cast<std::int32_t>(std::uint32_t(a) + std::uint32_t(b));
    };
    const auto sub = [](std::int32_t a, std::int32_t b) {
        return std::bit_cast<std::int32_t>(std::uint32_t(a) - std::uint32_t(b));
    };
    const auto left = sub(width / 2, size), right = add(width / 2, size);
    const auto top = sub(height / 2, size), bottom = add(height / 2, size);
    const auto shadow = (alpha & 0xFE) << 23;
    const auto color = (alpha << 24) | 0xFFFFFF;
    queue.line(
        {add(left, 1), add(top, 1), add(add(left, half), 1), add(add(top, half), 1), shadow});
    queue.line(
        {add(left, 1), add(bottom, 1), add(add(left, half), 1), add(sub(bottom, half), 1), shadow});
    queue.line(
        {add(right, 1), add(top, 1), add(sub(right, half), 1), add(add(top, half), 1), shadow});
    queue.line({add(right, 1), add(bottom, 1), add(sub(right, half), 1), add(sub(bottom, half), 1),
                shadow});
    queue.line({left, top, add(left, half), add(top, half), color});
    queue.line({left, bottom, add(left, half), sub(bottom, half), color});
    queue.line({right, top, sub(right, half), add(top, half), color});
    queue.line({right, bottom, sub(right, half), sub(bottom, half), color});
}
}
void HitMarker::draw(ui::RenderQueue& queue, const HitMarkerView& view) {
    if (!settings.enabled.value) {
        remaining = 0;
        return;
    }
    remaining = maximum_number(0.0f, remaining - view.frame_time);
    if (!view.player ||
        *(static_cast<const std::uint8_t*>(view.player) + view.rejection_byte) != 0 ||
        !(remaining > 0))
        return;
    const auto alpha = marker_alpha(remaining);
    if (!alpha) {
        remaining = 0;
        return;
    }
    emit_marker(queue, view.width, view.height, view.font_scale, alpha);
}
void HitMarker::draw_live(ui::RenderQueue& queue, void* player, void* const& globals,
                          const engine::DecodedOffset& rejection_byte) {
    if (!settings.enabled.value) {
        remaining = 0;
        return;
    }
    float frame_time;
    std::memcpy(&frame_time, static_cast<const std::byte*>(globals) + 0x14, sizeof frame_time);
    remaining = maximum_number(0.0f, remaining - frame_time);
    if (!player)
        return;
    if (*(static_cast<const std::uint8_t*>(player) + std::ptrdiff_t(rejection_byte)) != 0 ||
        !(remaining > 0))
        return;
    const auto alpha = marker_alpha(remaining);
    if (!alpha) {
        remaining = 0;
        return;
    }
    const auto screen = queue.screen();
    const auto scale = queue.fonts->scale;
    emit_marker(queue, screen.x, screen.y, scale, alpha);
}
} // namespace skeet::events

// ============================================================================
// Subcomponent: hit_marker_owner.cpp
// ============================================================================

namespace skeet::events {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define HIT_MEMBER __thiscall
#define HIT_FAST __fastcall
#else
#define HIT_MEMBER
#define HIT_FAST
#endif
using OwnerEntry = void(HIT_MEMBER*)(void*);
struct HudEndpoint : hud::HudFrameCallback {
    HitMarkerLifecycleOwner* owner;
};
void HIT_MEMBER destroy_hit_marker(void*);
void HIT_MEMBER reset_hit_marker(void*);
void HIT_MEMBER forward_hit_marker_reset(void*);
void HIT_MEMBER unchanged_hit_marker(void*) {} //433CCE8E RET
void HIT_FAST draw_hit_marker(void*, ui::RenderQueue*, void*, std::int32_t*);
const std::array<OwnerEntry, 9> lifecycle_table{
    destroy_hit_marker,   unchanged_hit_marker,     unchanged_hit_marker,
    unchanged_hit_marker, unchanged_hit_marker,     reset_hit_marker,
    unchanged_hit_marker, forward_hit_marker_reset, unchanged_hit_marker};
const std::array<hud::HudFrameEntry, 1> hud_table{draw_hit_marker};
}

class HitMarkerLifecycleOwner {
public:
    const OwnerEntry* vtable = nullptr;
    HudEndpoint hud_endpoint{{nullptr}, this};
    void* captured_surface; //native+C, written once by43486543..43486549
    engine::JobMemory memory;
    HitMarkerOwnerBindings bindings;
    HitMarker state;
    HitMarkerListener listener;

    HitMarkerLifecycleOwner(engine::OwnerRegistryBinding lifecycle,
                            engine::OwnerRegistryBinding hud_registry, ConfigRegistries& registries,
                            EventRegistry& events, HitMarkerOwnerBindings source)
        : memory(lifecycle.memory), bindings(source),
          state(source.settings, source.remaining, source.sound), listener{state, source.events} {
        lifecycle.append(this); //43486524 ->433676A0, primary still null
        hud::register_hud_frame(hud_registry, &hud_endpoint); //4348652C ->433D20B5
        vtable = lifecycle_table.data(); //4347253C
        hud_endpoint.vtable = hud_table.data(); //4346F6A8
        captured_surface = bindings.events.surface; //mem_int3.h:575,43486543
        esp::initialize_hit_marker_config(registries.descriptors, bindings.settings);
        bindings.remaining = 0.0f; //43486575
        misc::construct_native_text(bindings.sound, "buttons/arena_switch_press_02.wav", memory);
        listener.bind(events); //4348661D ->4336BAAD,89FA8001 /433BFDF6
    }
};

namespace {
void HIT_FAST draw_hit_marker(void* endpoint, ui::RenderQueue* queue, void* player, std::int32_t*) {
    auto& owner = *static_cast<HudEndpoint*>(static_cast<hud::HudFrameCallback*>(endpoint))->owner;
    owner.state.draw_live(*queue, player, owner.bindings.global_variables,
                          owner.bindings.rejection_byte);
}
void HIT_MEMBER reset_hit_marker(void* object) { //4336897C
    static_cast<HitMarkerLifecycleOwner*>(object)->state.reset();
}
void HIT_MEMBER forward_hit_marker_reset(void* object) { //433CF2F5
    const auto* owner = static_cast<HitMarkerLifecycleOwner*>(object);
    owner->vtable[5](object);
}
void HIT_MEMBER destroy_hit_marker(void* object) { //433FA5C7
    if (!object)
        return;
    auto* owner = static_cast<HitMarkerLifecycleOwner*>(object);
    misc::destroy_native_text(owner->bindings.sound, owner->memory); //4336B1E9, first
    // The original primary does not unregister the event/HUD entries. Their
    // common shutdown owns the ranges; no individual listener removal is added.
    const auto memory = owner->memory;
    owner->~HitMarkerLifecycleOwner();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement); //433837D0
}
}
std::size_t hit_marker_owner_size() noexcept {
    return sizeof(HitMarkerLifecycleOwner);
}
HitMarkerLifecycleOwner& construct_hit_marker(void* storage, engine::OwnerRegistryBinding lifecycle,
                                              engine::OwnerRegistryBinding hud_registry,
                                              ConfigRegistries& registries, EventRegistry& events,
                                              HitMarkerOwnerBindings bindings) {
    return *::new (storage)
        HitMarkerLifecycleOwner(lifecycle, hud_registry, registries, events, bindings);
}
HitMarker& hit_marker_runtime(HitMarkerLifecycleOwner& owner) noexcept {
    return owner.state;
}
#undef HIT_MEMBER
#undef HIT_FAST
} // namespace skeet::events

// ============================================================================
// Subcomponent: registry.cpp
// ============================================================================

namespace skeet::events {
namespace {
bool equal(const EventListener& a, const EventListener& b) {
    return a.name_hash == b.name_hash && a.owner == b.owner && a.callback == b.callback;
}
void fire(void* owner, GameEventReader& event, EventPlayerIndices& players) {
    weapon_fire(*static_cast<GameEventState*>(owner), event, players);
}
void hurt(void* owner, GameEventReader& event, EventPlayerIndices& players) {
    player_hurt(*static_cast<GameEventState*>(owner), event, players);
}
void death(void* owner, GameEventReader& event, EventPlayerIndices& players) {
    player_death(*static_cast<GameEventState*>(owner), event, players);
}
}
bool EventRegistry::add(EventListener listener, bool connected) {
    if (std::find_if(listeners.begin(), listeners.end(),
                     [&](const auto& item) { return equal(item, listener); }) != listeners.end())
        return false;
    listeners.push_back(listener);
    if (connected)
        subscriptions_dirty = true;
    return true;
}
bool EventRegistry::remove(EventListener listener, bool connected) {
    auto found = std::find_if(listeners.begin(), listeners.end(),
                              [&](const auto& item) { return equal(item, listener); });
    if (found == listeners.end())
        return false;
    listeners.erase(found);
    if (connected)
        subscriptions_dirty = true;
    return true;
}
bool EventRegistry::add(EventListener listener) {
    if (!add(listener, false))
        return false;
    if (engine::invoke<bool>(engine_interface, 26)) {
        subscriptions_dirty = true;
        *(static_cast<std::uint8_t*>(event_manager) + 0xA0) = 1;
    }
    return true;
}
bool EventRegistry::remove(EventListener listener) {
    if (!remove(listener, false))
        return false;
    if (engine::invoke<bool>(engine_interface, 26)) {
        subscriptions_dirty = true;
        *(static_cast<std::uint8_t*>(event_manager) + 0xA0) = 1;
    }
    return true;
}
void EventRegistry::dispatch(std::uint32_t hash, GameEventReader& event,
                             EventPlayerIndices* player) {
    if (!player)
        return;
    // The native dispatcher captures its end pointer before invoking callbacks.
    const auto count = listeners.size();
    for (std::size_t index = 0; index != count; ++index) {
        auto listener = listeners[index];
        if (listener.name_hash == hash)
            listener.callback(listener.owner, event, *player);
    }
}
bool EventRegistry::contains(std::uint32_t hash) const noexcept {
    for (const auto& listener : listeners)
        if (listener.name_hash == hash)
            return true;
    return false;
}
void bind_shot_events(EventRegistry& registry, GameEventState& state, bool connected) {
    registry.add({weapon_fire_hash, &state, fire}, connected);
    registry.add({player_hurt_hash, &state, hurt}, connected);
    registry.add({player_death_hash, &state, death}, connected);
}
void unbind_shot_events(EventRegistry& registry, GameEventState& state, bool connected) {
    registry.remove({weapon_fire_hash, &state, fire}, connected);
    registry.remove({player_hurt_hash, &state, hurt}, connected);
    registry.remove({player_death_hash, &state, death}, connected);
}
void bind_shot_events(EventRegistry& registry, GameEventState& state) {
    registry.add({weapon_fire_hash, &state, fire});
    registry.add({player_hurt_hash, &state, hurt});
    registry.add({player_death_hash, &state, death});
}
void unbind_shot_events(EventRegistry& registry, GameEventState& state) {
    registry.remove({weapon_fire_hash, &state, fire});
    registry.remove({player_hurt_hash, &state, hurt});
    registry.remove({player_death_hash, &state, death});
}
void dispatch_shot_event(GameEventState& state, std::uint32_t hash, GameEventReader& event,
                         EventPlayerIndices& player) {
    switch (hash) {
    case weapon_fire_hash:
        weapon_fire(state, event, player);
        break;
    case bullet_impact_hash:
        bullet_impact(state, event, player);
        break;
    case player_hurt_hash:
        player_hurt(state, event, player);
        break;
    case player_death_hash:
        player_death(state, event, player);
        break;
    }
}
void enable_event_subscriptions(const EventRegistry& registry,
                                std::span<const std::uint32_t> lua_names,
                                std::span<const SubscriptionDescriptor> descriptors,
                                std::span<std::uint32_t> mask) {
    if (registry.listeners.empty() && lua_names.empty())
        return;
    for (const auto& item : descriptors) {
        if (!item.name)
            continue;
        const auto hash = event_hash(item.name);
        if (!registry.contains(hash) &&
            std::find(lua_names.begin(), lua_names.end(), hash) == lua_names.end())
            continue;
        const auto identifier = static_cast<std::uint32_t>(item.identifier);
        const auto bit = std::uint32_t{1} << (identifier % 32);
        if (!(mask[identifier / 32] & bit))
            mask[identifier / 32] |= bit;
    }
}
void enable_engine_event_subscriptions(const EventRegistry& registry,
                                       std::span<const std::uint32_t> lua_names,
                                       const void* manager, void* mask) {
    if (registry.listeners.empty() && lua_names.empty())
        return;
    const auto* descriptor = field<const std::byte*>(manager, 4);
    const auto* const end = descriptor + field<std::int32_t>(manager, 0x10) * 0x34;
    for (; descriptor != end; descriptor += 0x34) {
        const auto* const names = field<const std::byte*>(manager, 0x78);
        if (!names)
            continue;
        const auto name_index = field<std::int32_t>(descriptor, 4);
        const auto* const name =
            field<const char*>(names, std::ptrdiff_t(name_index) * 0x18 + 0x10);
        if (!name)
            continue;
        const auto hash = event_hash(name);
        if (!registry.contains(hash) &&
            std::find(lua_names.begin(), lua_names.end(), hash) == lua_names.end())
            continue;
        const auto identifier = field<std::int32_t>(descriptor, 0);
        auto* const words = field<std::uint32_t*>(mask, 0xC);
        const auto bit = std::uint32_t{1} << (static_cast<std::uint32_t>(identifier) & 31);
        auto& word = words[identifier / 32];
        if (!(word & bit))
            word |= bit;
    }
}
} // namespace skeet::events

// ============================================================================
// Subcomponent: server_impacts.cpp
// ============================================================================

namespace skeet::events {
void process_server_impact(ServerImpactRuntime& runtime, GameEventReader& event,
                           EventPlayerIndices& players) {
    if (!runtime.settings.bullet_impacts.value || runtime.bindings.suspended)
        return;
    FieldValue user, coordinate;
    if (event.read(0x8361150E, FieldType::integer, user))
        return;
    std::array<float, 3> position;
    constexpr std::array<std::uint32_t, 3> fields{0x6FC60D76, 0x6EC61305, 0x6DC60E5C};
    for (std::size_t axis = 0; axis != position.size(); ++axis) {
        coordinate = FieldValue{};
        if (event.read(fields[axis], FieldType::floating, coordinate))
            return;
        position[axis] = coordinate.floating;
    }
    const auto index = players.player_for_user_id(user.integer);
    if (index != players.local_player_index())
        return;
    const std::array<float, 3> minimum{-1.0f, -1.0f, -1.0f};
    const std::array<float, 3> maximum{1.0f, 1.0f, 1.0f}, rotation{};
    // NativeCVTDQ2PS follows MXCSR rather than a host scalar conversion.
    const auto duration =
        _mm_cvtss_f32(_mm_cvtepi32_ps(_mm_cvtsi32_si128(runtime.settings.impact_duration.value)));
    engine::invoke<void>(runtime.bindings.debug_overlay, 1, &position, &minimum, &maximum,
                         &rotation, std::int32_t(0), std::int32_t(0), std::int32_t(255),
                         std::int32_t(127), duration);
}
} // namespace skeet::events
