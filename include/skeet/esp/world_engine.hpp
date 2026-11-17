#pragma once
#include "frame.hpp"
#include "skeet/engine/scene_queries.hpp"
#include "skeet/engine/offset_storage.hpp"
#include "skeet/engine/decoded_offset.hpp"

namespace skeet::esp {
using WorldDecodedOffset = engine::DecodedOffset;
struct WorldEngineOffsets {
    explicit WorldEngineOffsets(const engine::NativeOffsetGlobals&);
    // Each conversion decodes the current pointer/key words at the actual
    // property use site; offset initialization is the canonical engine owner.
    WorldDecodedOffset origin; //2A671B42, live network origin
    WorldDecodedOffset model_index; //47877DEB,int16
    WorldDecodedOffset owner_handle; //8D1BD8D0
    WorldDecodedOffset definition; //5C9F0FBB,uint16
    WorldDecodedOffset ammunition; //CA5EC698,int32
    WorldDecodedOffset reserve_ammunition; //49C770CA,int32
    WorldDecodedOffset team; //8B5586F6
    WorldDecodedOffset hostage_effects; //BB78DC43,byte
    WorldDecodedOffset hostage_life_state; //CB6C59C2,int32
    WorldDecodedOffset studio_wrapper; //32E4DAF1,pointer
    WorldDecodedOffset hitbox_set; //C2B6FB10,int32
    WorldDecodedOffset rules_allow_bomb; //82F5AD08,byte
    WorldDecodedOffset dropped_bomb_state; //71A5AC08,int32
    WorldDecodedOffset grenade_damage; //727317C4,float
    WorldDecodedOffset grenade_radius; //9D63BB7F,float
    WorldDecodedOffset explosion_effect_tick; //01E80986,int32
    WorldDecodedOffset glow_index; //442A38AE,int32
    WorldDecodedOffset smoke_active; //58AE2423,byte
    WorldDecodedOffset smoke_start_tick; //F267E956,int32
    WorldDecodedOffset inferno_start_tick; //0DCBA89C,int32
    WorldDecodedOffset bomb_ticking; //0E5BECE5,byte
    WorldDecodedOffset bomb_detonation_time; //D280D4E1,float
    WorldDecodedOffset bomb_defuse_end; //800BB0B1,float
    WorldDecodedOffset bomb_defuser_state; //950F7B20,int32
    WorldDecodedOffset bomb_origin; //A9EE6062, distinct from network origin2A671B42
    WorldDecodedOffset bomb_site_first, bomb_site_second; //FCCAF99B/E92192CA,player resource
    WorldDecodedOffset view_offset; //C246315E,Vector3
    WorldDecodedOffset armor; //47A6F50C,int32
    WorldDecodedOffset life_state; //9D920E35,byte
};
struct WorldEngineBindings {
    engine::SceneQueries& queries;
    void** const& game_rules; //4346A3A4, optional pointer-to-rules
    void* const& planted_bombs; //434688C0, data+0/count+C
    engine::NativeConVarReference friendly_fire; //43467C3C^211963A6 ConVar
    engine::NativeConVarReference game_type; //43467A8C^AC9299CC ConVar
    const std::int32_t& maximum_players; //*43469300
    const float& current_time; //*43467EB8
    const std::int32_t& bomb_label_texture; //43467DA8,reloaded after label formatting
    const std::int32_t& current_tick; //43475A60 pointee+4
    const float& tick_interval; //*4346A338
    const Vector3& view_origin; //43479CCC..43479CD4, read at each range use
    const std::uint8_t& suspended; //43468E58
    WorldEngineOffsets offsets;
};

// This is the actual world side of43408EC9. by_index returns an untouched raw
// entity; only the selected handler reads its properties, at the corresponding
// native use sites. No decoder callbacks or prebuilt FrameWorldEntity are used.
class WorldEntityEngine {
public:
    explicit WorldEntityEngine(WorldEngineBindings bindings) : bindings_(bindings) {}
    void* by_index(std::int32_t) const;
    bool dormant(void*) const;
    std::uint32_t client_class_index(void*) const;
    bool is_weapon(void*) const;
    std::int32_t highest_entity_index() const;
    std::int32_t maximum_players() const noexcept {
        return bindings_.maximum_players;
    }
    std::int32_t game_type() const;
    std::int32_t indicator_texture() const noexcept {
        return bindings_.bomb_label_texture;
    }
    std::int32_t ammunition(const WorldEntity&) const;
    std::int32_t ammunition_capacity(const WorldEntity&) const;
    char bomb_site(std::int32_t entity_index) const; //complete433AC842, live resource
    void dispatch(ui::RenderQueue&, EspFrameOwner&, std::int32_t& indicator_cursor,
                  const EspFrameContext&, RuntimePlayer& view, void*, EntityOverlayHandler);
    void collect_dropped(WorldEntityScan&, std::array<WorldEntity, 65>& frame_entities, void*,
                         std::int32_t index, std::uint32_t captured_options, const EspFrameOwner&,
                         const EspFrameContext&);
    const WorldEngineBindings& bindings() const noexcept {
        return bindings_;
    }

private:
    WorldEngineBindings bindings_;
    void hostage(ui::RenderQueue&, const EspFrameContext&, RuntimePlayer&, void*);
    void dropped_bomb(ui::RenderQueue&, EspFrameOwner&, const EspFrameContext&, void*);
    void planted_bomb(ui::RenderQueue&, EspFrameOwner&, std::int32_t&, const EspFrameContext&,
                      RuntimePlayer&, void*);
    float bomb_damage(EspFrameOwner&, void* target);
    void grenade(ui::RenderQueue&, EspFrameOwner&, const EspFrameContext&, void*);
    void inferno(ui::RenderQueue&, EspFrameOwner&, const EspFrameContext&, RuntimePlayer&, void*);
    void defuse_kit(ui::RenderQueue&, const EspFrameOwner&, const EspFrameContext&, RuntimePlayer&,
                    void*);
    void danger_zone(EspFrameOwner&, RuntimePlayer&, void*);
};
} // namespace skeet::esp
