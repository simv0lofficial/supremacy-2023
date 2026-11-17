#pragma once

// Engine Prediction Subsystem Declarations
#include "skeet/lagcomp/prediction_scope.hpp"
#include "skeet/client/commands.hpp"
#include "skeet/engine/startup_convars.hpp"
#include <cstdint>
#include "skeet/client/entity_observer_registry.hpp"
#include "skeet/engine/owner_registry.hpp"
#include <cstddef>
#include "skeet/rage_geometry.hpp"
#include <array>
#include <cstring>
#include "skeet/events/event_name_storage.hpp"
#include "skeet/prediction/observer_receiver.hpp"
#include "skeet/engine/preserved_initialization.hpp"
#include <optional>
#include <span>


// Declarations from command.hpp

namespace skeet::prediction {

#if defined(_M_IX86) || defined(__i386__)
using SeedGenerator = std::uint32_t(__fastcall*)(std::int32_t);
#else
using SeedGenerator = std::uint32_t (*)(std::int32_t);
#endif
// 433D3457..433D3470: *434679AC receives command +4 in ECX.
// Its engine result is masked to 31 bits and written to command +40.
void update_command_seed(client::Command&, SeedGenerator);

// The native list behind 43467C58 uses next +4 and enabled +8 on x86.
struct PredictionFlagNode {
    PredictionFlagNode* previous;
    PredictionFlagNode* next;
    std::uint8_t enabled;
};
void set_prediction_flags(PredictionFlagNode* first, std::uint8_t value) noexcept;

struct TimeTargets {
    float* frame_time; // 43467B04 +14
    float* current_time; // *43467EB8
    std::uint8_t* in_prediction; // *43469264 +8
};

// Inlined at 433D33CF..433D345A (also 433D321A and 433D57B0).
// The 433B0A4F destructor restores flag, current time, then frame time.
class TimeScope {
public:
    TimeScope(TimeTargets targets, bool paused, float interval, std::int32_t tick_base) noexcept;
    ~TimeScope() {
        restore();
    }
    TimeScope(const TimeScope&) = delete;
    TimeScope& operator=(const TimeScope&) = delete;
    void restore() noexcept;

private:
    TimeTargets targets;
    float frame_time;
    float current_time;
    std::uint8_t in_prediction;
};

struct CommandPredictionConVars {
    engine::NativeConVarReference aaa77b8a;
    engine::NativeConVarReference de58ce34;
};
struct CommandBindings {
    //The native owner reloads these pointer cells between engine methods.
    void* const& prediction; // *43469264: SetupMove +50, FinishMove +54
    void* const& move_helper; // *43468EC8: SetHost +4
    void* const& game_movement; // *4346A7F8: ProcessMovement +4
    void* move_data; // 43478CD8
    PredictionFlagNode** flag_head; // *43467C58, read again on destruction
    lagcomp::PredictionTargets targets;
    // Production captures these two live cells at each433697EE entry. The
    // resulting PredictionScope retains those per-call objects for restore.
    const CommandPredictionConVars* live_convars = nullptr;
};
struct CommandPlayerOffsets {
    std::ptrdiff_t current_command_f58e6118;
    std::ptrdiff_t vector_6610746e;
};
//Production433697EE targets depend on this call's copied command/player.
//The template supplies resolved global/ConVar addresses, never these local
//targets. This also binds prediction+8/+18 and GameMovement+8 directly.
CommandBindings bind_command_prediction(const CommandBindings&, void* const& prediction,
                                        void* player, client::Command& copied_command,
                                        const CommandPlayerOffsets&) noexcept;

// 433D4404..433D4558 runs this sequence with a copied command at stack +340.
// Engine methods are actual vtable boundaries; the return-spoof trampoline
// 433E533F changes their return path, not their arguments or side effects.
// The owner continues weapon handling before 433D46F4 ends these scopes.
class CommandPrediction {
public:
    CommandPrediction(const CommandBindings&, void* player, void* command,
                      std::uint32_t random_seed);
    ~CommandPrediction() {
        restore();
    }
    CommandPrediction(const CommandPrediction&) = delete;
    CommandPrediction& operator=(const CommandPrediction&) = delete;
    void restore() noexcept;

private:
    const CommandBindings& bindings;
    lagcomp::PredictionScope saved;
    bool active = true;
};
} // namespace skeet::prediction

// Declarations from entity_update.hpp

