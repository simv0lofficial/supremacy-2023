#pragma once
#include "other_config.hpp"
#include "skeet/effects/glow.hpp"
#include "skeet/network/update_observers.hpp"
#include <array>
#include <cstddef>

namespace skeet::esp {
struct TabletUpgradeState {
    std::int32_t frame = 0; //constructor+718 is calloc0; lifecycle reset writes-1
    std::uint8_t status = 0; //saved constructor+E90
    std::array<std::uint32_t, 3> value_bits{}; //+E94
    std::array<std::int32_t, 8> identifiers{}; //+EA0
    std::array<std::uint32_t, 8> time_bits{}; //+EC0
};
struct TabletUpgradeOffsets {
    std::ptrdiff_t active_weapon; //CD812654, player handle
    std::ptrdiff_t definition; //5C9F0FBB, uint16
    std::ptrdiff_t status; //581805BB, byte
    std::ptrdiff_t values; //D6E91C78, three float words
    std::ptrdiff_t identifiers; //68484E0D, eight int32 identifiers
    std::ptrdiff_t times; //54E478AA, eight float words
};
struct TabletUpgradeBindings {
    const WorldEspConfig& settings; //OtherESP553D3C3F, distinct from MISC
    void* const& local_player; //43475808
    void* const& entity_list; //4346A890
    std::int32_t* const& frame_count; //434678F8
    float* const& current_time; //43467EB8
    TabletUpgradeOffsets offsets;
};
// Complete433E1983 and4331E0CC. Only tablet definition72 is admitted. All
// modified fields are saved before mutation and restored in the same frame.
void prepare_tablet_upgrade(TabletUpgradeState&, const TabletUpgradeBindings&);
void restore_tablet_upgrade(TabletUpgradeState&, const TabletUpgradeBindings&);

// Native43472170 at constructor+18 combines tablet pre/post with glow phase3.
// Borrow the same state held by EspFrameOwner and the canonical GlowRuntime.
class PlayerEspUpdateHandler final : public network::UpdateObserver {
public:
    PlayerEspUpdateHandler(engine::OwnerRegistryBinding, TabletUpgradeState&,
                           const TabletUpgradeBindings&, effects::GlowRuntime&);

private:
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_ESP_MEMBER __thiscall
#define SKEET_ESP_FAST __fastcall
#else
#define SKEET_ESP_MEMBER
#define SKEET_ESP_FAST
#endif
    static void SKEET_ESP_MEMBER prepare(void*);
    static void SKEET_ESP_MEMBER restore(void*);
    static void SKEET_ESP_MEMBER glow(void*);
    static void SKEET_ESP_MEMBER unchanged(void*);
    static void SKEET_ESP_FAST unchanged_player(void*, void*, std::uintptr_t, std::uintptr_t,
                                                std::uintptr_t);
    static void SKEET_ESP_FAST unchanged_view(void*, float*, float*, float*);
    static void SKEET_ESP_FAST unchanged_post(void*, void*);
    inline static constexpr network::UpdateObserverVtable entries_{
        {prepare, restore, unchanged, glow, unchanged},
        unchanged_player,
        unchanged_view,
        unchanged_post};
#undef SKEET_ESP_MEMBER
#undef SKEET_ESP_FAST
    TabletUpgradeState& state_;
    const TabletUpgradeBindings& bindings_;
    effects::GlowRuntime& glow_;
};
} // namespace skeet::esp
