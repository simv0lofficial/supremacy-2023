#pragma once
#include "skeet/script/bridge.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/engine/netvars.hpp"
#include "skeet/esp/flags.hpp"
#include "skeet/esp/projection.hpp"
#include "skeet/rage/targeting.hpp"
#include <span>
#include <string>
#include <unordered_map>
#include <variant>

namespace skeet::engine {
struct NativeOffsetStorage;
}
namespace skeet::esp {
class EnginePlayers;
class EspOwnerRuntime;
struct PlayerDrawContext;
}
namespace skeet::script {
// ScriptOwner is constructed before ESP. The production route borrows ESP's
// actual singleton cell and resolves its sole EnginePlayers/draw context at the
// API use. Standalone semantic inputs explicitly borrow an existing player view.
class EntityPlayerSource {
public:
    EntityPlayerSource(esp::EnginePlayers& standalone) noexcept : source_(&standalone) {}
    static EntityPlayerSource canonical(esp::EspOwnerRuntime* const&);
    esp::EnginePlayers& get() const;
    const esp::PlayerDrawContext& draw_context(const esp::PlayerDrawContext* standalone) const;
    bool is_canonical(esp::EspOwnerRuntime* const&) const noexcept;

private:
    explicit EntityPlayerSource(esp::EspOwnerRuntime* const* slot) noexcept : source_(slot) {}
    std::variant<esp::EnginePlayers*, esp::EspOwnerRuntime* const*> source_;
};
struct EntityOffsets {
    std::ptrdiff_t life_state; // 9D920E35
    std::ptrdiff_t suppressed; // CDE290D2
    std::ptrdiff_t enemy_excluded; // D070684C
    std::ptrdiff_t active_weapon; // CD812654
    std::ptrdiff_t resource_alive; // D266B537
    std::ptrdiff_t resource_connected;
    std::ptrdiff_t resource_team;
    std::ptrdiff_t resource_health;
    std::ptrdiff_t studio_header; // 32E4DAF1
    std::ptrdiff_t hitbox_set; // C2B6FB10
    std::ptrdiff_t bone_cache; // 17978090
};

// Borrowed engine objects and decoded netvar operands. The live player
// producer and draw context are shared with the concrete ESP frame dispatcher.
struct EntityBindings {
    void* engine = nullptr; // 4346A514
    void* entity_list = nullptr; // 4346A890
    void* base_client = nullptr; // 434683E0
    void** local_player = nullptr; // 43475808
    // These outer pointer globals can be replaced between Lua callbacks.
    // Borrow their variables, so every use observes the current pointee slot.
    void** const& game_rules; // 4346A3A4
    void** const& player_resource; // 43468FF0
    void* game_rules_proxy_table = nullptr; // 4346A720
    const std::int32_t* max_players = nullptr; // *43469300
    const esp::Projection* projection = nullptr;
    rage::TargetBindings targeting;
    EntityOffsets offsets{};
    EntityPlayerSource players;
    const esp::PlayerDrawContext* flag_context = nullptr;
    const std::uint8_t* esp_activation = nullptr; // 4346A65C
    const engine::RecvVarProxy* byte_proxy = nullptr; // 43477B54
    const engine::RecvVarProxy* short_proxy = nullptr; // 43477B58
    void* class_size_interface = nullptr; // 43469304, optional native name/size query
    engine::ClientClass* related_receive_class = nullptr; // 43467DBC
};

// Consumer operands come from the actual key-word storage initialized by
// 4348D41D. This also shares its two discovered receive-proxy field slots.
void bind_entity_offsets(EntityBindings&, const engine::NativeOffsetStorage&);

struct EntityProperty {
    std::uint32_t offset = 0;
    std::uint8_t type = 0, element_type = 0;
    std::uint16_t element_stride = 0, element_count = 0, reserved = 0;
};
static_assert(sizeof(EntityProperty) == 12);

class EntityModule {
public:
    EntityModule(EntityBindings& bindings, client::PlayerStateCache& cache) noexcept
        : bindings_(bindings), cache_(cache) {}
    EntityModule(Bridge& bridge, EntityBindings& bindings, client::PlayerStateCache& cache) noexcept
        : bindings_(bindings), cache_(cache), bridge_(&bridge) {}
    EntityModule(const EntityModule&) = delete;
    EntityModule& operator=(const EntityModule&) = delete;
    void register_exports(Bridge&); // 4338A7F2
    int get_local_player(void*); // 4336F045
    int get_all(void*); // 4334EBED
    int get_players(void*); // 433EF72B
    int get_game_rules(void*); // 433AEABC
    int get_player_resource(void*); // 433B4AFC
    int get_classname(void*); // 433C65DE
    int set_prop(void*); // 433B386F
    int get_prop(void*); // 43381620
    int new_prop(void*); // 433E4068
    int is_enemy(void*); // 433CE96E
    int is_alive(void*); // 433B86EF
    int is_dormant(void*); // 43400374
    int get_player_name(void*); // 4334E7C2
    int get_player_weapon(void*); // 43380003
    int hitbox_position(void*); // 433F983F
    int get_steam64(void*); // 43349EE4
    int get_bounding_box(void*); // 43406C8B
    int get_origin(void*); // 4334B59C
    int get_esp_data(void*); // 43367F94
private:
    EntityBindings& bindings_;
    client::PlayerStateCache& cache_;
    Bridge* bridge_ = nullptr;
    std::uint32_t rules_handle_ = UINT32_MAX;
    std::unordered_map<std::string, EntityProperty> custom_properties_;
    std::unordered_map<std::string, EntityProperty> resolved_properties_;
    std::unordered_map<std::string, std::uint32_t> class_sizes_;
    const LuaApi& lua() const {
        return *bridge_->api;
    }
    bool in_game() const;
    void* entity(std::uint32_t index) const;
    void* player(void*, std::uint32_t& index) const;
    bool dormant(void* entity) const;
    bool enemy(void* entity, bool dormant) const;
    std::uint32_t argument_index(void*, int argument = 1) const;
    bool property(void*, void*, EntityProperty&);
    bool property_in_bounds(void*, const EntityProperty&);
    bool property_address(void*, int argument, const EntityProperty&, std::byte*&,
                          std::uint8_t&) const;
    template <int Index> static int entry(void*);
};
} // namespace skeet::script