namespace skeet::prediction {
struct EngineEntityUpdateBindings {
    void* const& engine_client; //4346A514
    void* const& entity_list; //4346A890
    void* const& pending_events; //43475A50, engine event queue, first node+0C
    void* const& base_client; //434683E0, ClientClass list virtual8
    std::uint32_t* const& replacement_property; //43469060, optional live pointer
    std::ptrdiff_t entity_property; //decoded2B554AD0
};
//Semantic view of the43472388 receiver. Its+4 engine interface and+8/+C
//filter identities are initialized by the enclosing434849D3 owner. Engine
//virtual59 dispatches FireEvents after clearing delays for these class IDs.
struct EngineEntityUpdateOwner {
    explicit EngineEntityUpdateOwner(const EngineEntityUpdateBindings& input) noexcept
        : bindings(input) {}
    void* source_interface;
    std::uint32_t first_filter_identity, second_filter_identity;
    EngineEntityUpdateBindings bindings;
};
// Registration borrows the physical receiver embedded in the same complete
// owner as the semantic view. Publication remains at434849F2 in that owner.
void register_engine_entity_update_owner(client::EntityObserverRegistry&,
                                         client::NativeEntityObserverReceiver&,
                                         EngineEntityUpdateOwner&);
//Complete4337D273. Native slot3's player/delta arguments are not consumed.
void update_prediction_entities(EngineEntityUpdateOwner&);
std::size_t engine_entity_update_owner_size() noexcept;
//Whole434849D3, reached by factoryJMP4347E364. Native-aligned storage comes
//from43387448; lifecycle slot0 releases it through the matching header.
//Registration order is primary then BA8; the caller supplies cross-owner order.
EngineEntityUpdateOwner& construct_engine_entity_update_owner(void* native_aligned_storage,
                                                              engine::OwnerRegistryBinding primary,
                                                              client::EntityObserverRegistry&,
                                                              const EngineEntityUpdateBindings&);
} //namespace skeet::prediction

// Declarations from history.hpp

namespace skeet::client {
struct EntityCommandOwner;
}
namespace skeet::prediction {
// Exact returned payload of 433952F0. This is a view of the existing 150-slot
// command-history owner, not a second producer or replacement history store.
struct CommandHistoryState {
    std::array<std::byte, 0x10> field_00;
    RageVector recoil_angles; // 10
    RageVector recoil_velocity; // 1C
    std::array<std::byte, 0x30> field_28;
    std::uint32_t weapon_identity; // 58
    std::array<std::byte, 4> field_5c;
    float timestamp; // 60
    std::array<std::byte, 9> field_64;
    std::uint8_t fired; // 6D
    std::array<std::byte, 2> field_6e;
    float shot_time; // 70
    std::array<std::byte, 0x14> field_74;
    std::uint32_t flags; // 88
};
struct CommandHistoryEntry {
    std::int32_t command_number;
    CommandHistoryState state;
};
static_assert(sizeof(CommandHistoryState) == 0x8C);
static_assert(sizeof(CommandHistoryEntry) == 0x90);
static_assert(offsetof(CommandHistoryState, recoil_angles) == 0x10);
static_assert(offsetof(CommandHistoryState, recoil_velocity) == 0x1C);
static_assert(offsetof(CommandHistoryState, weapon_identity) == 0x58);
static_assert(offsetof(CommandHistoryState, timestamp) == 0x60);
static_assert(offsetof(CommandHistoryState, fired) == 0x6D);
static_assert(offsetof(CommandHistoryState, shot_time) == 0x70);
static_assert(offsetof(CommandHistoryState, flags) == 0x88);
struct CommandHistoryView {
    //43475898 contains the canonical source EntityCommandOwner projection.
    //Read its current state.history; the semantic owner has no raw+6C prefix.
    static CommandHistoryView from_owner(const client::EntityCommandOwner* owner) noexcept;
    // Native signed remainder and first-dword equality; actual command
    // numbers select readable slots in their engine-owned storage.
    const CommandHistoryState* find(std::int32_t number) const noexcept {
        if (!owner_present_)
            return nullptr;
        const auto& entry = entries_[number % 150];
        return entry.command_number == number ? &entry.state : nullptr;
    }

private:
    const CommandHistoryEntry* entries_ = nullptr;
    //433952F0 tests the owner, not its allocation. An existing owner with a
    //null ring must not be silently treated as an absent owner.
    bool owner_present_ = false;
};
} // namespace skeet::prediction

// Declarations from observer_receiver.hpp

