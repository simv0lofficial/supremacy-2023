#pragma once
#include "registry.hpp"
#include "deaths.hpp"
#include "hit_log.hpp"
#include "hit_marker.hpp"

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
