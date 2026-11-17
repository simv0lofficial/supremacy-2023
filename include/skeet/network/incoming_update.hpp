#pragma once
#include "skeet/client/command_pipeline.hpp"
#include "skeet/client/entity_update_dispatch.hpp"
#include "skeet/client/entity_observer_registry.hpp"
#include "skeet/esp/sound_overlay.hpp"
#include "skeet/lagcomp/history.hpp"
#include "skeet/misc/ping_spike_channel.hpp"
#include "skeet/prediction/history.hpp"
#include "skeet/engine/startup_convars.hpp"

namespace skeet::esp {
class EspOwnerRuntime;
}
namespace skeet::network {
// Tail list4347644C. Unlike43476908, +10 is an enable word and a zero
// replacement still reaches the imported InterlockedExchange.
struct IncomingUpdatePatch {
    IncomingUpdatePatch* next;
    volatile std::int32_t* target;
    std::int32_t replacement;
    std::uint32_t field_0c, enabled;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(IncomingUpdatePatch, enabled) == 0x10);
#endif
struct IncomingUpdateGlobals {
    void* const& local_player; //43475808, captured before the sound callbacks
    const esp::ActiveSoundTree* const& sounds; //43476420, reloaded after removal
    void* const& engine_client; //4346A514
    void* const& entity_list; //4346A890
    void* const& engine_globals; //43467B04, receive interval+20
    client::EntityCommandOwner* const& prediction_owner; //43475898, canonical150-entry history
    std::int32_t* const& acknowledged; //43475BA4
    const std::int32_t* const& old_acknowledged; //43475794
    const std::int32_t*& sequence; //434758D8, same pointer slot as SendNetMsg
    const std::int32_t* const& update_clock; //43475A60, first pointed word
    const float* const& interval; //4346A338
    std::int32_t& previous_command; //4346D4B4, reset-1 by433F9270
    const std::int32_t& previous_processed_command; //4346D4B8
    const std::int32_t& transition_status; //4347C9EC
    lagcomp::TickTimingSamples& timing; //43478C70/74/78
    float& interpolation; //43476928
    client::EntityObserverRegistry& receivers; //43475BA8/AC/B0, actual insertion order
    IncomingUpdatePatch*& patches; //4347644C
};
struct IncomingUpdateBindings {
    engine::NativeConVarReference convar_43469238; //^F7E8922F, integer0 admits sound pruning
    engine::NativeConVarReference convar_4346a850; //^478720C2, ordered float0 rejects it
    std::ptrdiff_t rejected_9d920e35;
    std::ptrdiff_t tick_base_3d10b94c;
    client::ConstSentCommandHistoryView sent_commands; //43476414 angles/43476450 records
    const misc::PingSpikeNetworkHistory& latency;
    //434758DC publishes the whole ESP owner, whose sound state/context and
    // EnginePlayers identities are the same ones used by its other endpoints.
    esp::EspOwnerRuntime* const& sound_owner;
    client::EntityUpdateBindings::ExchangeWord exchange;
};

// Complete433F9270. All engine getters and history lookup occur in native
// order; changed is only set by its specific mismatch/status branch.
void reconcile_incoming_acknowledgement(void* player, bool& changed, IncomingUpdateGlobals&,
                                        const IncomingUpdateBindings&);
// Complete433CF7E4..433CFB4A, including original metadata slot6 and actual
// receiver slot3. incoming is the captured acknowledged-command delta.
void process_incoming_update(void* hooked_object, IncomingUpdateGlobals&,
                             const IncomingUpdateBindings&);
struct IncomingUpdateRuntime {
    IncomingUpdateGlobals globals;
    IncomingUpdateBindings bindings;
    void process(void* object) {
        process_incoming_update(object, globals, bindings);
    }
};
void bind_incoming_update_runtime(IncomingUpdateRuntime&) noexcept;
#if defined(_M_IX86) || defined(__i386__)
void __fastcall incoming_update_hook(void* object, void* register_edx);
#else
void incoming_update_hook(void* object, void* register_edx);
#endif
} //namespace skeet::network
