#pragma once
#include "history_receiver.hpp"
#include "skeet/config.hpp"
#include "skeet/engine/frame_update.hpp"
#include "skeet/events/registry.hpp"
#include "skeet/rage/shot_feedback.hpp"
#include <optional>

namespace skeet::client {
struct EntityObserverRegistry;
}
namespace skeet::lagcomp {
class HistoryOwnerRuntime;
struct HistoryOwnerRegistries {
    engine::OwnerRegistryBinding lifecycle; //43475B78
    client::EntityObserverRegistry& entities; //43475BA8
    engine::OwnerRegistryBinding packets; //4347584C, registrar43400957
    engine::OwnerRegistryBinding frame_update; //43475AD8, registrar433FCB12
    ConfigRegistries& configuration;
    events::EventRegistry& events;
};
struct HistoryOwnerFields {
    std::uint32_t packet_player_index; //B6F85C08; zero-based signed integer
    std::uint32_t packet_timing; //F62F1430; two raw dwords
    std::uint32_t local_life_state; //9D920E35, byte
};
struct HistoryOwnerBindings {
    HistoryOwnerRegistries registries;
    // Process bindings are borrowed. The owner makes its lifetime-stable
    // recording/publication bindings with configuration/exponent rebound to
    // its own single storage, retaining the shared cache/pool/job runtime.
    const HistoryRecordingBindings& recording;
    HistoryRecordingScratch* scratch; //explicit fixture input; production is automatic
    BooleanConfig& field_ed048f3d; //fresh canonical descriptor; value survives
    void* const& local_player; //actual43475808, read at each native gate
    engine::NativeConVarReference environment_convar; //4346A850 xor478720C2
    const ShotFeedbackState& feedback; //canonical pending history vector
    const std::int32_t& command_tick; //actual input to4340C3FA
    HistoryOwnerFields fields;
};

// Full4347E5DE owner. Primary4346EFA4 and its three interfaces share this
// one recording worker, configuration and player cache. Scratch is per call.
class HistoryOwnerRuntime {
public:
#if defined(_M_IX86) || defined(__i386__)
    using PrimaryEntry = void(__thiscall*)(void*);
    using PacketEntry = void(__fastcall*)(void*, const void*);
#else
    using PrimaryEntry = void (*)(void*);
    using PacketEntry = void (*)(void*, const void*);
#endif
    const PrimaryEntry* vtable = nullptr;
    HistoryOwnerRuntime(HistoryOwnerRuntime*& active, HistoryOwnerBindings);
    ~HistoryOwnerRuntime(); //4336B1E9 for the owned_crouch string only
    HistoryOwnerRuntime(const HistoryOwnerRuntime&) = delete;
    HistoryOwnerRuntime& operator=(const HistoryOwnerRuntime&) = delete;
    HistoryRecordingRuntime& recording() noexcept {
        return runtime_;
    }
    HistoryRecordingConfiguration& configuration() noexcept {
        return configuration_;
    }
    const float& movement_exponent() const noexcept {
        return movement_exponent_;
    }
    const char* crouch_suffix() const noexcept {
        return crouch_.begin;
    }
    std::int32_t first_activity() const noexcept {
        return first_activity_;
    }
    std::int32_t second_activity() const noexcept {
        return second_activity_;
    }
    void bind_configuration(); //primary1,4347E56C: target-hitbox descriptor
    void observe_packet(const void* packet); //433C00AB
    void update_environment(); //4334832C, frame-update byte is unused
    void on_event(events::GameEventReader&); //433E738A ->433EBBC2
private:
    friend void destroy_history_owner(void*, HistoryOwnerRuntime*&);
    struct PacketEndpoint {
        const PacketEntry* vtable = nullptr;
        HistoryOwnerRuntime* owner;
    };
    struct FrameEndpoint : engine::FrameUpdateEndpoint {
        HistoryOwnerRuntime* owner;
    };
    struct OwnedText {
        char* begin;
        char* end;
        char* limit;
    };
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_HISTORY_MEMBER __thiscall
#define SKEET_HISTORY_FAST __fastcall
#else
#define SKEET_HISTORY_MEMBER
#define SKEET_HISTORY_FAST
#endif
    static void SKEET_HISTORY_MEMBER destroy(void*);
    static void SKEET_HISTORY_MEMBER initialize(void*);
    static void SKEET_HISTORY_MEMBER unchanged(void*);
    static void SKEET_HISTORY_FAST packet(void*, const void*);
    static void SKEET_HISTORY_FAST frame(void*, std::uint8_t);
    static void event(void*, events::GameEventReader&, events::EventPlayerIndices&);
#undef SKEET_HISTORY_MEMBER
#undef SKEET_HISTORY_FAST
    inline static constexpr PrimaryEntry primary_[9]{destroy,   initialize, unchanged,
                                                     unchanged, unchanged,  unchanged,
                                                     unchanged, unchanged,  unchanged};
    inline static constexpr PacketEntry packet_[1]{packet};
    inline static constexpr engine::FrameUpdateEntry frame_[1]{frame};
    HistoryOwnerBindings bindings_;
    HistoryOwnerRuntime** active_;
    HistoryRecordingConfiguration configuration_;
    float movement_exponent_; //native+5C; bitpattern3EDC8172, not event hash
    HistoryVelocityReconstructionBindings physics_;
    HistoryVelocityCaptureBindings velocity_;
    HistoryAnimationPublishBindings publication_;
    HistoryRecordingBindings recording_;
    HistoryRecordingRuntime runtime_;
    std::optional<HistoryRecordingReceiver> receiver_;
    PacketEndpoint packet_endpoint_{nullptr, this};
    FrameEndpoint frame_endpoint_{{nullptr}, this};
    std::int32_t first_activity_, second_activity_;
    OwnedText crouch_;
};

std::size_t history_owner_size() noexcept;
// Constructor entry only: storage carries the real43387448 allocation header
// and is sized by history_owner_size(). This entry leaves active publication
// to its caller; no allocating wrapper, startup position or allocation-failure
// continuation is reconstructed here.
HistoryOwnerRuntime& construct_history_owner(void* storage, HistoryOwnerRuntime*& active,
                                             HistoryOwnerBindings);
//Complete433B5B46. The global is cleared before the null check; only the
//owned string and native allocation are released. Borrowed registries and
//the event triple retain the native common-shutdown ownership.
void destroy_history_owner(void* storage, HistoryOwnerRuntime*& active);
} //namespace skeet::lagcomp
