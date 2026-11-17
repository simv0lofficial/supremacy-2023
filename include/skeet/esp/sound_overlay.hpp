#pragma once
#include "player_engine.hpp"
#include "frame_event_state.hpp"
#include "skeet/events/registry.hpp"
#include "skeet/movement/collision.hpp"
#include "skeet/players/state.hpp"
#include "skeet/client/entity_update_dispatch.hpp"
#include "skeet/client/entity_observer_registry.hpp"
#include <array>
#include <cstddef>

namespace skeet::esp {

struct ActiveSound {
    Vector3 origin;
    std::array<std::byte, 0x2c> unused_0c;
    std::int32_t source_index; // native sound +38
    std::array<std::byte, 0x20> unused_3c;
};
static_assert(sizeof(ActiveSound) == 0x5c);
struct ActiveSoundNode {
    std::int32_t left, right, parent;
    std::uint32_t unused_0c;
    ActiveSound value;
};
static_assert(sizeof(ActiveSoundNode) == 0x6c);
// Actual 43476420 tree header. The engine owns its nodes and index bounds.
struct ActiveSoundTree {
    std::uint32_t unused_00;
    const ActiveSoundNode* nodes;
    std::uint32_t allocation_count, unused_0c;
    std::int32_t root;
    std::uint32_t count, unused_18;
    std::int32_t last_allocated;
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(offsetof(ActiveSoundTree, nodes) == 4);
static_assert(offsetof(ActiveSoundTree, root) == 0x10);
static_assert(offsetof(ActiveSoundTree, last_allocated) == 0x1c);
#endif

struct SoundOverlayBindings {
    MovementCollisionEngine& collision;
    std::uint8_t* const& trace_flag; // optional *434683D0, nested guards restore it
    void* const& debug_overlay; // optional 43468260
    void* const& temp_entities; // optional 434679EC, BeamRingPoint virtual9
};
struct SoundOverlayInputs {
    const PlayerEspConfig& config;
    const SoundOverlayBindings& bindings;
    const skeet::players::State& player_adjustments; // live4347BE09 + index*32
    const PlayerFrameState& frame; // canonical activation, global4346A65C
};
struct SoundOverlayContext : SoundOverlayInputs {
    SoundOverlayContext(SoundOverlayInputs inputs, EnginePlayers& engine_players,
                        PlayerRuntimeState& player_state, const PickupObservations& observations)
        : SoundOverlayInputs(inputs), players(engine_players), state(player_state),
          pickups(observations) {}
    EnginePlayers& players;
    PlayerRuntimeState& state; // same physical canonical player records
    const PickupObservations& pickups;
};
struct DormantRadarOffsets {
    std::ptrdiff_t zero_based_index; // B6F85C08, int32
    std::ptrdiff_t origin; // 2A671B42, Vector3
};

class SoundOverlay {
public:
    std::int32_t ring_model = -1; // native ESP owner +5C
    std::array<std::uint8_t, 3> listener_enabled{}; // +EEC/+EFC/+F0C

    // Sound-model tail of 433E2C67, including genuine network string tables.
    void initialize_model(void* network_string_tables);
    void initialize_model_table(void* actual_model_precache_table);
    // Complete 43330FEE. All source/owner resolution, admission, ground trace,
    // dormant geometry, ring rate limiting and native rendering stay here.
    void consume(RuntimePlayer* view, const ActiveSound&, const SoundOverlayContext&);
    // 4337F9F6 prefix -> complete 4336B697. First listener follows dormant;
    // the second and third follow the bomb bar. Their native order is kept.
    void synchronize_listeners(bool refresh, const PlayerEspConfig&, const WorldEspConfig&,
                               std::array<events::EventListener, 3>, events::EventRegistry&);
};

// Complete 433F0AEB: source registry traversal in native in-order sequence.
// Borrowing the pointer cell preserves the native reload after each consumer.
void process_sound_overlay(RuntimePlayer* view, SoundOverlay&, const ActiveSoundTree* const&,
                           const SoundOverlayContext&);
// Complete 433E0F98, borrowing the same pickup queue as the event producers.
void expire_pickup_observations(PickupObservations&, std::int32_t engine_frame,
                                float tick_interval) noexcept;

// The real four-slot secondary interface at owner+10 / vtable43472670.
// It registers in43475BA8 and consumes the shared queue at incoming slot3.
// Its complete ESP owner publishes the table after all seven registrations.
class SoundHistoryReceiver : public client::NativeEntityObserver {
public:
    SoundHistoryReceiver(client::EntityObserverRegistry&, PickupObservations&,
                         const void* const& engine_globals, const float* const& interval);
    void process_incoming(); //complete433E0F98; native player/delta are unused
private:
    PickupObservations& observations_;
    const void* const& globals_;
    const float* const& interval_;
};
// Dormant radar-position branch 433FBE76..433FC15D. The complete packet
// endpoint composes this prefix with its following bullet-tracer branch.
void update_dormant_radar(const void* packet, DormantRadarOffsets, const SoundOverlayContext&);

} // namespace skeet::esp