namespace skeet::client {
struct Command;
}
namespace skeet::prediction {
struct NativePredictionObserverReceiver;
struct PlayerCommandContext;
#if defined(_M_IX86) || defined(__i386__)
using NativePredictionCommandEntry = void(__fastcall*)(NativePredictionObserverReceiver*, void*,
                                                       const client::Command*);
using NativePredictionTickEntry = std::int32_t(__fastcall*)(NativePredictionObserverReceiver*,
                                                            std::int32_t, std::int32_t);
using NativePredictionRejectEntry = void(__fastcall*)(NativePredictionObserverReceiver*, void*,
                                                      const PlayerCommandContext*);
#else
using NativePredictionCommandEntry = void (*)(NativePredictionObserverReceiver*, void*,
                                              const client::Command*);
using NativePredictionTickEntry = std::int32_t (*)(NativePredictionObserverReceiver*, std::int32_t,
                                                   std::int32_t);
using NativePredictionRejectEntry = void (*)(NativePredictionObserverReceiver*, void*,
                                             const PlayerCommandContext*);
#endif
// Every slot consumes one stack word. Slots0/1/3 receive the live player in
// EDX; slot3 receives the context base rather than its Command at+4. Slot2
// receives command number in EDX and returns the next chained tick in EAX.
struct NativePredictionObserverVtable {
    NativePredictionCommandEntry capture;
    NativePredictionCommandEntry finish;
    NativePredictionTickEntry tick;
    NativePredictionRejectEntry rejected;
};
struct NativePredictionObserverReceiver {
    const NativePredictionObserverVtable* vtable = nullptr;
    void* source = nullptr; //source backreference; actual native array stores this endpoint
};
static_assert(offsetof(NativePredictionObserverReceiver, vtable) == 0);
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(NativePredictionObserverVtable) == 0x10);
static_assert(offsetof(NativePredictionObserverVtable, tick) == 8);
static_assert(offsetof(NativePredictionObserverVtable, rejected) == 0xC);
static_assert(sizeof(NativePredictionObserverReceiver) == 8);
#endif
} //namespace skeet::prediction

// Declarations from observer_registry.hpp

namespace skeet {
namespace client {
struct PredictionHistoryOwner;
struct EntityCommandOwner;
}
namespace misc {
class QuickSwitchRuntime;
struct ViewmodelLagState;
struct ViewmodelLagBindings;
}
namespace network {
struct ExploitRuntimeState;
}
namespace script {
struct Bridge;
}
namespace prediction {
struct ObserverRegistry;
struct PlayerCommandContext;
enum class ObserverOwnerId : std::uint32_t {
    prediction_history = 0x43472060u,
    quick_switch = 0x4346FA2Cu,
    exploit = 0x4346F7D0u,
    entity_commands = 0x43470924u,
    script = 0x43472B88u,
    viewmodel_lag = 0x4346F0D4u,
};
struct ObserverTable {
    ObserverOwnerId id;
    std::array<std::uint32_t, 4> methods;
};
std::span<const ObserverTable> native_prediction_observer_tables() noexcept;
std::optional<ObserverOwnerId> identify_prediction_observer(std::uint32_t vtable) noexcept;

// These two views borrow the same source state used by their other native
// interfaces. They do not own another script runtime or viewmodel history.
struct ScriptPredictionObserver {
    ScriptPredictionObserver(script::Bridge&, ObserverRegistry&);
    script::Bridge& bridge;
    events::EventNameRef before_event; // live434795C0 row, 433F48F6
    events::EventNameRef after_event; // live434795D0 row, 433C13C5
    events::EventNameRef command_number_field; // live43479580 row, exactly14bytes
    NativePredictionObserverReceiver receiver{};
};
struct ViewmodelPredictionObserver {
    misc::ViewmodelLagState& state;
    const misc::ViewmodelLagBindings& bindings;
    NativePredictionObserverReceiver receiver{};
};

// Registrar4341CEE4 appends source secondary-interface pointers into actual
//43475AB0/AB4/AB8. Borrowed native data cells are never reset here; no
// semantic vector/variant or second native buffer is constructed.
struct ObserverRegistry {
    ObserverRegistry(engine::PreservedInitialization&, engine::JobMemory);
    ObserverRegistry(std::shared_ptr<engine::NativeDataImage>, engine::JobMemory);
    ObserverRegistry(const ObserverRegistry&) = delete;
    ObserverRegistry& operator=(const ObserverRegistry&) = delete;
    ObserverRegistry(ObserverRegistry&&) = delete;
    ObserverRegistry& operator=(ObserverRegistry&&) = delete;
    void append(client::PredictionHistoryOwner&);
    void append(misc::QuickSwitchRuntime&);
    void append(network::ExploitRuntimeState&);
    void append(client::EntityCommandOwner&);
    void append(ScriptPredictionObserver&);
    void append(ViewmodelPredictionObserver&);
    // Each actual constructor publishes its final table after its remaining
    // registrar calls, at the separately proven native write below.
    void publish(client::PredictionHistoryOwner&) const noexcept; //4347D08D
    void publish(misc::QuickSwitchRuntime&) const noexcept; //4347DC4C
    void publish(network::ExploitRuntimeState&) const noexcept; //4347DF3F
    void publish(client::EntityCommandOwner&) const noexcept; //4347F99E
    void publish(ScriptPredictionObserver&) const noexcept; //4347FFC9
    void publish(ViewmodelPredictionObserver&) const noexcept; //43486CAA
    std::span<void* const> owners() const noexcept {
        return owners_.entries();
    }
    engine::OwnerRegistry& native_storage() const noexcept {
        return owners_;
    }
    const engine::NativeDataImage& image() const noexcept {
        return *image_;
    }

