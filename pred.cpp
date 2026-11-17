// Engine Prediction Subsystem Implementation
#include "skeet/prediction/command.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/prediction/entity_update.hpp"
#include "skeet/engine/netvars.hpp"
#include <array>
#include <cstring>
#include <new>
#include "skeet/prediction/observer_registry.hpp"
#include "skeet/prediction/player_simulation.hpp"
#include "skeet/client/prediction_history_owner.hpp"
#include "skeet/client/entity_command_owner.hpp"
#include "skeet/misc/quick_switch.hpp"
#include "skeet/misc/viewmodel_lag.hpp"
#include "skeet/network/exploit_runtime.hpp"
#include "skeet/script/bridge.hpp"
#include "skeet/engine/unload.hpp"
#include <stdexcept>
#include "skeet/lagcomp/prediction_scope.hpp"
#include <bit>


// ============================================================================
// Subcomponent: command.cpp
// ============================================================================

namespace skeet::prediction {
void update_command_seed(client::Command& command, SeedGenerator generator) {
    command.random_seed = static_cast<std::int32_t>(generator(command.number) & 0x7FFFFFFFu);
}
void set_prediction_flags(PredictionFlagNode* first, std::uint8_t value) noexcept {
    for (auto* node = first; node; node = node->next)
        node->enabled = value;
}

TimeScope::TimeScope(TimeTargets input, bool paused, float interval,
                     std::int32_t tick_base) noexcept
    : targets(input) {
    frame_time = *targets.frame_time;
    *targets.frame_time = paused ? 0.0f : interval;
    current_time = *targets.current_time;
    *targets.current_time = static_cast<float>(tick_base) * interval;
    in_prediction = *targets.in_prediction;
    *targets.in_prediction = 1;
}
void TimeScope::restore() noexcept {
    if (targets.in_prediction) {
        *targets.in_prediction = in_prediction;
        targets.in_prediction = nullptr;
    }
    if (targets.current_time) {
        *targets.current_time = current_time;
        targets.current_time = nullptr;
    }
    if (targets.frame_time) {
        *targets.frame_time = frame_time;
        targets.frame_time = nullptr;
    }
}
CommandBindings bind_command_prediction(const CommandBindings& input, void* const& prediction,
                                        void* player, client::Command& command,
                                        const CommandPlayerOffsets& offsets) noexcept {
    CommandBindings result{prediction,      input.move_helper, input.game_movement, input.move_data,
                           input.flag_head, input.targets,     input.live_convars};
    if (input.live_convars) {
        result.targets.convar_aaa77b8a = input.live_convars->aaa77b8a.get();
        result.targets.convar_de58ce34 = input.live_convars->de58ce34.get();
    }
    result.targets.in_prediction = static_cast<std::uint8_t*>(prediction) + 8;
    result.targets.first_time_predicted = static_cast<std::uint8_t*>(prediction) + 0x18;
    result.targets.command_predicted = &command.predicted;
    result.targets.player_current_command = reinterpret_cast<std::uint32_t*>(
        static_cast<std::byte*>(player) + offsets.current_command_f58e6118);
    result.targets.player_field_6610746e = reinterpret_cast<lagcomp::Vector*>(
        static_cast<std::byte*>(player) + offsets.vector_6610746e);
    result.targets.field_a7f8_08 =
        reinterpret_cast<std::uint32_t*>(static_cast<std::byte*>(result.game_movement) + 8);
    return result;
}

CommandPrediction::CommandPrediction(const CommandBindings& input, void* player, void* command,
                                     std::uint32_t random_seed)
    : bindings(input),
      saved(input.targets, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(player)),
            static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(command)), random_seed) {
    if (bindings.flag_head)
        set_prediction_flags(*bindings.flag_head, 1);
    engine::invoke<void>(bindings.move_helper, 1, player);
    engine::invoke<void>(bindings.prediction, 20, player, command, bindings.move_helper,
                         bindings.move_data);
    engine::invoke<void>(bindings.game_movement, 1, player, bindings.move_data);
    engine::invoke<void>(bindings.prediction, 21, player, command, bindings.move_data);
    engine::invoke<void>(bindings.move_helper, 1, static_cast<void*>(nullptr));
    engine::invoke<void>(player, 340);
}
void CommandPrediction::restore() noexcept {
    if (!active)
        return;
    if (bindings.flag_head)
        set_prediction_flags(*bindings.flag_head, 0);
    saved.restore();
    active = false;
}
} // namespace skeet::prediction

