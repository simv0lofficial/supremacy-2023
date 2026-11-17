#pragma once
#include "frame_handler.hpp"
#include "frame_events.hpp"
#include "line_of_sight.hpp"
#include "map_entity_parser.hpp"
#include "packet_tracers.hpp"
#include "player_engine.hpp"
#include "world_engine.hpp"
#include "tablet_upgrade.hpp"
#include "skeet/events/feature_listeners.hpp"
#include <optional>
#include <string>

namespace skeet::esp {
// The linked class records returned by native43467470 virtual10. Class IDs
// are the traversal counter, not a field read from these records.
struct EspClientClass {
    const char* name;
    void* unused_4;
    EspClientClass* next;
};
struct EspOwnerRegistries {
    engine::OwnerRegistryBinding owners; //43475B78
    engine::OwnerRegistryBinding hud; //43475A9C
    client::EntityObserverRegistry& histories; //43475BA8
    engine::OwnerRegistryBinding updates; //43475988
    engine::OwnerRegistryBinding packets; //4347584C
    engine::OwnerRegistryBinding frame_update; //43475AD8
    engine::OwnerRegistryBinding map_parser; //43475770
    events::EventRegistry& events;
    ConfigRegistries& configuration;
};
struct EspOwnerBindings {
    // All referenced contexts/cells are canonical and outlive this owner.
    EspOwnerRegistries registries;
    EspFrameOwner& state;
    PlayerEspConfig& player;
    WorldEspConfig& world;
    EffectsSettings& effects;
    EspFrameInputs frame;
    PlayerEngineBindings player_engine;
    WorldEngineBindings world_engine;
    SoundOverlayInputs sound;
    const TabletUpgradeBindings& tablet;
    skeet::effects::GlowRuntime& glow;
    LineOfSightInputs line_of_sight;
    const PacketTracerBindings& packet;
    events::DeathBindings& death;
    const void* const& engine_globals;
    const float* const& tick_interval;
    std::int32_t* const& event_frame;
    void* const& network_string_tables; //43467198
    void* const& client_classes; //actual43467470, reloaded after event registration
    void* const& beams; //actual4346A898, captured by this constructor
};

// Complete source owner43482632, primary vtable434725A0. Every secondary
// endpoint borrows state; none allocates a second ESP/player/sound cache.
class EspOwnerRuntime {
public:
#if defined(_M_IX86) || defined(__i386__)
    using PrimaryEntry = void(__thiscall*)(void*);
#else
    using PrimaryEntry = void (*)(void*);
#endif
    const PrimaryEntry* vtable = nullptr;
    EspOwnerRuntime(EspOwnerBindings, EspOwnerRuntime*& active);
    ~EspOwnerRuntime(); //4338580A; does not invent registry/config unlinking
    EspOwnerRuntime(const EspOwnerRuntime&) = delete;
    EspOwnerRuntime& operator=(const EspOwnerRuntime&) = delete;
    EspFrameOwner& state() const noexcept {
        return bindings_.state;
    }
    const PlayerEspConfig& player_config() const noexcept {
        return bindings_.player;
    }
    const std::int32_t* override_fov() const noexcept {
        return override_fov_;
    }
    PacketTracerState& tracers() noexcept {
        return tracers_;
    }
    EnginePlayers& player_engine() noexcept {
        return players_;
    }
    WorldEntityEngine& world_engine() noexcept {
        return world_;
    }
    const EspFrameContext& frame_context() const noexcept {
        return frame_;
    }
    const SoundOverlayContext& sound_context() const noexcept {
        return sound_;
    }
    void bind_configuration(); //43485C12
    void reset(); //43343465
    void initialize_models(); //433E2C67
private:
    friend void destroy_esp_owner(EspOwnerRuntime*&, EspOwnerRuntime*);
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_ESP_PRIMARY __thiscall
#else
#define SKEET_ESP_PRIMARY
#endif
    static void SKEET_ESP_PRIMARY destroy(void*);
    static void SKEET_ESP_PRIMARY configuration(void*);
    static void SKEET_ESP_PRIMARY reset_entry(void*);
    static void SKEET_ESP_PRIMARY models(void*);
    static void SKEET_ESP_PRIMARY unchanged(void*);
    inline static constexpr PrimaryEntry entries_[9]{destroy,   configuration, unchanged,
                                                     unchanged, unchanged,     reset_entry,
                                                     unchanged, models,        unchanged};
#undef SKEET_ESP_PRIMARY
    void initialize_configuration();
    void initialize_state();
    void initialize_classes();
    EspOwnerBindings bindings_;
    EspOwnerRuntime** active_;
    EnginePlayers players_;
    WorldEntityEngine world_;
    EspFrameContext frame_;
    SoundOverlayContext sound_;
    LineOfSightContext line_of_sight_;
    const std::int32_t* override_fov_ = nullptr; //+6D4, canonical MISC FC225A4F
    IntegerConfig field_8ce3b4b2_; //native descriptor+0x17C, value+0x18C
    ColoredWorldItem field_5aadd6ed_,
        field_b1054809_; //constructed but not exposed by the native menu
    PacketTracerState tracers_;
    EspFrameEvents events_;
    events::DormantAlphaDeathListener death_;
    // Optional storage postpones construction until each actual registration
    // point. These are concrete endpoints with their recovered method bodies.
    std::optional<EspFrameHandler> hud_;
    std::optional<SoundHistoryReceiver> histories_;
    std::optional<PlayerEspUpdateHandler> updates_;
    std::optional<PacketTracerHandler> packets_;
    std::optional<LineOfSightHandler> sight_;
    std::optional<EspMapEntityParser> parser_;
    std::vector<EntityOverlayHandler> class_handlers_;
    std::array<std::u16string, 20> item_labels_;
    std::array<std::u16string, 7> labels_;
    std::string radius_prefix_;
};
// Native43481EAB allocates the0xF90 native representation and publishes the
// canonical434758DC owner. This source representation has host-sized fields.
void initialize_esp_owner(EspOwnerRuntime*& active, EspOwnerBindings bindings);
// Complete43331A5C, including clearing the canonical slot when object is null.
void destroy_esp_owner(EspOwnerRuntime*& active, EspOwnerRuntime* object);
} //namespace skeet::esp