    // Native ECX=registered secondaryreceiver, EDX=player, stack=Command*.
    void capture_before(void* player, const client::Command&) const; // slot0forward
    void finish_prediction(void* player, const client::Command&) const; // slot1reverse
    void finish_forward(void* player, const client::Command&) const; // 4335A298
    // Slot3 receives the context BASE; ordinary slots0/1 receive its Command
    // at+4. Keeping the distinct input avoids treating command_number as tick.
    void rejected(void* player, const PlayerCommandContext&) const; // slot3forward
    // Native ECX=receiver, EDX=commandnumber, stack=incomingtick; each EAX
    // becomes the next owner's incoming tick. All six tables are handled.
    std::int32_t adjust_tick(std::int32_t command_number, std::int32_t tick) const;
    // Sole terminal release is actual433F515B: null-test current begin,
    // free its433837D0 allocation and leave all three cells unchanged.
    // C++ destruction releases no pointer buffer or borrowed source owner.
    void finalize_storage() const;

private:
    std::shared_ptr<engine::NativeDataImage> image_;
    engine::OwnerRegistry& owners_;
    engine::JobMemory memory_;
    void append_receiver(NativePredictionObserverReceiver&, void*);
};
std::int32_t adjust_observer_tickbases(const ObserverRegistry&, const client::Command&,
                                       std::int32_t tick);
} // namespace prediction
} // namespace skeet

// Declarations from player_simulation.hpp

namespace skeet::prediction {
struct ObserverRegistry;
// Inline player+decoded5DF1A4BC command context. Slot3 observers receive this
// base; slots0/1 receive &command. sequence_number at+68 is distinct from the
// embedded command.number at context+8.
struct PlayerCommandContext {
    std::uint8_t needs_processing;
    std::array<std::byte, 3> padding_01;
    client::Command command;
    std::int32_t sequence_number;
};
static_assert(sizeof(PlayerCommandContext) == 0x6C);
static_assert(offsetof(PlayerCommandContext, command) == 4);
static_assert(offsetof(PlayerCommandContext, sequence_number) == 0x68);

struct PlayerSimulationOffsets {
    std::ptrdiff_t command_context; // 5DF1A4BC
    std::ptrdiff_t simulation_enabled; // D070684C, byte
    std::ptrdiff_t rejection_byte; // 9D920E35
    std::ptrdiff_t simulation_tick; // A249061A, dword
    std::ptrdiff_t tick_base; // 3D10B94C
    std::ptrdiff_t attack_guard; // 53E7C7D2, byte
    std::ptrdiff_t recoil_angles; // 1BAE1168
    std::ptrdiff_t scaled_recoil_pitch; // 66146802
};
struct PlayerSimulationBindings {
    void* engine_client; // 4346A514
    void** original_player_table; // player hook metadata[-2]+8
    const std::int32_t* simulation_tick; // *43468D4C
    engine::NativeConVarReference recoil_scale_convar; //live4346794C XOR C429E1BF
    ObserverRegistry& observers; // one actual43475AB0 registry
    PlayerSimulationOffsets offsets;
};
// Complete player PhysicsSimulate hook433EB3F4..433EB6F1. ECX is a nonnull
// player and the original engine method has no stack arguments (slot155).
// Sentinel contexts notify slot3 forward and skip the original method. Other
// accepted contexts run capture forward, tick adjustment forward, original,
// then finish reverse. Recoil scaling belongs only to the rejected gate path.
void simulate_player_command(void* player, const PlayerSimulationBindings&);
// Production entry: resolve the current receiver's installed[-2] metadata at
// each actual original155 call, after any preceding prediction observers.
// The explicit-table fixture entry above retains its separate contract.
void simulate_hooked_player_command(void* player, const PlayerSimulationBindings&);
} // namespace skeet::prediction