// ============================================================================
// Subcomponent: entity_update.cpp
// ============================================================================

namespace skeet::prediction {
namespace {
template <class T> T field(const void* object, std::ptrdiff_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
template <class T> void put(void* object, std::ptrdiff_t offset, const T& value) noexcept {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof value);
}
#if defined(_M_IX86) || defined(__i386__)
using LifecycleEntry = void(__thiscall*)(void*);
#define ENTITY_UPDATE_MEMBER __thiscall
#else
using LifecycleEntry = void (*)(void*);
#define ENTITY_UPDATE_MEMBER
#endif
void ENTITY_UPDATE_MEMBER destroy_owner(void*);
void ENTITY_UPDATE_MEMBER unchanged_owner(void*) {} //433CCE8E
const std::array<LifecycleEntry, 9> lifecycle{
    destroy_owner,   unchanged_owner, unchanged_owner, unchanged_owner, unchanged_owner,
    unchanged_owner, unchanged_owner, unchanged_owner, unchanged_owner}; //4346F358
struct EntityUpdateInstance {
    const LifecycleEntry* vtable = nullptr;
    client::NativeEntityObserverReceiver entity_receiver;
    EngineEntityUpdateOwner observer;
    engine::JobMemory memory;
    EntityUpdateInstance(engine::OwnerRegistryBinding primary,
                         client::EntityObserverRegistry& entities,
                         const EngineEntityUpdateBindings& bindings)
        : observer(bindings), memory(primary.memory) {
        primary.append(this);
        register_engine_entity_update_owner(entities, entity_receiver, observer); //434849E2
        vtable = lifecycle.data(); //434849E7
        entities.publish_prediction_update(entity_receiver); //434849F2, table43472388
        //The five masked constructor bytes are proven A1 14 A5 46 43.
        observer.source_interface = bindings.engine_client;
        observer.first_filter_identity = std::uint32_t(reinterpret_cast<std::uintptr_t>(
            engine::find_client_class(bindings.base_client, 0xD929B4F3u)));
        observer.second_filter_identity = std::uint32_t(reinterpret_cast<std::uintptr_t>(
            engine::find_client_class(bindings.base_client, 0x847CC977u)));
    }
};
void ENTITY_UPDATE_MEMBER destroy_owner(void* object) { //433B1A7A→433837D0
    if (!object)
        return;
    auto* owner = static_cast<EntityUpdateInstance*>(object);
    const auto memory = owner->memory;
    owner->~EntityUpdateInstance();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
#undef ENTITY_UPDATE_MEMBER
}
void register_engine_entity_update_owner(client::EntityObserverRegistry& registry,
                                         client::NativeEntityObserverReceiver& receiver,
                                         EngineEntityUpdateOwner& owner) {
    registry.append_prediction_update(receiver, owner);
}
void update_prediction_entities(EngineEntityUpdateOwner& owner) {
    const auto& b = owner.bindings;
    if (engine::invoke<bool>(b.engine_client, 82))
        return;
    if (engine::invoke<bool>(b.engine_client, 93))
        return;
    auto* node = field<void*>(b.pending_events, 0xC);
    while (node) {
        auto* next = field<void*>(node, 0x38);
        const auto identity = field<std::uint32_t>(node, 0xC);
        if (identity == owner.first_filter_identity || identity == owner.second_filter_identity)
            put(node, 4, std::uint32_t(0));
        node = next;
    }
    //Native does not clear this scratch. Each admitted live player supplies
    //its saved word; the same engine entity-validity contract applies at restore.
    std::array<std::uint32_t, 64> saved;
    if (b.replacement_property) {
        for (std::int32_t index = 1; index <= 64; ++index) {
            auto* player = engine::invoke<void*>(b.entity_list, 3, index);
            if (player && engine::invoke<bool>(player, 158)) {
                saved[std::uint32_t(index) & 63u] = field<std::uint32_t>(player, b.entity_property);
                put(player, b.entity_property, *b.replacement_property);
            }
        }
    }
    engine::invoke<void>(owner.source_interface, 59);
    //Read the optional pointer again after the actual engine method. Unlike
    //the CreateMove override, this native loop restores saved zero words too.
    if (b.replacement_property) {
        for (std::int32_t index = 1; index <= 64; ++index) {
            auto* player = engine::invoke<void*>(b.entity_list, 3, index);
            if (player && engine::invoke<bool>(player, 158))
                put(player, b.entity_property, saved[std::uint32_t(index) & 63u]);
        }
    }
}
std::size_t engine_entity_update_owner_size() noexcept {
    return sizeof(EntityUpdateInstance);
}
EngineEntityUpdateOwner&
construct_engine_entity_update_owner(void* storage, engine::OwnerRegistryBinding primary,
                                     client::EntityObserverRegistry& entities,
                                     const EngineEntityUpdateBindings& bindings) {
    return (::new (storage) EntityUpdateInstance(primary, entities, bindings))->observer;
}
} //namespace skeet::prediction

// ============================================================================
// Subcomponent: observer_registry.cpp
// ============================================================================

#if defined(_M_IX86) || defined(__i386__)
#define PREDICTION_OBSERVER_FAST __fastcall
#else
#define PREDICTION_OBSERVER_FAST
#endif

namespace skeet::prediction {
namespace {
constexpr std::array<ObserverTable, 6> tables{{
    {ObserverOwnerId::prediction_history, {0x4332C3C9u, 0x4340B612u, 0x433E9F3Bu, 0x433C95B7u}},
    {ObserverOwnerId::quick_switch, {0x4341F4A3u, 0x43400E0Du, 0x433E9F3Bu, 0x433C95B7u}},
    {ObserverOwnerId::exploit, {0x433C95B7u, 0x433C95B7u, 0x43328C18u, 0x433C95B7u}},
    {ObserverOwnerId::entity_commands, {0x4332BBA4u, 0x43382703u, 0x433E9F3Bu, 0x433B265Cu}},
    {ObserverOwnerId::script, {0x433F48F6u, 0x433C13C5u, 0x433E9F3Bu, 0x433C95B7u}},
    {ObserverOwnerId::viewmodel_lag, {0x4332EFD2u, 0x43367D42u, 0x433E9F3Bu, 0x4332B1EBu}},
}};
void PREDICTION_OBSERVER_FAST empty_command(NativePredictionObserverReceiver*, void*,
                                            const client::Command*) {} //433C95B7
void PREDICTION_OBSERVER_FAST empty_rejected(NativePredictionObserverReceiver*, void*,
                                             const PlayerCommandContext*) {} //433C95B7
std::int32_t PREDICTION_OBSERVER_FAST unchanged_tick(NativePredictionObserverReceiver*,
                                                     std::int32_t, std::int32_t tick) {
    return tick;
} //433E9F3B
void PREDICTION_OBSERVER_FAST capture_history(NativePredictionObserverReceiver* receiver,
                                              void* player, const client::Command* command) {
    client::begin_prediction_history(
        *static_cast<client::PredictionHistoryOwner*>(receiver->source), player, *command);
}
void PREDICTION_OBSERVER_FAST finish_history(NativePredictionObserverReceiver* receiver,
                                             void* player, const client::Command* command) {
    client::capture_prediction_history(
        *static_cast<client::PredictionHistoryOwner*>(receiver->source), player, *command);
}
void PREDICTION_OBSERVER_FAST capture_quick_switch(NativePredictionObserverReceiver* receiver,
                                                   void* player, const client::Command* command) {
    static_cast<misc::QuickSwitchRuntime*>(receiver->source)
        ->capture_before_prediction(player, *command);
}
void PREDICTION_OBSERVER_FAST finish_quick_switch(NativePredictionObserverReceiver* receiver,
                                                  void* player, const client::Command* command) {
    static_cast<misc::QuickSwitchRuntime*>(receiver->source)->finish_prediction(player, *command);
}
std::int32_t PREDICTION_OBSERVER_FAST exploit_tick(NativePredictionObserverReceiver* receiver,
                                                   std::int32_t number, std::int32_t tick) {
    //43328C18 reads the canonical full-owner+200 command/tick producer.
    const auto& stored =
        static_cast<network::ExploitRuntimeState*>(receiver->source)->command_ticks[number % 150];
    return stored.command == number ? stored.tick : tick;
}
void PREDICTION_OBSERVER_FAST capture_entity_commands(NativePredictionObserverReceiver* receiver,
                                                      void* player,
                                                      const client::Command* command) {
    client::begin_entity_prediction(*static_cast<client::EntityCommandOwner*>(receiver->source),
                                    player, *command);
}
void PREDICTION_OBSERVER_FAST finish_entity_commands(NativePredictionObserverReceiver* receiver,
                                                     void* player, const client::Command* command) {
    client::finish_entity_prediction(*static_cast<client::EntityCommandOwner*>(receiver->source),
                                     player, *command);
}
void PREDICTION_OBSERVER_FAST reject_entity_commands(NativePredictionObserverReceiver* receiver,
                                                     void* player,
                                                     const PlayerCommandContext* context) {
    client::reject_entity_prediction(*static_cast<client::EntityCommandOwner*>(receiver->source),
                                     player, *context);
}
void PREDICTION_OBSERVER_FAST capture_script(NativePredictionObserverReceiver* receiver, void*,
                                             const client::Command* command) {
    auto& owner = *static_cast<ScriptPredictionObserver*>(receiver->source);
    if (owner.bridge.state) {
        const auto* event = owner.before_event.get();
        const auto number = command->number;
        script::dispatch_integer_event(owner.bridge, event, owner.command_number_field, number);
    }
}
void PREDICTION_OBSERVER_FAST finish_script(NativePredictionObserverReceiver* receiver, void*,
                                            const client::Command* command) {
    auto& owner = *static_cast<ScriptPredictionObserver*>(receiver->source);
    if (owner.bridge.state) {
        const auto* event = owner.after_event.get();
        const auto number = command->number;
        script::dispatch_integer_event(owner.bridge, event, owner.command_number_field, number);
    }
}
void PREDICTION_OBSERVER_FAST capture_viewmodel(NativePredictionObserverReceiver* receiver,
                                                void* player, const client::Command* command) {
    auto& owner = *static_cast<ViewmodelPredictionObserver*>(receiver->source);
    misc::prepare_viewmodel_prediction(owner.state, owner.bindings, player, *command);
}
void PREDICTION_OBSERVER_FAST finish_viewmodel(NativePredictionObserverReceiver* receiver,
                                               void* player, const client::Command* command) {
    auto& owner = *static_cast<ViewmodelPredictionObserver*>(receiver->source);
    misc::capture_viewmodel_prediction(owner.state, owner.bindings, player, *command);
}
void PREDICTION_OBSERVER_FAST reject_viewmodel(NativePredictionObserverReceiver* receiver, void*,
                                               const PlayerCommandContext* context) {
    auto& owner = *static_cast<ViewmodelPredictionObserver*>(receiver->source);
    misc::copy_previous_viewmodel_snapshot(owner.state, owner.bindings, context->command.number);
}
const NativePredictionObserverVtable history_entries{capture_history, finish_history,
                                                     unchanged_tick, empty_rejected};
const NativePredictionObserverVtable quick_switch_entries{capture_quick_switch, finish_quick_switch,
                                                          unchanged_tick, empty_rejected};
const NativePredictionObserverVtable exploit_entries{empty_command, empty_command, exploit_tick,
                                                     empty_rejected};
const NativePredictionObserverVtable entity_entries{capture_entity_commands, finish_entity_commands,
                                                    unchanged_tick, reject_entity_commands};
const NativePredictionObserverVtable script_entries{capture_script, finish_script, unchanged_tick,
                                                    empty_rejected};
const NativePredictionObserverVtable viewmodel_entries{capture_viewmodel, finish_viewmodel,
                                                       unchanged_tick, reject_viewmodel};

NativePredictionObserverReceiver& source_receiver(void* endpoint) {
    auto* receiver = static_cast<NativePredictionObserverReceiver*>(endpoint);
    if (!receiver ||
        (receiver->vtable != &history_entries && receiver->vtable != &quick_switch_entries &&
         receiver->vtable != &exploit_entries && receiver->vtable != &entity_entries &&
         receiver->vtable != &script_entries && receiver->vtable != &viewmodel_entries))
        throw std::logic_error("Native prediction entry has no concrete source receiver binding");
    return *receiver;
}
}
std::span<const ObserverTable> native_prediction_observer_tables() noexcept {
    return tables;
}
std::optional<ObserverOwnerId> identify_prediction_observer(std::uint32_t vtable) noexcept {
    for (const auto& table : tables)
        if (std::uint32_t(table.id) == vtable)
            return table.id;
    return std::nullopt;
}
ScriptPredictionObserver::ScriptPredictionObserver(script::Bridge& source,
                                                   ObserverRegistry& registry)
    : bridge(source), before_event(source.name(events::EventName::pre_predict_command)),
      after_event(source.name(events::EventName::predict_command)),
      command_number_field(source.name(events::EventName::command_number)) {
    registry.append(*this); //4347FF8A; table publication remains4347FFC9
}
ObserverRegistry::ObserverRegistry(engine::PreservedInitialization& initialization,
                                   engine::JobMemory memory)
    : ObserverRegistry(initialization.image_owner(), memory) {}
ObserverRegistry::ObserverRegistry(std::shared_ptr<engine::NativeDataImage> image,
                                   engine::JobMemory memory)
    : image_(image),
      owners_(engine::require_native_data_image(image_).native_cell<engine::OwnerRegistry>(
          0x43475AB0u)),
      memory_(memory) {
    if (!memory_.allocate || !memory_.free || !memory_.process_heap)
        throw std::invalid_argument(
            "Native prediction storage requires the canonical process heap");
}
void ObserverRegistry::append_receiver(NativePredictionObserverReceiver& receiver, void* source) {
    receiver.source = source;
    engine::register_owner(owners_, &receiver, memory_); //exact4341CEE4
}
void ObserverRegistry::append(client::PredictionHistoryOwner& owner) {
    append_receiver(owner.prediction_receiver, &owner);
}
void ObserverRegistry::append(misc::QuickSwitchRuntime& owner) {
    append_receiver(owner.prediction_receiver, &owner);
}
void ObserverRegistry::append(network::ExploitRuntimeState& owner) {
    append_receiver(owner.prediction_receiver, &owner);
}
void ObserverRegistry::append(client::EntityCommandOwner& owner) {
    append_receiver(owner.prediction_receiver, &owner);
}
void ObserverRegistry::append(ScriptPredictionObserver& owner) {
    append_receiver(owner.receiver, &owner);
}
void ObserverRegistry::append(ViewmodelPredictionObserver& owner) {
    append_receiver(owner.receiver, &owner);
}
void ObserverRegistry::publish(client::PredictionHistoryOwner& owner) const noexcept {
    owner.prediction_receiver.vtable = &history_entries;
}
void ObserverRegistry::publish(misc::QuickSwitchRuntime& owner) const noexcept {
    owner.prediction_receiver.vtable = &quick_switch_entries;
}
void ObserverRegistry::publish(network::ExploitRuntimeState& owner) const noexcept {
    owner.prediction_receiver.vtable = &exploit_entries;
}
void ObserverRegistry::publish(client::EntityCommandOwner& owner) const noexcept {
    owner.prediction_receiver.vtable = &entity_entries;
}
void ObserverRegistry::publish(ScriptPredictionObserver& owner) const noexcept {
    owner.receiver.vtable = &script_entries;
}
void ObserverRegistry::publish(ViewmodelPredictionObserver& owner) const noexcept {
    owner.receiver.vtable = &viewmodel_entries;
}

void ObserverRegistry::capture_before(void* player, const client::Command& command) const {
    //433EB55D..56B captures begin before end for ordinary slot0.
    auto** cursor = owners_.begin;
    auto** const end = owners_.end;
    for (; cursor != end; ++cursor) {
        auto& receiver = source_receiver(*cursor);
        receiver.vtable->capture(&receiver, player, &command);
    }
}
void ObserverRegistry::finish_prediction(void* player, const client::Command& command) const {
    //433EB643/649 captures begin before end and decrements before slot1.
    auto** const begin = owners_.begin;
    auto** cursor = owners_.end;
    while (cursor != begin) {
        auto& receiver = source_receiver(*--cursor);
        receiver.vtable->finish(&receiver, player, &command);
    }
}
void ObserverRegistry::finish_forward(void* player, const client::Command& command) const {
    //4335A283/289 takes a fresh end-before-begin snapshot for this caller.
    auto** const end = owners_.end;
    auto** cursor = owners_.begin;
    for (; cursor != end; ++cursor) {
        auto& receiver = source_receiver(*cursor);
        receiver.vtable->finish(&receiver, player, &command);
    }
}
void ObserverRegistry::rejected(void* player, const PlayerCommandContext& context) const {
    //433EB4F4/4FA: end then begin; pass the context BASE to slot3.
    auto** const end = owners_.end;
    auto** cursor = owners_.begin;
    for (; cursor != end; ++cursor) {
        auto& receiver = source_receiver(*cursor);
        receiver.vtable->rejected(&receiver, player, &context);
    }
}
std::int32_t ObserverRegistry::adjust_tick(std::int32_t number, std::int32_t tick) const {
    //Every proven slot2 caller captures end then begin and chains EAX.
    auto** const end = owners_.end;
    auto** cursor = owners_.begin;
    for (; cursor != end; ++cursor) {
        auto& receiver = source_receiver(*cursor);
        tick = receiver.vtable->tick(&receiver, number, tick);
    }
    return tick;
}
void ObserverRegistry::finalize_storage() const {
    engine::UnloadSystemFunctions memory;
    memory.free_heap = memory_.free;
    memory.process_heap = memory_.process_heap;
    engine::free_native_allocation(owners_.begin, memory); //complete433F515B; no cell stores
}
std::int32_t adjust_observer_tickbases(const ObserverRegistry& registry,
                                       const client::Command& command, std::int32_t tick) {
    return registry.adjust_tick(command.number, tick);
}
} //namespace skeet::prediction

#undef PREDICTION_OBSERVER_FAST

// ============================================================================
// Subcomponent: player_simulation.cpp
// ============================================================================

namespace skeet::prediction {
namespace {
template <class T>
[[gnu::always_inline]] inline T read(const void* object, std::ptrdiff_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
template <class T> void write(void* object, std::ptrdiff_t offset, const T& value) noexcept {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof value);
}
[[gnu::always_inline]] inline float multiply(float first, float second) noexcept {
    __asm__ __volatile__("mulss %1, %0" : "+x"(first) : "x"(second));
    return first;
}
template <bool Installed>
void original_simulation(void* player, const PlayerSimulationBindings& bindings) {
    void** table;
    if constexpr (Installed) {
        const auto installed = *static_cast<void***>(player);
        const auto* metadata = static_cast<const engine::VtableHookMetadata*>(installed[-2]);
        table = metadata->original_table;
    } else {
        table = bindings.original_player_table;
    }
    reinterpret_cast<engine::MemberFunction<void>>(table[155])(player);
}
template <bool Installed>
void simulate_player_command_impl(void* player, const PlayerSimulationBindings& bindings) {
    const auto& o = bindings.offsets;
    auto& context = *reinterpret_cast<PlayerCommandContext*>(static_cast<std::byte*>(player) +
                                                             o.command_context);
    if (!read<std::uint8_t>(player, o.simulation_enabled) || !context.needs_processing ||
        read<std::uint8_t>(player, o.rejection_byte) ||
        *bindings.simulation_tick == read<std::int32_t>(player, o.simulation_tick) ||
        engine::invoke<bool>(bindings.engine_client, 82) ||
        engine::invoke<bool>(bindings.engine_client, 93)) {
        original_simulation<Installed>(player, bindings);
        const auto scale =
            std::bit_cast<float>(lagcomp::read_prediction_convar(bindings.recoil_scale_convar));
        write(player, o.scaled_recoil_pitch, multiply(read<float>(player, o.recoil_angles), scale));
        return;
    }
    if (context.command.tick == 0x7F7FFFFF) {
        bindings.observers.rejected(player, context);
        write(player, o.simulation_tick, *bindings.simulation_tick);
        context.needs_processing = 0;
        return;
    }
    bindings.observers.capture_before(player, context.command);
    if (!bindings.observers.owners().empty()) {
        const auto tick = bindings.observers.adjust_tick(context.sequence_number,
                                                         read<std::int32_t>(player, o.tick_base));
        write(player, o.tick_base, tick);
    }
    std::uint8_t* attack_guard = nullptr;
    std::uint8_t saved_guard;
    if (context.command.movement.buttons & 0x801u) {
        attack_guard =
            reinterpret_cast<std::uint8_t*>(static_cast<std::byte*>(player) + o.attack_guard);
        saved_guard = *attack_guard;
        *attack_guard = 0;
    }
    original_simulation<Installed>(player, bindings);
    if (attack_guard)
        *attack_guard = saved_guard;
    bindings.observers.finish_prediction(player, context.command);
}
}
void simulate_player_command(void* player, const PlayerSimulationBindings& bindings) {
    simulate_player_command_impl<false>(player, bindings);
}
void simulate_hooked_player_command(void* player, const PlayerSimulationBindings& bindings) {
    simulate_player_command_impl<true>(player, bindings);
}
} // namespace skeet::prediction
