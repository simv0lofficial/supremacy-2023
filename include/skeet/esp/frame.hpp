#pragma once
#include "player_draw.hpp"
#include "entity_overlay.hpp"
#include "entity_models.hpp"
#include "planted_bomb.hpp"
#include "grenade_prediction.hpp"
#include "sound_overlay.hpp"
#include "tablet_upgrade.hpp"
#include "frame_event_state.hpp"
#include "skeet/hud/observer_panel.hpp"
#include "skeet/players/state.hpp"

namespace skeet::esp {
class WorldEntityEngine;
class EnginePlayers;
struct FramePlantedBomb {
    const PlantedBombState& state;
    const ActiveBomb* registered_bomb = nullptr;
    BombTraceProvider trace{};
    const BombSites* sites = nullptr;
};

struct FrameWorldEntity {
    WorldEntity entity;
    // Each registered class supplies its actual decoded engine fields. Only
    // the matching handler member is read; no rendering decision is delegated.
    const HostageState* hostage = nullptr;
    const DroppedBombState* dropped_bomb = nullptr;
    const FramePlantedBomb* planted_bomb = nullptr;
    const GrenadeEntityState* grenade = nullptr;
    const InfernoTeamState* inferno_team = nullptr;
};
struct EspFrameOwner {
    explicit EspFrameOwner(client::PlayerStateCache& shared) : players(shared) {}
    EspFrameOwner(client::PlayerStateCache& shared, std::uint8_t& activation)
        : frame{0, 0, PlayerFrameActivation{activation}}, players(shared) {}
    PlayerFrameState frame;
    PlayerRuntimeState players;
    SoundOverlay sounds; // same owner: model +5C and conditional listener bytes
    TabletUpgradeState tablet; // same owner: upgrade snapshot/frame marker
    float bomb_peak_damage = 500.0f; // constructor +6B0, next to frame.timer (+6AC)
    std::uint32_t state_6b4 = 0;
    PickupObservations pickups; // constructor +71C/+720, shared with dormant update
    std::int32_t notice_entity = 0; //calloc0; lifecycle reset writes-1
    std::int32_t notice_texture = -1;
    std::int32_t defuse_kit_model = -1;
    std::span<const EntityOverlayHandler> class_handlers;
    EntityModelState entity_models;
    std::u16string_view dropped_bomb_label;
    GrenadeEntityClasses grenade_classes{};
    GrenadeEntityLabels grenade_labels{};
};
struct EspFrameInputs {
    const PlayerEspConfig& player_config;
    const WorldEspConfig& world_config;
    const GrenadeEspConfig& grenade_config;
    const PlayerDrawContext& draw;
    DroppedEntityProviders dropped;
    effects::GlowEngine* glow = nullptr;
    const IndicatorLayout& indicator_layout;
    InputBitmap keys;
    // Live native byte4347BE09 + slot*32, shared with PLAYERS and plist.
    const skeet::players::State& player_adjustments;
};
struct EspFrameContext : EspFrameInputs {
    EspFrameContext(EspFrameInputs input, EnginePlayers& source, WorldEntityEngine& world)
        : EspFrameInputs(input), players(source), entities(world) {}
    EnginePlayers& players;
    WorldEntityEngine& entities;
};

// Complete semantic 43408EC9 owner. World class handlers execute immediately
// after each GetClientEntity visit. Player preparation, drawing and state
// restoration remain in the same loop iteration, including observer panels.
void draw_esp_frame(ui::RenderQueue&, EspFrameOwner&, std::int32_t& indicator_cursor,
                    const EspFrameContext&);

} // namespace skeet::esp
