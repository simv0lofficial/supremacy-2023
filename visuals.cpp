// Visuals & ESP Subsystem Implementation
#include <string>
#include "detail/esp_detail.hpp"
#include "detail/flag_table_detail.hpp"
#include "detail/grenade_live_detail.hpp"
#include "detail/world_engine_detail.hpp"
#include "util/address.h"
#include "skeet/esp/c4.hpp"
#include "skeet/esp/player_overlay.hpp"
#include "skeet/esp/config.hpp"
#include "skeet/esp/effects_config.hpp"
#include "skeet/esp/entity_models.hpp"
#include "skeet/esp/frame.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/events/game_events.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <new>
#include <stdexcept>
#include "skeet/esp/entity_overlay.hpp"
#include "skeet/esp/world_engine.hpp"
#include "skeet/font_metrics.hpp"
#include "skeet/esp/flag_table_initializer.hpp"
#include "skeet/client/native_startup.hpp"
#include "skeet/client/shutdown.hpp"
#include "skeet/engine/preserved_initialization.hpp"
#include "skeet/esp/player_engine.hpp"
#include "skeet/esp/frame_events.hpp"
#include "skeet/esp/frame_handler.hpp"
#include "skeet/esp/grenade_auxiliary.hpp"
#include "skeet/esp/grenade_warning_owner.hpp"
#include "skeet/movement/collision.hpp"
#include <bit>
#include <cstdio>
#include "skeet/esp/grenade_damage.hpp"
#include "skeet/engine/decoded_offset.hpp"
#include <array>
#include "skeet/esp/grenade_engine.hpp"
#include <xmmintrin.h>
#include "skeet/esp/grenade_environment.hpp"
#include "skeet/misc/grenade_command.hpp"
#include "skeet/client/command_recapture.hpp"
#include "skeet/render/backend.hpp"
#include "skeet/esp/grenade_live_bindings.hpp"
#include "skeet/engine/offset_storage.hpp"
#include "skeet/esp/grenade_prediction.hpp"
#include "skeet/rage/targeting.hpp"
#include <immintrin.h>
#include "skeet/effects/glow.hpp"
#include "skeet/esp/grenade_prediction_handler.hpp"
#include "skeet/movement/simulation_tick.hpp"
#include "skeet/color_picker.hpp"
#include "skeet/esp/hitbox_highlight.hpp"
#include "skeet/esp/hitbox_highlight_engine.hpp"
#include "skeet/esp/owner_runtime.hpp"
#include "skeet/esp/indicators.hpp"
#include "skeet/render/font_runtime.hpp"
#include "skeet/text_metrics.hpp"
#include <windows.h>
#include "skeet/esp/line_of_sight.hpp"
#include "skeet/esp/map_entity_parser.hpp"
#include <cstdlib>
#include "skeet/esp/model_color_config.hpp"
#include "skeet/esp/offscreen.hpp"
#include "skeet/resolver.hpp"
#include "skeet/esp/other_config.hpp"
#include "skeet/engine/owner_allocation.hpp"
#include "skeet/esp/packet_tracers.hpp"
#include "skeet/legit/accuracy.hpp"
#include "skeet/rage/pending_shot_publication.hpp"
#include "skeet/rage/targeting_commit.hpp"
#include "skeet/esp/planted_bomb.hpp"
#include "skeet/esp/player_draw.hpp"
#include "skeet/script/flags.hpp"
#include "skeet/esp/player_runtime.hpp"
#include "skeet/esp/projection.hpp"
#include "skeet/esp/runtime_bindings.hpp"
#include "skeet/client/runtime_storage.hpp"
#include "skeet/engine/scene_runtime_owner.hpp"
#include "skeet/esp/runtime_owner.hpp"
#include "skeet/client/runtime_composition.hpp"
#include "skeet/client/player_runtime_owner.hpp"
#include "skeet/client/visual_runtime_owner.hpp"
#include "skeet/effects/glow_engine.hpp"
#include "skeet/engine/netvar_initialization.hpp"
#include "skeet/rage/spread_storage.hpp"
#include "skeet/render/renderer_assets.hpp"
#include "skeet/render/startup_renderer.hpp"
#include "skeet/script/source_runtime_owner.hpp"
#include "skeet/esp/skeleton.hpp"
#include "skeet/esp/sound_overlay.hpp"
#include "skeet/esp/tablet_upgrade.hpp"
#include "skeet/esp/user_message_handlers.hpp"
#include "skeet/esp/weapon_icon.hpp"
#include "skeet/esp/weapon_text.hpp"
#include <cwchar>
#include "skeet/hud/composite_handlers.hpp"
#include "skeet/hud/composite_runtime.hpp"
#include "skeet/menu_window.hpp"
#include "skeet/render/backend_container.hpp"
#include "skeet/script/paint.hpp"
#include "skeet/hud/crosshair.hpp"
#include "skeet/hud/feature_indicators.hpp"
#include "skeet/hud/movement_indicators.hpp"
#include "skeet/hud/observer_panel.hpp"
#include "skeet/hud/penetration_reticle.hpp"
#include "skeet/lagcomp/update_timing.hpp"
#include "skeet/hud/penetration_reticle_owner.hpp"
#include "skeet/hud/penetration_runtime.hpp"
#include "skeet/rage_segments.hpp"
#include "skeet/hud/screen_overlay.hpp"
#include <vector>
#include "skeet/hud/screen_overlay_owner.hpp"
#include "skeet/hud/screen_overlay_update.hpp"
#include "skeet/core.hpp"
#include "skeet/hud/simple_owners.hpp"
#include "skeet/misc/reveal_ranks.hpp"
#include <cstddef>
#include "skeet/hud/simple_runtime_owner.hpp"
#include "skeet/engine/hook_runtime_owner.hpp"


// ============================================================================
// Subcomponent: c4.cpp
// ============================================================================


namespace skeet::esp {
inline float squared(Vector3 v) noexcept {
    return (v.y * v.y + v.x * v.x) + v.z * v.z;
}
inline float clamp(float value, float low, float high) noexcept {
    return _mm_cvtss_f32(_mm_min_ss(_mm_max_ss(_mm_set_ss(value), _mm_set_ss(low)), _mm_set_ss(high)));
}
}
namespace skeet::esp {
using namespace detail;

void draw_dropped_c4(ui::RenderQueue& queue, const WorldEspConfig& config,
                     const Projection& projection, const DroppedC4State& state,
                     std::u16string_view label) {
    if (!config.dropped_weapons.value)
        return;
    if (state.local_player_present && state.local_state == 0)
        return;
    if (state.entity_state != 0)
        return;
    Point point;
    if (!project_point(projection, state.origin, point))
        return;
    queue.text(point.x, point.y, config.bomb.color.value, 0x14, 0, label);
    if (config.dropped_weapons.value & 0x10) {
        auto distance = decimal(distance_in_feet(state.view_origin.x - state.origin.x,
                                                 state.view_origin.y - state.origin.y,
                                                 state.view_origin.z - state.origin.z));
        distance += u" FT";
        queue.text(point.x, add(point.y, 10), config.bomb.color.value, 0x14, 0, distance);
    }
}

float bomb_damage(const ActiveBomb* bomb, const BombDamageTarget& target, float peak_damage,
                  BombTraceProvider provider) {
    if (!bomb || !bomb->ticking)
        return 0.0f;
    const Vector3 start{bomb->origin.x, bomb->origin.y, bomb->origin.z + 8.0f};
    const Vector3 delta{start.x - start.x, start.y - start.y, (start.z - 40.0f) - start.z};
    const auto swept = (delta.y * delta.y + delta.x * delta.x) + delta.z * delta.z != 0.0f;
    const auto ground =
        provider.trace(provider.context, {start, delta, swept}, 0x200400Bu, bomb->entity);
    auto origin = bomb->origin;
    if (ground.fraction != 1.0f) {
        origin.x = ground.plane_normal.x * 0.6f + ground.end_position.x;
        origin.y = ground.plane_normal.y * 0.6f + ground.end_position.y;
        origin.z = ground.plane_normal.z * 0.6f + ground.end_position.z;
    }
    const auto radius = (peak_damage * 3.5f) * std::bit_cast<float>(0x3EAAAAABu);
    const auto denominator = (radius + radius) * radius;
    const auto x = (target.origin.x + target.view_offset.x) - origin.x;
    const auto y = (target.origin.y + target.view_offset.y) - origin.y;
    const auto z = (target.origin.z + target.view_offset.z) - (origin.z + 1.0f);
    const auto damage = std::exp(-((y * y + x * x) + z * z) / denominator) * peak_damage;
    return target.armor > 0 ? damage * 0.5f : damage;
}

} // namespace skeet::esp

// ============================================================================
// Subcomponent: config.cpp
// ============================================================================

namespace skeet::esp {
void initialize_overlay_config(ConfigLink& registry, OverlayConfig& c) noexcept {
    initialize_boolean_config(registry, c.bounding_box, 0xCFF9EBADu);
    initialize_color_config(registry, c.box_color, 0x4D243E0Du, 0x82FFFFFFu);
    initialize_boolean_config(registry, c.health_bar, 0x49733D3Cu);
    initialize_color_config(registry, c.health_color, 0xD2E84B64u, 0xFF50E178u);
    initialize_boolean_config(registry, c.name, 0xA1C152A1u);
    initialize_color_config(registry, c.name_color, 0x25FEB6C1u, 0xC8FFFFFFu);
    initialize_boolean_config(registry, c.distance, 0x0F6D81D5u);
    initialize_boolean_config(registry, c.flags, 0x084A8B33u);
    initialize_boolean_config(registry, c.ammo, 0x43F02F68u);
    initialize_color_config(registry, c.ammo_color, 0x8BEC37F4u, 0xE6C88C50u);
    initialize_boolean_config(registry, c.out_of_fov, 0x95A660ABu);
    initialize_integer_config(registry, c.arrow_size, 0x4564BFC7u, 12);
    initialize_integer_config(registry, c.arrow_distance, 0x09CBEDAEu, 100);
    initialize_color_config(registry, c.arrow_color, 0x7D18A71Fu, 0xFFFFFFFFu);
}

void initialize_player_esp_config(ConfigRegistries& registries, PlayerEspConfig& c) noexcept {
    auto& registry = registries.descriptors;
    initialize_hotkey_config(registries, c.activation, 0x4700A298u, 0x10, HotkeyMode::always);
    initialize_boolean_config(registry, c.bounding_box, 0xCFF9EBADu);
    initialize_color_config(registry, c.box_color, 0x4D243E0Du, 0x82FFFFFFu);
    initialize_boolean_config(registry, c.health_bar, 0x49733D3Cu);
    initialize_color_config(registry, c.health_color, 0xD2E84B64u, 0xFF50E178u);
    initialize_boolean_config(registry, c.name, 0xA1C152A1u);
    initialize_color_config(registry, c.name_color, 0x25FEB6C1u, 0xC8FFFFFFu);
    initialize_boolean_config(registry, c.weapon_text, 0xBB3FDF6Au);
    initialize_boolean_config(registry, c.weapon_icon, 0x6A874B2Cu);
    initialize_color_config(registry, c.weapon_icon_color, 0x00F64778u, 0xC8FFFFFFu);
    initialize_boolean_config(registry, c.distance, 0x0F6D81D5u);
    initialize_boolean_config(registry, c.glow, 0xB1D008B7u);
    initialize_color_config(registry, c.glow_color, 0xECE872CBu, 0x99783CB4u);
    initialize_boolean_config(registry, c.visualize_sounds, 0xE8D2A207u);
    initialize_color_config(registry, c.sound_color, 0x7E3B4C1Bu, 0xFF3F3FFAu);
    initialize_boolean_config(registry, c.flags, 0x084A8B33u);
    initialize_boolean_config(registry, c.ammo, 0x43F02F68u);
    initialize_color_config(registry, c.ammo_color, 0x8BEC37F4u, 0xE6C88C50u);
    initialize_boolean_config(registry, c.line_of_sight, 0x1DE914E6u);
    initialize_color_config(registry, c.sight_color, 0x745EFA1Eu, 0xFF0000FFu);
    initialize_boolean_config(registry, c.skeleton, 0x523E4B35u);
    initialize_color_config(registry, c.skeleton_color, 0x0E995F95u, 0xFFAAFFFFu);
    initialize_boolean_config(registry, c.teammates, 0x45D9D2ACu);
    initialize_boolean_config(registry, c.out_of_fov, 0x95A660ABu);
    initialize_integer_config(registry, c.arrow_size, 0x4564BFC7u, 12);
    initialize_integer_config(registry, c.arrow_distance, 0x09CBEDAEu, 100);
    initialize_color_config(registry, c.arrow_color, 0x7D18A71Fu, 0xFFFFFFFFu);
    initialize_boolean_config(registry, c.dormant, 0xC410C5D3u);
}
void initialize_hit_marker_config(ConfigLink& registry,
                                  HitMarkerConfig& c) noexcept { // 0x43486537..75
    initialize_boolean_config(registry, c.enabled, 0x9C345DFAu);
    initialize_boolean_config(registry, c.sound, 0x69C9D9F2u);
}
void initialize_aimbot_visualization_config(
    ConfigLink& registry, AimbotVisualizationConfig& c) noexcept { // 0x434804E7..543
    initialize_boolean_config(registry, c.enabled, 0x535095F5u);
    initialize_color_config(registry, c.color, 0x7451E155u, 0xCEE55200u);
    initialize_boolean_config(registry, c.safe_point, 0xE9CC8B95u);
    initialize_color_config(registry, c.safe_point_color, 0xE20F6035u, 0x920000FFu);
}
void initialize_player_esp_settings(ConfigRegistries& registries,
                                    PlayerEspSettings& settings) noexcept {
    initialize_player_esp_config(registries, settings.player);
    initialize_hit_marker_config(registries.descriptors, settings.hit_marker);
    initialize_aimbot_visualization_config(registries.descriptors, settings.aimbot_visualization);
    // Static descriptor at 0x4346906C, linked by 0x43490B75.
    initialize_boolean_config(registries.descriptors, settings.money, 0x53970838u);
}

} // namespace skeet::esp

// ============================================================================
// Subcomponent: effects_config.cpp
// ============================================================================

namespace skeet::esp {

void initialize_effects_settings(ConfigRegistries& registries, EffectsSettings& s) noexcept {
    auto& r = registries.descriptors;
    initialize_boolean_config(r, s.remove_flash, 0x461D3721u); // static 0x43468FF4
    initialize_boolean_config(r, s.remove_smoke, 0x623BD4F2u); // owner 0x43480354
    initialize_boolean_config(r, s.remove_fog, 0x3359E29Cu);
    initialize_boolean_config(r, s.remove_sky, 0xFBD4768Du);
    initialize_integer_config(r, s.visual_recoil, 0x70BA27E0u, 0); // static 0x4346744C
    initialize_integer_config(r, s.wall_transparency, 0x9B4090D9u, 100);
    initialize_integer_config(r, s.prop_transparency, 0x8BED781Eu, 100);
    initialize_integer_config(r, s.brightness, 0x2E5DD918u, 0);
    initialize_color_config(r, s.night_color, 0x2A0B0DF3u, 0xFF2E292Bu);
    initialize_boolean_config(r, s.remove_scope, 0x171EED00u); // owner 0x4348601F
    initialize_boolean_config(r, s.instant_scope, 0xEAFCEAD9u);
    initialize_boolean_config(r, s.disable_post_processing, 0xA8321370u);
    initialize_boolean_config(r, s.third_person_alive, 0xD35F8540u); // static 0x43468A00
    initialize_hotkey_config(registries, s.third_person_key, 0x3685FCE3u, 0x1B,
                             HotkeyMode::always); // static 0x43468438
    initialize_boolean_config(r, s.third_person_dead, 0x264E3E21u); // static 0x4346DAE0
    initialize_boolean_config(r, s.disable_teammates, 0x4CBB0E86u); // static 0x43468544
    initialize_boolean_config(r, s.disable_ragdolls, 0xB589F662u); // static 0x43469010
    initialize_boolean_config(r, s.bullet_tracers, 0x3A5285A6u);
    initialize_color_config(r, s.tracer_color, 0x635F47DEu, 0xFFFF8296u);
    initialize_boolean_config(r, s.bullet_impacts, 0xD54A1CBDu);
    initialize_integer_config(r, s.impact_duration, 0x4C044F71u, 4);
    initialize_boolean_config(r, s.harmless_molotovs, 0xC82CBD3Bu); // owner 0x43484B04
    initialize_color_config(r, s.harmless_molotov_color, 0xF9200DEFu, 0xFFFF5800u);
    initialize_integer_config(r, s.molotov_percent, 0x921CAA0Au, 65);
}

} // namespace skeet::esp

// ============================================================================
// Subcomponent: entity_models.cpp
// ============================================================================

#pragma STDC FENV_ACCESS ON

namespace skeet::esp {
namespace {
// Original256-word rehash table43473248..43473648.
constexpr std::array<std::uint32_t, 256> primes{
    2u,          3u,          5u,          7u,          11u,         13u,         17u,
    19u,         23u,         29u,         31u,         37u,         41u,         43u,
    47u,         53u,         59u,         61u,         67u,         71u,         73u,
    79u,         83u,         89u,         97u,         103u,        109u,        113u,
    127u,        137u,        139u,        149u,        157u,        167u,        179u,
    193u,        199u,        211u,        227u,        241u,        257u,        277u,
    293u,        313u,        337u,        359u,        383u,        409u,        439u,
    467u,        503u,        541u,        577u,        619u,        661u,        709u,
    761u,        823u,        887u,        953u,        1031u,       1109u,       1193u,
    1289u,       1381u,       1493u,       1613u,       1741u,       1879u,       2029u,
    2179u,       2357u,       2549u,       2753u,       2971u,       3209u,       3469u,
    3739u,       4027u,       4349u,       4703u,       5087u,       5503u,       5953u,
    6427u,       6949u,       7517u,       8123u,       8783u,       9497u,       10273u,
    11113u,      12011u,      12983u,      14033u,      15173u,      16411u,      17749u,
    19183u,      20753u,      22447u,      24281u,      26267u,      28411u,      30727u,
    33223u,      35933u,      38873u,      42043u,      45481u,      49201u,      53201u,
    57557u,      62233u,      67307u,      72817u,      78779u,      85229u,      92203u,
    99733u,      107897u,     116731u,     126271u,     136607u,     147793u,     159871u,
    172933u,     187091u,     202409u,     218971u,     236897u,     256279u,     277261u,
    299951u,     324503u,     351061u,     379787u,     410857u,     444487u,     480881u,
    520241u,     562841u,     608903u,     658753u,     712697u,     771049u,     834181u,
    902483u,     976369u,     1056323u,    1142821u,    1236397u,    1337629u,    1447153u,
    1565659u,    1693859u,    1832561u,    1982627u,    2144977u,    2320627u,    2510653u,
    2716249u,    2938679u,    3179303u,    3439651u,    3721303u,    4026031u,    4355707u,
    4712381u,    5098259u,    5515729u,    5967347u,    6456007u,    6984629u,    7556579u,
    8175383u,    8844859u,    9569143u,    10352717u,   11200489u,   12117689u,   13109983u,
    14183539u,   15345007u,   16601593u,   17961079u,   19431899u,   21023161u,   22744717u,
    24607243u,   26622317u,   28802401u,   31160981u,   33712729u,   36473443u,   39460231u,
    42691603u,   46187573u,   49969847u,   54061849u,   58488943u,   63278561u,   68460391u,
    74066549u,   80131819u,   86693767u,   93793069u,   101473717u,  109783337u,  118773397u,
    128499677u,  139022417u,  150406843u,  162723577u,  176048909u,  190465427u,  206062531u,
    222936881u,  241193053u,  260944219u,  282312799u,  305431229u,  330442829u,  357502601u,
    386778277u,  418451333u,  452718089u,  489790921u,  529899637u,  573292817u,  620239453u,
    671030513u,  725980837u,  785430967u,  849749479u,  919334987u,  994618837u,  1076067617u,
    1164186217u, 1259520799u, 1362662261u, 1474249943u, 1594975441u, 1725587117u, 1866894511u,
    2019773507u, 2185171673u, 2364114217u, 2557710269u, 2767159799u, 2993761039u, 3238918481u,
    3504151727u, 3791104843u, 4101556399u, 4294967291u,
};
void* model_table(void* const& tables) { //433AE8B7, wanted9FE6B136
    const auto count = engine::invoke<std::int32_t>(tables, 5);
    for (std::int32_t index = 0; index < count; ++index) {
        auto* table = engine::invoke<void*>(tables, 4, index);
        if (!table)
            continue;
        const auto* name = engine::invoke<const char*>(table, 1);
        if (name && events::event_hash(name) == 0x9FE6B136u)
            return table;
    }
    return nullptr;
}
}
DangerZoneModels::DangerZoneModels(engine::JobMemory memory) : memory_(memory) {
    static_assert(sizeof(void*) != 4 || sizeof(Node) == 12);
    if (!memory_.allocate || !memory_.free || !memory_.process_heap)
        throw std::invalid_argument("Entity model map requires native process heap exports");
}
DangerZoneModels::~DangerZoneModels() {
    release_storage();
}
void DangerZoneModels::release_storage() {
    clear();
    if (bucket_count_ > 1)
        free(buckets_);
    // The native owner is dead after this point. Keep the borrowed semantic
    // wrapper empty so its later C++ lifetime end does not free dead storage.
    initial_ = nullptr;
    buckets_ = &initial_;
    bucket_count_ = 1;
    next_resize_ = 0;
}
void* DangerZoneModels::allocate(std::size_t size) { //43387448
    const auto allocation_size = size + 0x20;
    auto* raw = memory_.allocate(memory_.process_heap, 8, allocation_size);
    if (!raw)
        return nullptr;
    const auto base = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (base + 15) & ~std::uintptr_t(15);
    const auto displacement = std::uint32_t(header - base);
    const auto stored_size = std::uint32_t(allocation_size);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &stored_size, 4);
    return reinterpret_cast<void*>(header + 16);
}
void DangerZoneModels::free(void* object) { //433837D0
    if (!object)
        return;
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory_.free(memory_.process_heap, 0, bytes - 16 - displacement);
}
void DangerZoneModels::clear() {
    const auto count = bucket_count_;
    auto** buckets = buckets_;
    for (std::uint32_t index = 0; index < count; ++index) {
        auto* node = buckets[index];
        while (node) {
            auto* next = node->next;
            free(node);
            node = next;
        }
        buckets[index] = nullptr;
    }
    size_ = 0;
}
void DangerZoneModels::rehash(std::uint32_t count) {
    auto** next = static_cast<Node**>(allocate((std::size_t(count) + 1) * sizeof(Node*)));
    std::memset(next, 0, std::size_t(count) * sizeof(Node*));
    next[count] = reinterpret_cast<Node*>(~std::uintptr_t{});
    std::uint32_t old_bucket = 0;
    for (; old_bucket < bucket_count_; ++old_bucket) {
        while (auto* node = buckets_[old_bucket]) {
            const auto bucket = std::uint32_t(node->entry.model_index) % count;
            buckets_[old_bucket] = node->next;
            node->next = next[bucket];
            next[bucket] = node;
        }
    }
    if (bucket_count_ > 1)
        free(buckets_);
    bucket_count_ = count;
    buckets_ = next;
}
bool DangerZoneModels::insert(std::uint16_t model, DangerZoneItem& item) {
    auto* node = ::new (allocate(sizeof(Node))) Node{{model, &item}, nullptr};
    auto bucket = std::uint32_t(model) % bucket_count_;
    for (auto* previous = buckets_[bucket]; previous; previous = previous->next) {
        if (previous->entry.model_index == model) {
            free(node);
            return false;
        }
    }
    const auto wanted = size_ + 1;
    if (wanted > next_resize_) {
        const auto previous = bucket_count_ == 1 ? 0u : bucket_count_;
        if (wanted > previous) {
            // Constructor policy is exactly load1/growth2. Model keys are
            // uint16, so these integer products are exact native float values.
            const auto minimum = std::max(previous * 2, wanted);
            const auto count = *std::lower_bound(primes.begin(), primes.end(), minimum);
            next_resize_ = count;
            bucket = std::uint32_t(model) % count;
            rehash(count);
        } else
            next_resize_ = previous;
    }
    node->next = buckets_[bucket];
    buckets_[bucket] = node;
    ++size_;
    return true;
}
DangerZoneItem* DangerZoneModels::find(std::uint16_t model) const noexcept {
    for (auto* node = buckets_[std::uint32_t(model) % bucket_count_]; node; node = node->next)
        if (node->entry.model_index == model)
            return node->entry.item;
    return nullptr;
}
DangerZoneModels::Iterator DangerZoneModels::begin() const noexcept {
    std::uint32_t bucket = 0;
    while (bucket < bucket_count_ && !buckets_[bucket])
        ++bucket;
    return {this, bucket, bucket == bucket_count_ ? nullptr : buckets_[bucket]};
}
DangerZoneModels::Iterator& DangerZoneModels::Iterator::operator++() noexcept {
    node_ = node_->next;
    if (!node_) {
        do {
            ++bucket_;
        } while (bucket_ < map_->bucket_count_ && !map_->buckets_[bucket_]);
        if (bucket_ < map_->bucket_count_)
            node_ = map_->buckets_[bucket_];
    }
    return *this;
}
EntityModelState::EntityModelState(engine::JobMemory memory) : models(memory) {
    const auto templates = danger_zone_item_templates();
    for (std::size_t index = 0; index < descriptors.size(); ++index) {
        descriptors[index].label = templates[index].label;
        descriptors[index].distance = 0.0f; //constructor+descriptor1C
    }
}
void EntityModelState::bind_config(const WorldEspConfig& config) noexcept {
    const auto templates = danger_zone_item_templates();
    for (std::size_t index = 0; index < descriptors.size(); ++index)
        descriptors[index].setting = &(config.*templates[index].setting);
}
void reset_danger_zone_models(EntityModelState& state) noexcept {
    for (const auto& entry : state.models)
        entry.item->distance = std::numeric_limits<float>::max();
}
void collect_danger_zone_model(EntityModelState& state, const WorldEntity& entity,
                               Vector3 local) noexcept {
    if (entity.dormant || entity.model_index < 0)
        return;
    auto* item = state.models.find(std::uint16_t(entity.model_index));
    if (!item || !item->setting->enabled.value)
        return;
    const auto x = local.x - entity.origin.x, y = local.y - entity.origin.y,
               z = local.z - entity.origin.z;
    const auto distance = std::sqrt((y * y + x * x) + z * z);
    if (distance >= item->distance)
        return;
    item->distance = distance;
    item->closest_origin = entity.origin;
}
void draw_danger_zone_models(ui::RenderQueue& queue, const Projection& projection,
                             std::int32_t mode, bool enabled, const EntityModelState& state) {
    if (mode != 6 || !enabled)
        return;
    for (const auto& entry : state.models)
        draw_danger_zone_items(queue, projection, 6, true, {entry.item, 1});
}
void reset_esp_frame_owner(EspFrameOwner& owner, effects::GlowRuntime& glow) {
    owner.frame.timer = 0;
    owner.state_6b4 = 0;
    owner.sounds.ring_model = -1;
    glow.bomb_handle = -1;
    owner.tablet.frame = -1;
    owner.bomb_peak_damage = 500.0f;
    owner.frame.transient = 0;
    glow.weapons_active = false;
    owner.pickups.count = 0;
    owner.entity_models.models.clear();
    owner.notice_entity = -1;
}
void initialize_esp_entity_models(EspFrameOwner& owner, effects::GlowRuntime& glow,
                                  const WorldEspConfig& config, void* const& tables) {
    reset_esp_frame_owner(owner, glow);
    owner.entity_models.bind_config(config);
    auto* models = model_table(tables);
    if (!models)
        return;
    const auto count = engine::invoke<std::int32_t>(models, 3);
    const auto templates = danger_zone_item_templates();
    for (std::int32_t index = 0; index < count; ++index) {
        const auto* name = engine::invoke<const char*>(models, 9, index);
        if (!name)
            continue;
        const auto length = std::char_traits<char>::length(name);
        if (length <= 38) {
            if (length == 28 && events::event_hash(name) == 0x58A15B00u)
                owner.defuse_kit_model = index;
            continue;
        }
        if (name[7] != 'p' || name[12] != '_' || name[16] != 'v' || name[21] != '/')
            continue;
        const auto hash = events::event_hash(name);
        for (std::size_t item = 0; item < templates.size(); ++item) {
            if (hash != templates[item].model_hash)
                continue;
            owner.entity_models.models.insert(std::uint16_t(index),
                                              owner.entity_models.descriptors[item]);
            break;
        }
    }
    owner.sounds.initialize_model_table(models);
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: entity_overlay.cpp
// ============================================================================

namespace skeet::esp {
using namespace detail;
namespace {
float distance_squared(Vector3 a, Vector3 b) noexcept {
    const auto x = a.x - b.x, y = a.y - b.y, z = a.z - b.z;
    return (y * y + x * x) + z * z;
}
void distance_label(ui::RenderQueue& q, Point at, std::uint32_t color, float squared) {
    auto text = decimal(mul(sub(2, truncate(std::sqrt(squared) / -12.0f)) / 5, 5));
    text += u" FT";
    q.text(at.x, at.y, color, 0x14, 0, text);
}
Vector3 transform(const Matrix3x4& m, Vector3 v, bool second = false) noexcept {
    return {((v.x * m.rows[0][0] + v.y * m.rows[0][1]) + v.z * m.rows[0][2]) + m.rows[0][3],
            ((v.y * m.rows[1][1] + v.x * m.rows[1][0]) + v.z * m.rows[1][2]) + m.rows[1][3],
            second
                ? ((v.z * m.rows[2][2] + v.x * m.rows[2][0]) + v.y * m.rows[2][1]) + m.rows[2][3]
                : ((v.x * m.rows[2][0] + v.y * m.rows[2][1]) + v.z * m.rows[2][2]) + m.rows[2][3]};
}
}
std::uint32_t entity_class_hash(std::string_view name) noexcept {
    std::uint32_t hash = 0x811C9DC5u;
    for (const auto byte : name) {
        if (!byte)
            break;
        hash = (hash ^ std::uint8_t(byte)) * 0x01000193u;
    }
    return hash;
}
EntityOverlayHandler entity_overlay_handler(std::uint32_t hash) noexcept {
    switch (hash) {
    case 0x1A122BDF:
        return EntityOverlayHandler::hostage;
    case 0x1D49B0B5:
        return EntityOverlayHandler::dropped_bomb;
    case 0xE1614C81:
        return EntityOverlayHandler::planted_bomb;
    case 0xE31DC1B8:
    case 0xC09C6702:
    case 0xA239EA8F:
    case 0x69629121:
        return EntityOverlayHandler::grenade_projectile;
    case 0xABE104C9:
        return EntityOverlayHandler::inferno;
    case 0x24E7D8E9:
    case 0x5AA1386D:
    case 0xAE060272:
    case 0xD3227436:
    case 0xDBDC655C:
        return EntityOverlayHandler::danger_zone_item;
    case 0xCD91F684:
        return EntityOverlayHandler::defuse_kit;
    default:
        return EntityOverlayHandler::none;
    }
}
std::vector<EntityOverlayHandler> entity_overlay_handlers(std::span<const std::uint32_t> hashes) {
    std::vector<EntityOverlayHandler> handlers(hashes.size());
    std::transform(hashes.begin(), hashes.end(), handlers.begin(), entity_overlay_handler);
    return handlers;
}
bool ammunition_weapon(std::uint16_t definition) noexcept {
    constexpr std::array<std::uint32_t, 3> bits{0xFF8B6F9Eu, 0xB00001DFu, 1};
    return definition < 65 && (bits[definition >> 5] & (std::uint32_t(1) << (definition & 31)));
}
EntityOverlayHandler select_world_entity(WorldEntityScan& result, const WorldEntity& entity,
                                         std::span<const EntityOverlayHandler> handlers,
                                         i32 maximum_players, i32 highest, std::uint32_t options,
                                         i32 kit_model, const Projection& projection,
                                         Vector3 view) {
    if (std::int32_t(entity.index) <= maximum_players || std::int32_t(entity.index) >= highest ||
        (entity.index & 0xFFFF0000u) || !entity.identity || entity.dormant)
        return EntityOverlayHandler::none;
    const auto handler = entity.class_index < handlers.size() ? handlers[entity.class_index]
                                                              : EntityOverlayHandler::none;
    if (handler != EntityOverlayHandler::none) {
        result.specialized.push_back({handler, &entity});
        return handler;
    }
    if (!options || entity.owner_handle != -1)
        return EntityOverlayHandler::none;
    std::uint32_t definition;
    if (entity.weapon) {
        definition = entity.definition;
        if (ammunition_weapon(entity.definition) &&
            (entity.ammunition < 0 || add(entity.ammunition, entity.reserve_ammunition) <= 0))
            return EntityOverlayHandler::none;
    } else {
        if (kit_model == -1 || std::int32_t(entity.model_index) != kit_model)
            return EntityOverlayHandler::none;
        definition = 55;
    }
    Point screen;
    if (!project_point(projection, entity.origin, screen) || definition >= 65)
        return EntityOverlayHandler::none;
    auto& selected = result.dropped[definition];
    const auto squared = distance_squared(view, entity.origin);
    if (selected.entity && squared >= selected.distance_squared)
        return EntityOverlayHandler::none;
    selected = {&entity, squared, screen};
    return EntityOverlayHandler::none;
}
WorldEntityScan scan_world_entities(std::span<const WorldEntity> entities,
                                    std::span<const EntityOverlayHandler> handlers,
                                    i32 maximum_players, i32 highest, std::uint32_t options,
                                    i32 kit_model, const Projection& projection, Vector3 view) {
    WorldEntityScan result;
    for (const auto& entity : entities)
        select_world_entity(result, entity, handlers, maximum_players, highest, options, kit_model,
                            projection, view);
    return result;
}
namespace {
template <class Options, class Color, class Ammunition, class Capacity>
void draw_dropped(ui::RenderQueue& q, const WorldEntityScan& scan, DroppedEntityProviders providers,
                  Options options, Color color, Ammunition ammunition_at, Capacity capacity_at) {
    if (!options())
        return;
    for (std::uint16_t definition = 0; definition != 65; ++definition) {
        const auto& dropped = scan.dropped[definition];
        if (!dropped.entity)
            continue;
        if (!q.fonts)
            throw std::logic_error("Dropped entity ESP requires a font backend");
        const auto font_height = ui::font_height(*q.fonts, 4);
        auto baseline = dropped.screen.y;
        Point measured{};
        if (options() & 1) {
            measured.x = 40;
            draw_weapon_icon(q, providers.icons, definition, {dropped.screen.x, baseline}, color(),
                             &measured.x);
            baseline = add(baseline, 12);
        }
        if (options() & 2) {
            draw_weapon_text(q, providers.names, definition,
                             {dropped.screen.x, add(baseline, font_height / 2)}, color(),
                             &measured);
            baseline = add(baseline, font_height);
        }
        if (definition != 55 && (options() & 8)) {
            if (!measured.x)
                measured.x = 20;
            const auto x = sub(dropped.screen.x, measured.x / 2);
            const auto ammunition = ammunition_at(*dropped.entity);
            if (ammunition != -1) {
                const auto capacity = capacity_at(*dropped.entity);
                if (capacity > 0) {
                    const auto fraction =
                        sse_min(sse_max(float(ammunition) / float(capacity), 0.0f), 1.0f);
                    const auto filled = truncate(float(measured.x) * fraction);
                    q.rectangle({sub(x, 1), add(baseline, 2), add(measured.x, 2), 4,
                                 (((color() >> 24) * 160) / 255) << 24});
                    q.rectangle({x, add(baseline, 3), filled, 2, color()});
                    if (fraction < 0.25f)
                        q.text(add(x, filled), add(baseline, 5), 0xC8FFFFFFu, 0x14, 0,
                               decimal(ammunition));
                }
            }
            baseline = add(baseline, 6);
        }
        if (options() & 16)
            distance_label(q, {dropped.screen.x, add(baseline, font_height / 2)}, color(),
                           dropped.distance_squared);
    }
}
}
void draw_dropped_entities(ui::RenderQueue& q, const WorldEntityScan& scan, std::uint32_t options,
                           std::uint32_t color, DroppedEntityProviders providers) {
    draw_dropped(
        q, scan, providers, [=] { return options; }, [=] { return color; },
        [](const WorldEntity& entity) { return entity.ammunition; },
        [=](const WorldEntity& entity) {
            if (!providers.ammunition_capacity)
                throw std::logic_error(
                    "Dropped entity ammo bar requires the engine capacity query");
            return providers.ammunition_capacity(providers.context, entity);
        });
}
void draw_dropped_entities(ui::RenderQueue& q, const WorldEntityScan& scan,
                           const WorldEspConfig& config, DroppedEntityProviders providers,
                           WorldEntityEngine& engine) {
    draw_dropped(
        q, scan, providers, [&] { return std::uint32_t(config.dropped_weapons.value); },
        [&] { return config.dropped_weapon_color.value; },
        [&](const WorldEntity& entity) { return engine.ammunition(entity); },
        [&](const WorldEntity& entity) { return engine.ammunition_capacity(entity); });
}
void draw_dropped_bomb(ui::RenderQueue& q, const Projection& projection, Vector3 view,
                       std::uint32_t options, std::uint32_t color, std::u16string_view label,
                       const DroppedBombState& bomb) {
    if (!options || (bomb.game_rules_present && !bomb.game_rules_allow) || bomb.bomb_state != 0)
        return;
    Point screen;
    if (!project_point(projection, bomb.origin, screen))
        return;
    q.text(screen.x, screen.y, color, 0x14, 0, label);
    if (options & 16)
        distance_label(q, {screen.x, add(screen.y, 10)}, color,
                       distance_squared(view, bomb.origin));
}
void draw_defuse_kit(ui::RenderQueue& q, const Projection& projection, Vector3 view,
                     std::uint32_t options, std::uint32_t color, i32 team, std::int16_t model,
                     i32 kit_model, Vector3 origin) {
    if (!options || team == 2 || std::int32_t(model) != kit_model)
        return;
    Point screen;
    if (!project_point(projection, origin, screen))
        return;
    q.text(screen.x, screen.y, color, 0x14, 0, u"KIT");
    if (options & 16)
        distance_label(q, {screen.x, add(screen.y, 10)}, color, distance_squared(view, origin));
}
void draw_hostage(ui::RenderQueue& q, const Projection& projection, Vector3 local, bool enabled,
                  bool box, bool distance, std::uint32_t color, const HostageState& entity) {
    if (!enabled || entity.dormant || (entity.effects & 2) || entity.life_state > 1 ||
        !entity.bones_available || !entity.model_available || entity.hitboxes.empty())
        return;
    i32 left = INT32_MAX, top = INT32_MAX, right = INT32_MIN, bottom = INT32_MIN;
    for (const auto& hitbox : entity.hitboxes) {
        if (hitbox.bone >= entity.bones.size() || hitbox.bone >= 256)
            throw std::out_of_range("Hostage hitbox exceeds the native bone workspace");
        const auto first = transform(entity.bones[hitbox.bone], hitbox.minimum);
        const auto second = transform(entity.bones[hitbox.bone], hitbox.maximum, true);
        for (const auto corner : std::array<Vector3, 8>{{{first.x, first.y, first.z},
                                                         {first.x, first.y, second.z},
                                                         {first.x, second.y, first.z},
                                                         {first.x, second.y, second.z},
                                                         {second.x, first.y, first.z},
                                                         {second.x, first.y, second.z},
                                                         {second.x, second.y, first.z},
                                                         {second.x, second.y, second.z}}}) {
            Point screen;
            if (!project_point(projection, corner, screen))
                return;
            left = std::min(left, screen.x);
            right = std::max(right, screen.x);
            top = std::min(top, screen.y);
            bottom = std::max(bottom, screen.y);
        }
    }
    left = std::max(left, 0);
    right = std::max(right, 0);
    top = std::max(top, 0);
    bottom = std::max(bottom, 0);
    if (left >= right || top >= bottom)
        return;
    const auto width = sub(right, left), height = sub(bottom, top);
    const auto center = add(left, width / 2);
    q.text(center, sub(top, 5), color, 0x14, 0, u"HOSTAGE");
    auto baseline = add(top, 3);
    if (box) {
        q.rectangle_outline({sub(left, 1), sub(top, 1), add(width, 2), add(height, 2), 0x8C000000u},
                            1);
        q.rectangle_outline({left, top, width, height, color}, 1);
        q.rectangle_outline({add(left, 1), add(top, 1), sub(width, 2), sub(height, 2), 0x8C000000u},
                            1);
        baseline = add(bottom, 6);
    }
    if (distance)
        distance_label(q, {center, baseline}, 0xC8FFFFFFu, distance_squared(local, entity.origin));
}
void reset_danger_zone_items(std::span<DangerZoneItem> items) noexcept {
    for (auto& item : items)
        item.distance = std::numeric_limits<float>::max();
}
std::array<DangerZoneItemTemplate, 20> danger_zone_item_templates() noexcept {
    using C = WorldEspConfig;
    return {{{0x6427A53Fu, u"DRONE GUN", &C::drone_gun},
             {0x66F439F2u, u"BLACKHAWK", &C::blackhawk},
             {0xEC5261E8u, u"DRONE", &C::drone},
             {0x2D7C23AAu, u"RANDOM CASE", &C::random_case},
             {0x807E926Cu, u"TOOL CASE", &C::tool_case},
             {0xDE29550Au, u"PISTOL CASE", &C::pistol_case},
             {0x4DF8C90Cu, u"EXPLOSIVE CASE", &C::explosive_case},
             {0x6D711DFDu, u"HEAVY WEAPON CASE", &C::heavy_weapon_case},
             {0xF9D1C0FCu, u"LIGHT WEAPON CASE", &C::light_weapon_case},
             {0x31DBE954u, u"DUFFLEBAG", &C::dufflebag},
             {0xA22A50FFu, u"JAMMER", &C::jammer},
             {0x17927C7Au, u"AMMOBOX", &C::ammobox},
             {0x97187BC7u, u"ARMOR", &C::armor},
             {0xDEC16CE1u, u"ARMOR+HELMET", &C::armor},
             {0x7CAFC101u, u"HELMET", &C::armor},
             {0x87365B32u, u"PARACHUTE", &C::parachute_pack},
             {0x1D83CCB9u, u"BRIEFCASE", &C::briefcase},
             {0xDDE26198u, u"UPGRADE TABLET ZONE", &C::tablet_upgrade_zone},
             {0x1EB563B0u, u"UPGRADE TABLET DRONE", &C::tablet_upgrade_drone},
             {0xA44C6770u, u"CASH", &C::cash_stack}}};
}
void collect_danger_zone_item(std::span<DangerZoneItem> items, const WorldEntity& entity,
                              Vector3 local) noexcept {
    if (entity.dormant || entity.model_index < 0)
        return;
    for (auto& item : items) {
        if (item.model_index != entity.model_index)
            continue;
        if (!(item.setting ? item.setting->enabled.value : item.enabled))
            return;
        const auto distance = std::sqrt(distance_squared(local, entity.origin));
        if (distance >= item.distance)
            return;
        item.distance = distance;
        item.closest_origin = entity.origin;
        return;
    }
}
void draw_danger_zone_items(ui::RenderQueue& q, const Projection& projection, i32 game_mode,
                            bool enabled, std::span<const DangerZoneItem> items) {
    if (game_mode != 6 || !enabled)
        return;
    for (const auto& item : items) {
        if (item.distance == std::numeric_limits<float>::max())
            continue;
        Point screen;
        if (!project_point(projection, item.closest_origin, screen))
            continue;
        const auto color = item.setting ? item.setting->color.value : item.color;
        q.text(screen.x, screen.y, color, 0x14, 0, item.label);
        const auto distance_color =
            ((item.setting ? item.setting->color.value : item.color) & 0xFF000000u) | 0x00FFFFFFu;
        auto text = decimal(mul(sub(2, truncate(item.distance / -12.0f)) / 5, 5));
        text += u" FT";
        q.text(screen.x, add(screen.y, 9), distance_color, 0x14, 0, text);
    }
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: flag_table_initializer.cpp
// ============================================================================

namespace skeet::esp {
namespace {
FlagTableInitialization* active_initialization = nullptr;
std::uint32_t address(const void* pointer) noexcept {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(pointer));
}
char16_t* pointer(std::uint32_t value) noexcept {
    return reinterpret_cast<char16_t*>(std::uintptr_t(value));
}
void* allocate_text(const engine::JobMemory& memory, std::uint32_t bytes) { //43387448
    if (!bytes)
        return nullptr;
    auto* const raw = memory.allocate(memory.process_heap, 8, bytes + 0x20u);
    if (!raw)
        return nullptr;
    const auto base = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (base + 15u) & ~std::uintptr_t(15);
    const auto displacement = static_cast<std::uint32_t>(header - base);
    const auto allocation_size = bytes + 0x20u;
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    return reinterpret_cast<void*>(header + 16);
}
void free_text(const engine::JobMemory& memory, std::uint32_t value) { //433837D0
    if (!value)
        return;
    auto* const bytes = reinterpret_cast<std::byte*>(std::uintptr_t(value));
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
void finalize_flags() {
    active_initialization->flags.finalize();
}
void initialize_flags(void* owner) {
    auto& context = *static_cast<FlagTableInitialization*>(owner);
    context.flags.initialize(); //43407D58 CALL43485097 precedes registration
    (void)context.callbacks.append(context.source_finalizer(0x43401BC2u));
}
}

FlagTable::FlagTable(engine::PreservedInitialization& initialization, engine::JobMemory memory)
    : FlagTable(initialization.image_owner(), memory) {}
FlagTable::FlagTable(std::shared_ptr<engine::NativeDataImage> image, engine::JobMemory memory)
    : image_(image), memory_(memory),
      rows_(engine::require_native_data_image(image_).native_cell<std::array<NativeFlagRow, 18>>(
          0x43479120u)),
      shared_empty_(image_->native_cell<char16_t>(0x434759ACu)) {
    if (!memory_.allocate || !memory_.free || !memory_.process_heap)
        throw std::invalid_argument("Native ESP flag strings require their process heap");
}

void FlagTable::construct_text(NativeFlagRow& row, const char16_t* source) {
    //433D2786 clears only the pointer triple, then scans the terminated source.
    row.first = 0;
    row.last = 0;
    row.limit = 0;
    std::uint32_t length = 0;
    while (source[length])
        ++length;
    const auto capacity = length + 1u;
    std::uint32_t first, limit;
    if (capacity < 2) { //4339E00C shared two-byte empty allocation
        first = address(&shared_empty_);
        limit = first + 2u;
    } else {
        first = address(allocate_text(memory_, capacity * 2u));
        limit = first + capacity * 2u;
    }
    row.first = first;
    row.last = first;
    row.limit = limit;
    //43342C16 caches the newly published first pointer across memmove. Its
    // last-pointer write and terminator use that same destination afterwards.
    const auto destination = row.first;
    const auto copied = length * 2u;
    std::memmove(pointer(destination), source, copied);
    row.last = destination + copied;
    *pointer(destination + copied) = 0;
}

void FlagTable::initialize() { //43485097..434856C5
    for (std::size_t index = 0; index < 15; ++index) {
        const auto initial = detail::initial_flag_entry(index);
        auto& row = rows_[index];
        row.mask = initial.mask;
        row.color = initial.color;
        construct_text(row, initial.text.data());
    }
    //43485691 clears all72 bytes before publishing the three empty triples.
    std::memset(rows_.data() + 15, 0, 3 * sizeof(NativeFlagRow));
    const auto empty = address(&shared_empty_);
    for (std::size_t index = 15; index < 18; ++index) {
        auto& row = rows_[index];
        row.first = empty;
        row.last = empty;
        row.limit = empty + 2u;
    }
}

void FlagTable::finalize() { //43401BC2 /433C9151 /433823B5
    for (std::size_t index = 18; index; --index) {
        const auto& row = rows_[index - 1];
        const auto capacity = (row.limit - row.first) & 0xFFFFFFFEu;
        if (static_cast<std::int32_t>(capacity) > 2 && row.first)
            free_text(memory_, row.first);
    }
    //The terminal native destructor leaves every row word unchanged.
}
std::u16string_view FlagTable::text(std::size_t index) const noexcept {
    const auto& row = rows_[index];
    return {pointer(row.first), std::uint32_t(row.last - row.first) / 2u};
}

void (*FlagTableInitialization::source_finalizer(std::uint32_t native_address))() {
    if (native_address != 0x43401BC2u)
        throw std::out_of_range("Finalizer is not the recovered ESP flag-table entry");
    if (active_initialization && active_initialization != this)
        throw std::logic_error("ESP flag-table exit callback requires one stable source binding");
    active_initialization = this;
    return finalize_flags;
}
void bind_flag_table_initializer(client::NativeStartupSources& sources,
                                 FlagTableInitialization& context) {
    sources.bind_initializer({0x43407D58u, &context, initialize_flags});
}
} //namespace skeet::esp

// ============================================================================
// Subcomponent: flags.cpp
// ============================================================================

namespace skeet::esp {
using namespace detail;

FlagEntry detail::initial_flag_entry(std::size_t index) noexcept { //43485097 values
    switch (index) {
    case 0:
        return {1, 0xC8FFFFFFu, u"H"};
    case 1:
        return {2, 0xC8FFFFFFu, u"K"};
    case 2:
        return {4, 0xC8FFFFFFu, u"HK"};
    case 3:
        return {8, 0xD2E1B43Cu, u"ZOOM"};
    case 4:
        return {0x10, 0xD2E1B43Cu, u"BLIND"};
    case 5:
        return {0x20, 0xD2E1B43Cu, u"RELOAD"};
    case 6:
        return {0x40, 0xFF3200FFu, u"C4"};
    case 7:
        return {0x80, 0xFF3200FFu, u"VIP"};
    case 8:
        return {0x100, 0xFF3200FFu, u"DEFUSE"};
    case 9:
        return {0x200, 0xC8FFFFFFu, u"FD"};
    case 10:
        return {0x400, 0xFF3200FFu, u"PIN"};
    case 11:
        return {0x800, 0xC8FFFFFFu, u"HIT"};
    case 12:
        return {0x1000, 0xC8FFFFFFu, u"O"};
    case 13:
        return {0x2000, 0xC8FFFFFFu, u"X"};
    case 14:
        return {0x20000, 0xFF3200FFu, u"X"};
    default:
        return {0, 0, u""};
    }
}
std::array<FlagEntry, 18> default_flag_entries() noexcept {
    std::array<FlagEntry, 18> table;
    for (std::size_t index = 0; index < table.size(); ++index)
        table[index] = detail::initial_flag_entry(index);
    return table;
}

float reload_cycle(const AnimationLayer& layer) noexcept { // 0x433CEFB3
    if (layer.count >= 1 && layer.weight != 0.0f && layer.activity == 0x3C7)
        return layer.cycle;
    return 1.0f;
}

bool correction_pending(const RecordHistory& h) noexcept { // 0x4334D33C
    if (!h.correction_count)
        return false;
    const auto& slot = h.corrections[h.correction_index % 6];
    return slot.counter < sub(slot.limit, 1);
}

bool fake_duck(const RecordHistory& h, float amount, float speed) noexcept { // 0x433D1FFD
    return h.count >= 2 && sub(h.ticks[h.first & 31], h.ticks[(h.first + 1) & 31]) > 2 &&
           amount > 0.0f && amount < 1.0f && speed == 8.0f;
}

std::uint32_t active_flags(const PlayerFlagState& p) noexcept { // 0x43417468
    std::uint32_t flags = 0;
    if (p.game_mode != 2) {
        flags = (p.helmet ? 1u : 0u) | (p.armor > 0 ? 2u : 0u);
        if (flags == 3)
            flags = 4;
    }
    if (!p.dormant) {
        if (p.scoped)
            flags |= 8;
        if (p.flash_alpha > 20.0f)
            flags |= 0x10;
        if (p.vip)
            flags |= 0x80;
        if (p.team == 3 && p.defusing)
            flags |= 0x100;
        if (fake_duck(p.history, p.duck_amount, p.duck_speed))
            flags |= 0x200;
        if (p.weapon_valid) {
            if (p.ammunition != -1 && reload_cycle(p.animation) != 1.0f)
                flags |= 0x20;
            if (p.weapon_kind == 9 && p.pin_pulled)
                flags |= 0x400;
        }
    }
    if (p.entity_index == p.bomb_owner_index)
        flags |= 0x40;
    // The engine time vcall returns through x87; the original rounds the
    // extended quotient to float only after adding 0.5.
    const auto now = truncate(
        float(static_cast<long double>(p.engine_time) / static_cast<long double>(p.tick_interval) +
              0.5L));
    const auto difference = sub(p.last_hit_tick, now);
    const auto magnitude = difference < 0 ? sub(0, difference) : difference;
    if (magnitude <= 16)
        flags |= 0x800;
    if (p.on_shot_enabled) {
        if (p.target_is_player && p.local_player_present && !p.local_player_disallowed &&
            p.on_shot_extra_enabled)
            flags |= 0x1000;
        if (p.history.count) {
            const auto tick = truncate(p.simulation_time / p.simulation_tick_interval + 0.5f);
            if (tick < p.history.ticks[p.history.first & 31])
                flags |= 0x20000;
            else if (correction_pending(p.history))
                flags |= 0x2000;
        }
    }
    return flags;
}

void draw_flags(ui::RenderQueue& q, Point& cursor, i32 height, std::uint32_t mask,
                std::span<const FlagEntry, 18> table) { // 0x4340A7AA..0x4340A818
    if (!mask)
        return;
    for (const auto& entry : table) {
        if (!(entry.mask & mask))
            continue;
        q.text(cursor.x, cursor.y, entry.color, 4, 0, entry.text);
        cursor.y = add(cursor.y, height);
    }
}

void draw_flags(ui::RenderQueue& q, Point& cursor, i32 height, std::uint32_t mask,
                const FlagTable& table) { //4340A7AA..4340A818
    if (!mask)
        return;
    for (std::size_t index = 0; index < 18; ++index) {
        const auto& row = table.rows()[index];
        if (!(row.mask & mask))
            continue;
        q.text(cursor.x, cursor.y, row.color, 4, 0, table.text(index));
        cursor.y = add(cursor.y, height);
    }
}

} // namespace skeet::esp

// ============================================================================
// Subcomponent: frame.cpp
// ============================================================================

#pragma STDC FENV_ACCESS ON

namespace skeet::esp {
void draw_esp_frame(ui::RenderQueue& queue, EspFrameOwner& owner, std::int32_t& cursor,
                    const EspFrameContext& c) {
    begin_player_frame(owner.frame, c.player_config.activation.value.binding, c.keys,
                       c.players.absolute_frame_time());
    if (c.world_config.bomb.enabled.value && owner.frame.timer > 0.0f &&
        owner.notice_entity != -1) {
        const auto site = c.entities.bomb_site(owner.notice_entity);
        const auto fraction =
            detail::sse_min(detail::sse_max((3.0f - owner.frame.timer) / 3.0f, 0.0f), 1.0f);
        if (c.draw.scripts)
            draw_indicator(*c.draw.scripts, queue, cursor, {&site, 1}, 0xFF69F3FCu,
                           c.indicator_layout, c.entities.indicator_texture(), fraction);
        else
            draw_indicator(queue, cursor, {&site, 1}, 0xFF69F3FCu, c.indicator_layout,
                           c.entities.indicator_texture(), fraction);
    }
    auto* initial_view = effective_view_player(c.players);
    if (!initial_view)
        return;
    const auto game_mode = c.entities.game_type();
    if (game_mode == 6 && c.world_config.danger_zone_items.value)
        reset_danger_zone_models(owner.entity_models);
    WorldEntityScan collected;
    std::array<WorldEntity, 65> dropped_entities;
    const auto highest = c.entities.highest_entity_index();
    const auto world_maximum_players = c.entities.maximum_players();
    for (auto index = world_maximum_players + 1; index < highest; ++index) {
        if (std::uint32_t(index) & 0xFFFF0000u)
            continue;
        auto* entity = c.entities.by_index(index);
        if (!entity)
            continue;
        if (c.entities.dormant(entity))
            continue;
        const auto class_index = c.entities.client_class_index(entity);
        const bool registered = class_index < owner.class_handlers.size() &&
                                owner.class_handlers[class_index] != EntityOverlayHandler::none;
        if (registered) {
            const auto handler = owner.class_handlers[class_index];
            c.entities.dispatch(queue, owner, cursor, c, *initial_view, entity, handler);
            continue;
        }
        const auto scan_options = std::uint32_t(c.world_config.dropped_weapons.value);
        // This branch already captured an absent class handler. The native
        // owner has no deferred specialized vector and does not look it up
        // again after the IsWeapon engine call.
        c.entities.collect_dropped(collected, dropped_entities, entity, index, scan_options, owner,
                                   c);
    }
    draw_dropped_entities(queue, collected, c.world_config, c.dropped, c.entities);
    draw_danger_zone_models(queue, c.draw.projection, game_mode,
                            c.world_config.danger_zone_items.value, owner.entity_models);
    const auto maximum_players = c.players.maximum_players();
    for (std::int32_t slot = 1; slot < maximum_players; ++slot) {
        const bool blocked =
            owner.frame.activation &&
            c.player_adjustments.records[std::uint32_t(slot)].adjustments.disable_visuals != 0;
        const auto prepared =
            prepare_live_player(queue, owner.players, c.player_config, c.world_config, c.players,
                                initial_view, slot, owner.frame.activation, blocked);
        if (prepared.stage == PreparedPlayerStage::observer_panel) {
            draw_live_observer(queue, owner.frame, c.players, initial_view, prepared);
        } else if (prepared.stage == PreparedPlayerStage::overlay) {
            draw_live_prepared_player(queue, c.player_config, c.world_config, prepared, c.draw,
                                      c.players);
        }
    }
}

} // namespace skeet::esp

// ============================================================================
// Subcomponent: frame_events.cpp
// ============================================================================

namespace skeet::esp {
namespace {
constexpr std::uint32_t pickup_hash = 0x1A7A32B9, spawn_hash = 0xC90B4171;
constexpr std::uint32_t round_hash = 0x3D07D291, planted_hash = 0xDCBA11CF;
constexpr std::uint32_t equip_hash = 0xAF956ECC, begin_hash = 0xEF1E4D35, abort_hash = 0xEDD7EBF0;
bool integer(events::GameEventReader& event, std::uint32_t hash, events::FieldValue& value) {
    return !event.read(hash, events::FieldType::integer, value);
}
}
RuntimePlayer* EspFrameEvents::event_player(std::uint32_t user, events::EventPlayerIndices& indices,
                                            std::int32_t& index) {
    index = indices.player_for_user_id(user);
    if (index < 1 || index > 64)
        return nullptr;
    auto* player = players_.by_index(players_.context, index);
    return player && players_.is_player(players_.context, *player) ? player : nullptr;
}
void EspFrameEvents::item_pickup_slerp(events::GameEventReader& event,
                                       events::EventPlayerIndices& indices) {
    if (!config_.dormant.value)
        return;
    events::FieldValue behavior, user, item;
    if (!integer(event, 0x5D23E3D6, behavior) || !integer(event, 0x8361150E, user) ||
        !integer(event, 0x9BC0F45A, item) || behavior.integer != 1)
        return;
    std::int32_t index;
    if (!event_player(user.integer, indices, index))
        return;
    auto& queue = owner_.pickups;
    const auto next = queue.count + 1;
    queue.count = next < 128 ? next : 128;
    if (queue.count != 1)
        std::memmove(queue.entries.data() + 1, queue.entries.data(),
                     (queue.count - 1) * sizeof(PickupObservation));
    queue.entries[0].item_index = std::uint16_t(item.integer);
    queue.entries[0].player_index = std::uint16_t(index);
    queue.entries[0].frame = *frame_;
}
void EspFrameEvents::player_spawn(events::GameEventReader& event,
                                  events::EventPlayerIndices& indices) {
    auto& queue = owner_.pickups;
    const auto* counter = frame_;
    while (queue.count && queue.entries[queue.count - 1].frame < *counter)
        --queue.count;
    events::FieldValue user;
    if (!integer(event, 0x8361150E, user))
        return;
    std::int32_t index;
    if (!event_player(user.integer, indices, index))
        return;
    auto& geometry = owner_.players.cache.get(std::uint32_t(index))->geometry;
    geometry.last_live_tick = 0;
    geometry.alpha = 0;
}
void EspFrameEvents::item_equip(events::GameEventReader& event,
                                events::EventPlayerIndices& indices) {
    if (!config_.dormant.value)
        return;
    events::FieldValue user, definition;
    if (!integer(event, 0x8361150E, user) || !integer(event, 0x3D4DBA99, definition))
        return;
    std::int32_t index;
    if (!event_player(user.integer, indices, index))
        return;
    owner_.players.cache.get(std::uint32_t(index))->cached_definition =
        std::uint16_t(definition.integer);
}
void EspFrameEvents::site(events::GameEventReader& event) {
    events::FieldValue value;
    if (integer(event, 0xA55BB3BF, value))
        owner_.notice_entity = std::int32_t(value.integer);
}
void EspFrameEvents::bomb_begin_plant(events::GameEventReader& event) {
    owner_.frame.timer = 3.0f;
    site(event);
}
void EspFrameEvents::reset_plant(events::GameEventReader& event) {
    owner_.frame.timer = 0;
    site(event);
}
void EspFrameEvents::round_start(events::GameEventReader& event) {
    owner_.pickups.count = 0;
    reset_plant(event);
}
void EspFrameEvents::pickup(void* self, events::GameEventReader& event,
                            events::EventPlayerIndices& indices) {
    static_cast<EspFrameEvents*>(self)->item_pickup_slerp(event, indices);
}
void EspFrameEvents::spawn(void* self, events::GameEventReader& event,
                           events::EventPlayerIndices& indices) {
    static_cast<EspFrameEvents*>(self)->player_spawn(event, indices);
}
void EspFrameEvents::equip(void* self, events::GameEventReader& event,
                           events::EventPlayerIndices& indices) {
    static_cast<EspFrameEvents*>(self)->item_equip(event, indices);
}
void EspFrameEvents::begin(void* self, events::GameEventReader& event,
                           events::EventPlayerIndices&) {
    static_cast<EspFrameEvents*>(self)->bomb_begin_plant(event);
}
void EspFrameEvents::reset(void* self, events::GameEventReader& event,
                           events::EventPlayerIndices&) {
    static_cast<EspFrameEvents*>(self)->reset_plant(event);
}
void EspFrameEvents::round(void* self, events::GameEventReader& event,
                           events::EventPlayerIndices&) {
    static_cast<EspFrameEvents*>(self)->round_start(event);
}
void EspFrameEvents::bind_before_death(events::EventRegistry& registry) {
    registry.add({pickup_hash, this, pickup});
    registry.add({spawn_hash, this, spawn});
}
void EspFrameEvents::bind_after_death(events::EventRegistry& registry) {
    registry.add({round_hash, this, round});
    registry.add({planted_hash, this, reset});
}
std::array<events::EventListener, 3> EspFrameEvents::dormant_listeners() noexcept {
    return {{{equip_hash, this, equip}, {begin_hash, this, begin}, {abort_hash, this, reset}}};
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: frame_handler.cpp
// ============================================================================

namespace skeet::esp {
EspFrameHandler::EspFrameHandler(engine::OwnerRegistryBinding registry, EspFrameOwner& owner,
                                 const EspFrameContext& context)
    : owner_(owner), context_(context) {
    // Native registration precedes installation of this interface's vtable.
    hud::register_hud_frame(registry, this);
    vtable = entries_;
}

#if defined(_M_IX86) || defined(__i386__)
void __fastcall EspFrameHandler::draw(void* object, ui::RenderQueue* queue, void*,
                                      std::int32_t* cursor) {
#else
void EspFrameHandler::draw(void* object, ui::RenderQueue* queue, void*, std::int32_t* cursor) {
#endif
    auto& handler = *static_cast<EspFrameHandler*>(static_cast<hud::HudFrameCallback*>(object));
    //43408EC9 resolves43359AFA itself; the dispatcher's local-player argument
    // is passed by the interface ABI but is not read by the native body.
    draw_esp_frame(*queue, handler.owner_, *cursor, handler.context_);
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_auxiliary.cpp
// ============================================================================

namespace skeet::esp {
GrenadeAuxiliaryLiveOffsets::GrenadeAuxiliaryLiveOffsets(const engine::NativeOffsetGlobals& g)
    : rejection_bytes{{{g, 0x43468E78, 0x434684E4, 0x9D920E35, 0},
                       {g, 0x4346A5DC, 0x43468430, 0xCDE290D2, 0},
                       {g, 0x43468BF8, 0x43467B8C, 0xD070684C, 0},
                       {g, 0x43467DD0, 0x4346A610, 0xAF649DAC, 0}}},
      team{g, 0x434688B0, 0x4346A518, 0x8B5586F6, 0},
      survival_team{g, 0x434678AC, 0x43467B7C, 0x430C8993, 0},
      origin{g, 0x434670CC, 0x43468BCC, 0xA9EE6062, 0},
      duck_amount{g, 0x434679F8, 0x434675F8, 0x3309C0CF, 0},
      smoke_active{g, 0x4346A530, 0x43467564, 0x58AE2423, 0},
      view_angles{g, 0x43468EA0, 0x4346A74C, 0xE23A9BE1, 0} {}
namespace {
/* [removed duplicate field] */
float negative(float value) {
    return std::bit_cast<float>(std::bit_cast<std::uint32_t>(value) ^ 0x80000000U);
}
Vector3 difference(Vector3 a, Vector3 b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}
float squared_yxz(Vector3 a) {
    return (a.y * a.y + a.x * a.x) + a.z * a.z;
}
Vector3 normalized(Vector3 a) {
    const float length = std::sqrt((a.x * a.x + a.y * a.y) + a.z * a.z);
    const float inverse = 1.0f / (length + std::numeric_limits<float>::epsilon());
    return {a.x * inverse, a.y * inverse, a.z * inverse};
}
GrenadeRay line(Vector3 start, Vector3 end) {
    const auto delta = difference(end, start);
    return {start, delta, {}, squared_yxz(delta) != 0.0f, true};
}
std::uint32_t identity(const void* object) {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(object));
}
Vector3 forward(Vector3 angles, const RageTrigonometry& trig) {
    constexpr float radians = 0.01745329238474369f;
    const float sy = trig.sine(angles.y * radians);
    const float cy = trig.cosine(angles.y * radians);
    const float sp = trig.sine(angles.x * radians);
    const float cp = trig.cosine(angles.x * radians);
    return {cp * cy, cp * sy, negative(sp)};
}
template <class Context>
void flash_basis(Vector3 delta, const Context& context, Vector3& right, Vector3& up) {
    Vector3 angles{};
    if (delta.y == 0.0f && delta.x == 0.0f)
        angles.x = delta.z <= 0.0f ? 90.0f : 270.0f;
    else {
        constexpr float degrees = 57.2957763671875f;
        angles.y = static_cast<float>(context.arc_tangent(delta.y, delta.x)) * degrees;
        const float horizontal = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        angles.x = static_cast<float>(context.arc_tangent(negative(delta.z), horizontal)) * degrees;
    }
    // Actual43366128 is called with no forward output, followed by right/up.
    // Retain roll-zero sin/cos calls and operation order rather than replacing
    // the pair with a cross product derived from the unrounded input delta.
    constexpr float radians = 0.01745329238474369f;
    const auto& trig = context.trigonometry;
    const float sp = trig.sine(angles.x * radians), cp = trig.cosine(angles.x * radians);
    const float sy = trig.sine(angles.y * radians), cy = trig.cosine(angles.y * radians);
    const float sr = trig.sine(angles.z * radians), cr = trig.cosine(angles.z * radians);
    const float cr_negative = cr * -1.0f;
    const float srsp = sr * sp;
    right = normalized(
        {cr_negative * negative(sy) - srsp * cy, cr_negative * cy - srsp * sy, (sr * -1.0f) * cp});
    const float crsp = cr * sp;
    up = normalized({crsp * cy - negative(sy) * sr, crsp * sy - sr * cy, cr * cp});
}
bool visible(const GrenadeDamageTrace& trace, std::uint32_t target) {
    return trace.fraction == 1.0f || trace.entity == target;
}
std::u16string feet_annotation(std::int32_t distance) {
    const float feet = static_cast<float>(distance) / 12.0f;
    char text[32];
    const auto count = std::snprintf(text, sizeof text, "%.1f ft", static_cast<double>(feet));
    return std::u16string(text, text + count);
}
template <class Context> void* entity_list(const Context& context) {
    return context.targeting.entity_list;
}
void* entity_list(const GrenadeAuxiliaryLiveContext& context) {
    return context.entity_list;
}
template <class Context> void* local_player(const Context& context) {
    return context.targeting.local_player;
}
void* local_player(const GrenadeAuxiliaryLiveContext& context) {
    return context.local_player;
}
template <class Context> const void* game_type(const Context& context) {
    return context.targeting.game_type;
}
const void* game_type(const GrenadeAuxiliaryLiveContext& context) {
    return context.game_type;
}
template <class Context> const void* teammates_are_enemies(const Context& context) {
    return context.targeting.teammates_are_enemies;
}
const void* teammates_are_enemies(const GrenadeAuxiliaryLiveContext& context) {
    return context.teammates_are_enemies;
}
template <class Context> const rage::TargetOffsets& target_offsets(const Context& context) {
    return context.targeting.offsets;
}
const GrenadeAuxiliaryLiveOffsets& target_offsets(const GrenadeAuxiliaryLiveContext& context) {
    return context.offsets;
}
template <class Context> const players::State& player_adjustments(const Context& context) {
    return context.targeting.player_adjustments;
}
const players::State& player_adjustments(const GrenadeAuxiliaryLiveContext& context) {
    return context.player_adjustments;
}
template <class Context> std::ptrdiff_t smoke_offset(const Context& context) {
    return context.smoke_active;
}
std::ptrdiff_t smoke_offset(const GrenadeAuxiliaryLiveContext& context) {
    return context.offsets.smoke_active;
}
template <class Context> std::ptrdiff_t view_angles_offset(const Context& context) {
    return context.view_angles;
}
std::ptrdiff_t view_angles_offset(const GrenadeAuxiliaryLiveContext& context) {
    return context.offsets.view_angles;
}
std::int32_t convar_integer(const void* object) {
    for (;;) {
        const auto* parent = field<const void*>(object, 0x1C);
        if (parent == object)
            break;
        object = parent;
    }
    return std::bit_cast<std::int32_t>(
        field<std::uint32_t>(object, 0x30) ^
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(object)));
}
template <class Context> bool enemy(const Context& context, const void* target) {
    // Complete4339EF8C. The current game type selects which live decoder is
    // touched; no eagerly populated TargetOffsets snapshot is constructed.
    const auto* local = local_player(context);
    if (!local || !target || local == target)
        return false;
    const auto& offsets = target_offsets(context);
    if (convar_integer(game_type(context)) == 6) {
        const std::ptrdiff_t offset = offsets.survival_team;
        const auto team = field<std::int32_t>(local, offset);
        return team == -1 || team != field<std::int32_t>(target, offset);
    }
    const std::ptrdiff_t offset = offsets.team;
    const auto team = field<std::int32_t>(local, offset);
    const auto other = field<std::int32_t>(target, offset);
    if (team == 2) {
        if (other == 3)
            return true;
    } else if (team == 3) {
        if (other == 2)
            return true;
    } else
        return false;
    return convar_integer(teammates_are_enemies(context)) != 0;
}
template <class Context> bool eligible(const Context& context, void* target) {
    // Complete43358962. In the live scene, none of the preceding genuine
    // entity/networkable virtuals can freeze the later43475808 local-player
    // read in4339EF8C. The offsets and PLAYERS storage remain canonical.
    if (!target || !engine::invoke<bool>(target, 158))
        return false;
    const auto& adjustments = player_adjustments(context);
    const auto& offsets = target_offsets(context);
    if (field<std::uint8_t>(target, offsets.rejection_bytes[0]))
        return false;
    if (engine::invoke<std::int32_t>(target, 122) <= 0)
        return false;
    auto* networkable = static_cast<std::byte*>(target) + 8;
    if (engine::invoke<bool>(networkable, 9))
        return false;
    if (field<std::uint8_t>(target, offsets.rejection_bytes[1]) ||
        field<std::uint8_t>(target, offsets.rejection_bytes[2]))
        return false;
    const auto index = engine::invoke<std::uint32_t>(networkable, 10);
    if (adjustments.records[index].adjustments.whitelist ||
        field<std::uint8_t>(target, offsets.rejection_bytes[3]))
        return false;
    return enemy(context, target);
}
} // namespace

template <class Context>
static GrenadeFlashResult flash(Vector3 position, std::uint32_t ignored_entity,
                                const Context& context) {
    GrenadeFlashResult result;
    Vector3 nearest_origin{};
    const auto& offsets = target_offsets(context);
    const auto maximum_players = context.maximum_players;
    for (std::int32_t index = 1; index <= maximum_players; ++index) {
        auto* entity = engine::invoke<void*>(entity_list(context), 3, index);
        if (!eligible(context, entity))
            continue;
        auto origin = *engine::invoke<const Vector3*>(entity, 10);
        // 43367C8B returns the live virtual origin with the native duck
        // smoothstep eye height added; its C export omits that SSE tail.
        origin.z = grenade_player_view_height(field<float>(entity, offsets.duck_amount)) + origin.z;
        const float distance = squared_yxz(difference(origin, position));
        if (distance < result.distance_squared) {
            result.target = entity;
            result.distance_squared = distance;
            nearest_origin = origin;
        }
    }
    if (!result.target)
        return result;
    const auto origin = *engine::invoke<const Vector3*>(result.target, 10);
    const auto maximum = *engine::invoke<const Vector3*>(result.target, 77);
    const Vector3 eye{origin.x, origin.y, (maximum.z - 8.0f) + origin.z};
    Vector3 right, up;
    flash_basis(difference(eye, position), context, right, up);
    const auto target = identity(result.target);
    const GrenadeDamageFilter filter{GrenadeDamageFilterKind::ignore_entity, ignored_entity};
    {
        MovementTraceFlagGuard guard(context.trace_flag);
        auto trace = context.engine.trace(line(position, eye), 0x6004001U, filter);
        if (visible(trace, target))
            result.exposure = 1.0f;
        else {
            const Vector3 ends[]{
                {up.x * 50.0f + position.x, up.y * 50.0f + position.y, up.z * 50.0f + position.z},
                {(right.x * 75.0f + position.x) + up.x * 10.0f,
                 (right.y * 75.0f + position.y) + up.y * 10.0f,
                 (right.z * 75.0f + position.z) + up.z * 10.0f},
                {(position.x - right.x * 75.0f) + up.x * 10.0f,
                 (position.y - right.y * 75.0f) + up.y * 10.0f,
                 (position.z - right.z * 75.0f) + up.z * 10.0f}};
            for (const auto end : ends) {
                trace = context.engine.trace(line(position, end), 0x6004001U, filter);
                trace = context.engine.trace(line(trace.end, eye), 0x6004001U, filter);
                if (visible(trace, target))
                    result.exposure += 0.16699999570846558f;
            }
        }
    }
    if (!(result.exposure > 0.0f))
        return result;
    float duration =
        3.549999952316284f - std::sqrt(result.distance_squared) * 0.0011833333410322666f;
    if (!(duration > 0.0f))
        return result;
    const auto view =
        forward(field<Vector3>(result.target, view_angles_offset(context)), context.trigonometry);
    const auto toward_flash = normalized(difference(position, nearest_origin));
    const float facing =
        (toward_flash.y * view.y + toward_flash.x * view.x) + toward_flash.z * view.z;
    // COMISD threshold,facing followed by JBE also takes the first arm for
    // unordered engine angles.
    if (!(static_cast<double>(facing) < 0.6))
        duration *= 2.5f;
    else if (!(static_cast<double>(facing) < 0.3))
        duration *= 1.75f;
    else if (facing < -0.20000000298023224f)
        duration *= 0.5f;
    float fraction = ((duration * result.exposure) * 0.6000000238418579f) / 5.099999904632568f;
    fraction = detail::sse_min(detail::sse_max(fraction, 0.0f), 1.0f);
    result.percent = detail::truncate(fraction * 100.0f);
    return result;
}

template <class Context>
static void fire_distance(const GrenadeState& state, const Context& context, std::int32_t& distance,
                          bool& highlight) {
    float best = 30625.0f;
    float height = std::numeric_limits<float>::max();
    const auto& offsets = target_offsets(context);
    const auto maximum_players = context.maximum_players;
    for (std::int32_t index = 1; index <= maximum_players; ++index) {
        auto* entity = engine::invoke<void*>(entity_list(context), 3, index);
        if (!eligible(context, entity))
            continue;
        const auto delta = difference(state.position, field<Vector3>(entity, offsets.origin));
        const float squared = squared_yxz(delta);
        if (squared < best) {
            best = squared;
            height = delta.z;
        }
    }
    if (best == 30625.0f)
        return;
    if (context.smoke_projectile_class) {
        const auto last = field<std::int32_t>(entity_list(context), 0x24);
        for (std::int32_t index = maximum_players + 1; index <= last; ++index) {
            if (static_cast<std::uint32_t>(index) & 0xffff0000U)
                continue;
            auto* entity = engine::invoke<void*>(entity_list(context), 3, index);
            if (!entity)
                continue;
            auto* networkable = static_cast<std::byte*>(entity) + 8;
            const auto* type = engine::invoke<const void*>(networkable, 2);
            if (type != context.smoke_projectile_class ||
                !field<std::uint8_t>(entity, smoke_offset(context)))
                continue;
            if (grenade_warning_smoke_overlap(state.position,
                                              field<Vector3>(entity, offsets.origin)))
                return;
        }
    }
    distance = detail::truncate(std::sqrt(best));
    if (state.fire_ground && distance < 150 && height >= -20.0f && height < 128.0f)
        highlight = true;
}

void evaluate_grenade_fire_distance(const GrenadeState& state,
                                    const GrenadeAuxiliaryContext& context, std::int32_t& distance,
                                    bool& highlight) {
    fire_distance(state, context, distance, highlight);
}
void evaluate_grenade_fire_distance(const GrenadeState& state,
                                    const GrenadeFireDistanceContext& context,
                                    std::int32_t& distance, bool& highlight) {
    fire_distance(state, context, distance, highlight);
}
void evaluate_grenade_fire_distance(const GrenadeState& state,
                                    const GrenadeAuxiliaryLiveContext& context,
                                    std::int32_t& distance, bool& highlight) {
    fire_distance(state, context, distance, highlight);
}
GrenadeFlashResult evaluate_grenade_flash(Vector3 position, std::uint32_t ignored,
                                          const GrenadeAuxiliaryContext& context) {
    return flash(position, ignored, context);
}
GrenadeFlashResult evaluate_grenade_flash(Vector3 position, std::uint32_t ignored,
                                          const GrenadeAuxiliaryLiveContext& context) {
    return flash(position, ignored, context);
}

template <class Context>
static GrenadeAuxiliaryResult auxiliary(const GrenadeState& state, const Context& context) {
    GrenadeAuxiliaryResult result;
    if (state.kind == GrenadeKind::flash) {
        result.magnitude =
            evaluate_grenade_flash(state.position, state.owner_entity, context).percent;
        if (result.magnitude > 0)
            result.annotation = detail::decimal(result.magnitude) + u"%";
    } else if (state.kind == GrenadeKind::molotov || state.kind == GrenadeKind::incendiary) {
        result.draw_path = state.fire_ground;
        if (!result.draw_path)
            return result;
        evaluate_grenade_fire_distance(state, context, result.magnitude, result.highlight);
        if (result.magnitude > 0)
            result.annotation = feet_annotation(result.magnitude);
    } else if (state.kind == GrenadeKind::smoke && state.extinguished)
        result.magnitude = 100;
    return result;
}
GrenadeAuxiliaryResult evaluate_grenade_auxiliary(const GrenadeState& state,
                                                  const GrenadeAuxiliaryContext& context) {
    return auxiliary(state, context);
}
GrenadeAuxiliaryResult evaluate_grenade_auxiliary(const GrenadeState& state,
                                                  const GrenadeAuxiliaryLiveContext& context) {
    return auxiliary(state, context);
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_damage.cpp
// ============================================================================

namespace skeet::esp {
namespace {
Vector3 subtract(Vector3 a, Vector3 b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}
float length_xyz(Vector3 v) {
    return std::sqrt((v.x * v.x + v.y * v.y) + v.z * v.z);
}
/* [removed duplicate squared_yxz] */
/* [removed duplicate line] */
/* [removed duplicate negative] */
}
std::array<Vector3, 5> make_grenade_damage_samples(Vector3 origin, Vector3 eye, float eye_offset,
                                                   Vector3 top, Vector3 angles,
                                                   const GrenadeDamageMath& math) {
    const auto& trig = math.trigonometry;
    constexpr float radians = 0.01745329238474369f;
    const float sy = trig.sine(angles.y * radians);
    const float cy = trig.cosine(angles.y * radians);
    const float sp = trig.sine(angles.x * radians);
    const float cp = trig.cosine(angles.x * radians);
    const float sr = trig.sine(angles.z * radians);
    const float cr = trig.cosine(angles.z * radians);
    const float cr_negative = cr * -1.0f;
    const float srsp = sr * sp;
    const float right_x = negative(sy) * cr_negative - srsp * cy;
    const float right_y = cr_negative * cy - srsp * sy;
    const float right_z = (sr * -1.0f) * cp;
    const float right_length =
        std::sqrt((right_x * right_x + right_y * right_y) + right_z * right_z);
    const float scale = 16.0f / (right_length + std::numeric_limits<float>::epsilon());
    const float x = scale * right_x, y = scale * right_y;
    const float sample_z = origin.z + eye_offset;
    return {eye, top, origin, Vector3{origin.x - x, origin.y - y, sample_z},
            Vector3{x + origin.x, y + origin.y, sample_z}};
}
namespace {
template <bool live>
float exposure(Vector3 position, std::uint32_t target, const std::array<Vector3, 5>& samples,
               const void* entity, const engine::DecodedOffset* origin_offset,
               GrenadeDamageEngine& engine) {
    const float first = grenade_surface_transmission(position, samples[0], target, engine) * 0.4f;
    const float second =
        grenade_surface_transmission(position, samples[1], target, engine) * 0.2f + first;
    auto third_sample = samples[2];
    if constexpr (live)
        std::memcpy(&third_sample,
                    static_cast<const std::byte*>(entity) + std::ptrdiff_t(*origin_offset),
                    sizeof third_sample);
    const float third =
        grenade_surface_transmission(position, third_sample, target, engine) * 0.2f + second;
    const float fourth =
        grenade_surface_transmission(position, samples[3], target, engine) * 0.1f + third;
    return grenade_surface_transmission(position, samples[4], target, engine) * 0.1f + fourth;
}
// Exact 13 vectors at 434745F0..4347468C. The missing eighth diagonal is
// intentional; synthesizing a symmetric fourteen-direction set changes it.
constexpr std::array<Vector3, 13> offset_directions{{{1, 0, 0},
                                                     {-1, 0, 0},
                                                     {0, 1, 0},
                                                     {0, -1, 0},
                                                     {0, 0, 1},
                                                     {0, 0, -1},
                                                     {1, 1, 1},
                                                     {1, -1, 1},
                                                     {-1, 1, 1},
                                                     {-1, -1, 1},
                                                     {1, 1, -1},
                                                     {1, -1, -1},
                                                     {-1, -1, -1}}};
}

bool grenade_damage_should_hit(GrenadeDamageFilterKind kind,
                               const GrenadeDamageFilterEntity& entity) noexcept {
    if (!entity.base_filter_accepts)
        return false;
    if (kind == GrenadeDamageFilterKind::ignore_entity)
        return true;
    return entity.converted_entity_present && !entity.virtual_278 && !entity.virtual_298;
}
float grenade_player_view_height(float duck) noexcept {
    const float squared = duck * duck;
    const float doubled = squared + squared;
    const float smooth = squared * 3.0f - doubled * duck;
    return (1.0f - smooth) * 18.0f + 46.0f;
}
float grenade_surface_transmission(Vector3 explosion, Vector3 sample, std::uint32_t target,
                                   GrenadeDamageEngine& engine) {
    const auto ray = line(explosion, sample);
    auto trace = engine.trace(ray, 0x46004003U, {GrenadeDamageFilterKind::ignore_entity, target});
    if (trace.fraction == 1.0f)
        return 1.0f;
    const auto world = engine.world_entity();
    const auto obstacle = trace.entity;
    if (obstacle == world || !obstacle || obstacle == target)
        return 0.0f;
    const auto owner = engine.owner_handle(obstacle);
    const auto owner_entity = owner == 0xffffffffU ? 0U : engine.entity_for_handle(owner);
    if (owner_entity == target)
        return 0.0f;
    trace = engine.trace(ray, 1U, {GrenadeDamageFilterKind::ignore_entity, 0});
    if (trace.fraction != 1.0f)
        return 0.0f;
    const auto object = engine.physics_object(obstacle);
    if (!object)
        return 0.75f;
    const auto material = engine.physics_material(object);
    const float normalized = engine.surface_density(material) / 3000.0f;
    if (normalized < 0.0f)
        return 1.0f;
    if (normalized >= 0.0f && normalized < 1.0f)
        return float(1.0 - double(normalized));
    return 0.0f;
}
namespace {
template <bool live>
GrenadeDamageResult evaluate_geometry(Vector3 explosion, std::uint32_t target,
                                      Vector3 distance_point, const std::array<Vector3, 5>& samples,
                                      const void* entity,
                                      const engine::DecodedOffset* origin_offset,
                                      GrenadeDamageEngine& engine, const GrenadeDamageMath& math) {
    GrenadeDamageResult result;
    result.sample_position = explosion;
    result.distance_point = distance_point;
    const auto delta = subtract(result.distance_point, explosion);
    if (!(squared_yxz(delta) < 122500.0f))
        return result;
    result.in_range = true;
    result.exposure = exposure<live>(explosion, target, samples, entity, origin_offset, engine);
    if (result.exposure < 1.0f) {
        float best = result.exposure;
        auto best_position = explosion;
        for (const auto& direction : offset_directions) {
            const float inverse =
                1.0f / (length_xyz(direction) + std::numeric_limits<float>::epsilon());
            const Vector3 end{(direction.x * inverse) * 8.0f + explosion.x,
                              (direction.y * inverse) * 8.0f + explosion.y,
                              (direction.z * inverse) * 8.0f + explosion.z};
            const auto trace = engine.trace(line(explosion, end), 0x42004003U,
                                            {GrenadeDamageFilterKind::exclude_players, target});
            auto candidate = trace.end;
            if (trace.fraction < 1.0f || trace.all_solid || trace.start_solid)
                candidate = {(explosion.x - trace.end.x) * 0.05f + trace.end.x,
                             (explosion.y - trace.end.y) * 0.05f + trace.end.y,
                             (explosion.z - trace.end.z) * 0.05f + trace.end.z};
            const float next =
                exposure<live>(candidate, target, samples, entity, origin_offset, engine);
            if (next > best) {
                best = next;
                best_position = candidate;
            }
        }
        if (best > result.exposure) {
            result.exposure = best;
            result.sample_position = best_position;
        }
    }
    if (result.exposure > 0.0f) {
        const float distance = length_xyz(subtract(result.distance_point, result.sample_position));
        const float exponent = (negative(distance) * distance) / 27222.220703125f;
        result.damage = (math.exponential(exponent) * 99.0f) * result.exposure;
    }
    return result;
}
}
GrenadeDamageResult evaluate_he_damage_geometry(Vector3 explosion, std::uint32_t target,
                                                Vector3 distance_point,
                                                const std::array<Vector3, 5>& samples,
                                                GrenadeDamageEngine& engine,
                                                const GrenadeDamageMath& math) {
    return evaluate_geometry<false>(explosion, target, distance_point, samples, nullptr, nullptr,
                                    engine, math);
}
GrenadeDamageResult
evaluate_he_damage_live_geometry(Vector3 explosion, std::uint32_t target, Vector3 distance_point,
                                 const std::array<Vector3, 5>& samples, const void* entity,
                                 const engine::DecodedOffset& origin_offset,
                                 GrenadeDamageEngine& engine, const GrenadeDamageMath& math) {
    return evaluate_geometry<true>(explosion, target, distance_point, samples, entity,
                                   &origin_offset, engine, math);
}
GrenadeDamageResult evaluate_he_damage(Vector3 explosion, const GrenadeDamagePlayer& player,
                                       GrenadeDamageEngine& engine, const GrenadeDamageMath& math) {
    GrenadeDamageResult result;
    result.sample_position = explosion;
    const float height = grenade_player_view_height(player.duck_amount) * 0.85f;
    result.distance_point = {player.distance_origin.x, player.distance_origin.y,
                             height + player.distance_origin.z};
    if (!(squared_yxz(subtract(result.distance_point, explosion)) < 122500.0f))
        return result;
    Vector3 eye;
    if (player.alternate_eye)
        eye = {player.view_offset.x + player.virtual_origin.x,
               player.virtual_origin.y + player.view_offset.y,
               player.virtual_origin.z + player.view_offset.z};
    else
        eye = {player.origin.x, player.origin.y,
               grenade_player_view_height(player.duck_amount) + player.origin.z};
    result = evaluate_he_damage_geometry(
        explosion, player.entity, result.distance_point,
        make_grenade_damage_samples(player.origin, eye, eye.z - player.origin.z,
                                    {player.origin.x, player.origin.y, player.origin.z + 71.0f},
                                    player.absolute_angles, math),
        engine, math);
    if (result.exposure > 0.0f && player.armor > 0)
        result.damage *= 0.6f;
    return result;
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_engine.cpp
// ============================================================================

namespace skeet::esp {
namespace {
/* [removed duplicate field] */
/* [removed duplicate identity] */
void* grenade_pointer(std::uint32_t value) noexcept {
    return reinterpret_cast<void*>(std::uintptr_t(value));
}
engine::TraceRay native_ray(const GrenadeRay& source) noexcept {
    engine::TraceRay ray;
    std::memcpy(ray.start.data(), &source.start, sizeof source.start);
    std::memcpy(ray.delta.data(), &source.delta, sizeof source.delta);
    std::memcpy(ray.extents.data(), &source.extents, sizeof source.extents);
    if (!source.is_ray) {
        // Native symmetric hull construction adds the zero center offset to
        // start, then negates that offset. Preserve the resulting signed zeros.
        for (std::size_t i = 0; i < 3; ++i)
            ray.start[i] = _mm_cvtss_f32(_mm_add_ss(_mm_set_ss(ray.start[i]), _mm_setzero_ps()));
        constexpr auto negative_zero = std::bit_cast<float>(0x80000000u);
        ray.start_offset = {negative_zero, negative_zero, negative_zero, 0};
    }
    ray.is_ray = source.is_ray;
    ray.swept = source.swept;
    return ray;
}
#if defined(_M_IX86) || defined(__i386__)
#define GRENADE_MEMBER __thiscall
#else
#define GRENADE_MEMBER
#endif

void* const* required_table(void* const* table) {
    if (!table)
        throw std::logic_error("Required grenade trace filter table is not bound");
    return table;
}
void initialize_trace(MovementCollisionTrace& result) {
    // Native433B7469 and the repeated HE trace records initialize these fields
    // before the engine fills the complete0x54-byte trace. Fraction is one.
    result.fraction = 1.0f;
    result.all_solid = 0;
    result.start_solid = 0;
    result.hitgroup = -1;
    result.entity = 0;
    result.hitbox = -1;
}

struct ProjectileFilter {
    engine::TraceSkipFilterPrefix prefix;
    std::uint32_t field_10 = 0;
    std::int16_t kind;
    std::uint16_t bounces;
    const GrenadeEngineBindings* bindings;
    static bool GRENADE_MEMBER should_hit(void* object, void* entity, std::uint32_t mask) {
        const auto& self = *static_cast<ProjectileFilter*>(object);
        const auto table = required_table(self.bindings->entity_filter_vtable);
        return reinterpret_cast<engine::MemberFunction<bool, void*, std::uint32_t>>(table[0])(
            object, entity, mask);
    }
    static std::int32_t GRENADE_MEMBER trace_type(void* object) {
        const auto& self = *static_cast<ProjectileFilter*>(object);
        const auto table = required_table(self.bindings->entity_filter_vtable);
        return reinterpret_cast<engine::MemberFunction<std::int32_t>>(table[1])(object);
    }
    static void GRENADE_MEMBER set_entity(void* object, void* entity) {
        const auto& self = *static_cast<ProjectileFilter*>(object);
        const auto table = required_table(self.bindings->entity_filter_vtable);
        reinterpret_cast<engine::MemberFunction<void, void*>>(table[2])(object, entity);
    }
    static void GRENADE_MEMBER set_group(void* object, std::int32_t group) {
        const auto& self = *static_cast<ProjectileFilter*>(object);
        const auto table = required_table(self.bindings->entity_filter_vtable);
        reinterpret_cast<engine::MemberFunction<void, std::int32_t>>(table[3])(object, group);
    }
    static void GRENADE_MEMBER set_extra(void* object, std::uint32_t extra) {
        const auto& self = *static_cast<ProjectileFilter*>(object);
        const auto table = required_table(self.bindings->entity_filter_vtable);
        reinterpret_cast<engine::MemberFunction<void, std::uint32_t>>(table[4])(object, extra);
    }
    static void* const* table() {
        static void* const methods[]{
            reinterpret_cast<void*>(should_hit), reinterpret_cast<void*>(trace_type),
            reinterpret_cast<void*>(set_entity), reinterpret_cast<void*>(set_group),
            reinterpret_cast<void*>(set_extra)};
        return methods;
    }
    ProjectileFilter(const GrenadeTraceFilter& source, const GrenadeEngineBindings& actual)
        : prefix{table(), pointer(source.ignored_entity), source.collision_group, 0},
          kind(static_cast<std::int16_t>(source.kind)), bounces(source.projectile_flags),
          bindings(&actual) {}
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(ProjectileFilter, kind) == 0x14);
static_assert(offsetof(ProjectileFilter, bounces) == 0x16);
static_assert(offsetof(ProjectileFilter, field_10) == 0x10);
#endif
struct DamageFilter {
    engine::TraceSkipFilterPrefix prefix;
    const GrenadeEngineBindings* bindings;
    GrenadeDamageFilterKind kind;
    static bool GRENADE_MEMBER should_hit(void* object, void* entity, std::uint32_t mask) {
        const auto& self = *static_cast<DamageFilter*>(object);
        const auto table = required_table(self.bindings->skip_filter_vtable);
        if (!reinterpret_cast<engine::MemberFunction<bool, void*, std::uint32_t>>(table[0])(
                object, entity, mask))
            return false;
        if (self.kind == GrenadeDamageFilterKind::ignore_entity)
            return true;
        // Full4333D32B conversion, followed by4334EA3E's two client exclusions.
        if (engine::invoke<bool>(self.bindings->static_props, 3, entity))
            return false;
        auto* converted = engine::invoke<void*>(entity, 7);
        if (!converted || engine::invoke<bool>(converted, 158))
            return false;
        return !engine::invoke<bool>(converted, 166);
    }
    static std::int32_t GRENADE_MEMBER trace_type(void* object) {
        const auto& self = *static_cast<DamageFilter*>(object);
        const auto table = required_table(self.bindings->skip_filter_vtable);
        return reinterpret_cast<engine::MemberFunction<std::int32_t>>(table[1])(object);
    }
    static void GRENADE_MEMBER set_entity(void* object, void* entity) {
        const auto& self = *static_cast<DamageFilter*>(object);
        const auto table = required_table(self.bindings->skip_filter_vtable);
        reinterpret_cast<engine::MemberFunction<void, void*>>(table[2])(object, entity);
    }
    static void GRENADE_MEMBER set_group(void* object, std::int32_t group) {
        const auto& self = *static_cast<DamageFilter*>(object);
        const auto table = required_table(self.bindings->skip_filter_vtable);
        reinterpret_cast<engine::MemberFunction<void, std::int32_t>>(table[3])(object, group);
    }
    static void* const* table() {
        static void* const methods[]{
            reinterpret_cast<void*>(should_hit), reinterpret_cast<void*>(trace_type),
            reinterpret_cast<void*>(set_entity), reinterpret_cast<void*>(set_group)};
        return methods;
    }
    DamageFilter(const GrenadeDamageFilter& source, const GrenadeEngineBindings& actual)
        : prefix{table(), pointer(source.ignored_entity), 0, 0}, bindings(&actual),
          kind(source.kind) {}
};
struct SmokeCollector {
    void* const* vtable;
    void** entities;
    std::int32_t capacity;
    const void* filter_class;
    std::int32_t count = 0;
    const EngineGrenades* engine;
    static std::int32_t GRENADE_MEMBER enumerate(void* object, void* handle_entity) {
        auto& self = *static_cast<SmokeCollector*>(object);
        // Native433E92B5 captures entity-list virtual4 before asking the
        // IHandleEntity for its handle, then reloads the list receiver.
        const auto& cells = self.engine->bindings();
        const auto table = *static_cast<void***>(cells.entity_list);
        const auto resolve =
            reinterpret_cast<engine::MemberFunction<void*, std::uint32_t>>(table[4]);
        const auto* handle = engine::invoke<const std::uint32_t*>(handle_entity, 2);
        const auto reference_handle = *handle;
        auto* entity = resolve(cells.entity_list, reference_handle);
        if (!entity)
            return 0;
        auto* base = engine::invoke<void*>(entity, 7);
        if (!base || (self.filter_class && self.engine->client_class(base) != self.filter_class))
            return 0;
        if (self.count >= self.capacity)
            return 1;
        self.entities[self.count++] = base;
        return 0;
    }
    static void* const* table() {
        static void* const methods[]{reinterpret_cast<void*>(enumerate)};
        return methods;
    }
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(SmokeCollector, count) == 0x10);
static_assert(offsetof(SmokeCollector, filter_class) == 0xC);
#endif
}

void* EngineGrenades::by_index(std::int32_t index) const {
    return engine::invoke<void*>(bindings_.entity_list, 3, index);
}
void* EngineGrenades::by_handle(std::uint32_t handle) const {
    return engine::invoke<void*>(bindings_.entity_list, 4, handle);
}
const void* EngineGrenades::client_class(void* entity) const {
    return engine::invoke<const void*>(static_cast<std::byte*>(entity) + 8, 2);
}
std::int32_t EngineGrenades::index(void* entity) const {
    return engine::invoke<std::int32_t>(static_cast<std::byte*>(entity) + 8, 10);
}
bool EngineGrenades::player(void* entity) const {
    return engine::invoke<bool>(entity, 158);
}
bool EngineGrenades::dormant(void* entity) const {
    return engine::invoke<bool>(static_cast<std::byte*>(entity) + 8, 9);
}
Vector3 EngineGrenades::live_origin(void* entity) const {
    return *engine::invoke<const Vector3*>(entity, 10);
}
GrenadeTraceProvider EngineGrenades::provider(std::uint8_t* flag) noexcept {
    return {this, trace_projectile, flag};
}
GrenadeTrace EngineGrenades::trace_projectile(void* context, const GrenadeRay& source,
                                              std::uint32_t mask,
                                              const GrenadeTraceFilter& filter) {
    auto& self = *static_cast<EngineGrenades*>(context);
    auto ray = native_ray(source);
    ProjectileFilter actual(filter, self.bindings_);
    MovementCollisionTrace result;
    initialize_trace(result);
    engine::invoke<void>(self.bindings_.engine_trace, 5, &ray, mask, &actual, &result);
    GrenadeTrace trace;
    std::memcpy(&trace.end, &result.end, sizeof trace.end);
    std::memcpy(&trace.normal, &result.plane_normal, sizeof trace.normal);
    std::memcpy(&trace.fraction, &result.fraction, sizeof trace.fraction);
    trace.contents = result.contents;
    trace.surface_flags = result.surface_flags;
    trace.all_solid = result.all_solid;
    trace.start_solid = result.start_solid;
    // The mandatory live collision overload performs native entity getters at
    // their actual post-trace sites. Trace packing must not pre-read them.
    trace.entity.identity = result.entity;
    return trace;
}
GrenadeDamageTrace EngineGrenades::trace(const GrenadeRay& source, std::uint32_t mask,
                                         const GrenadeDamageFilter& filter) {
    if (filter.kind != GrenadeDamageFilterKind::ignore_entity &&
        filter.kind != GrenadeDamageFilterKind::exclude_players)
        throw std::logic_error("Invalid grenade damage trace filter kind");
    auto ray = native_ray(source);
    DamageFilter actual(filter, bindings_);
    MovementCollisionTrace result;
    initialize_trace(result);
    engine::invoke<void>(bindings_.engine_trace, 5, &ray, mask, &actual, &result);
    GrenadeDamageTrace trace;
    std::memcpy(&trace.end, &result.end, sizeof trace.end);
    std::memcpy(&trace.fraction, &result.fraction, sizeof trace.fraction);
    trace.all_solid = result.all_solid != 0;
    trace.start_solid = result.start_solid != 0;
    trace.entity = result.entity;
    return trace;
}
std::uint32_t EngineGrenades::world_entity() {
    return identity(by_index(0));
}
std::uint32_t EngineGrenades::owner_handle(std::uint32_t entity) {
    return field<std::uint32_t>(grenade_pointer(entity), bindings_.offsets.owner_handle);
}
std::uint32_t EngineGrenades::entity_for_handle(std::uint32_t handle) {
    return identity(by_handle(handle));
}
std::uint32_t EngineGrenades::physics_object(std::uint32_t entity) {
    return identity(field<void*>(grenade_pointer(entity), bindings_.offsets.physics_object));
}
std::int32_t EngineGrenades::physics_material(std::uint32_t object) {
    return engine::invoke<std::int32_t>(pointer(object), 38);
}
float EngineGrenades::surface_density(std::int32_t material) {
    float density, thickness, friction, elasticity;
    engine::invoke<void>(bindings_.surface_properties, 4, material, &density, &thickness, &friction,
                         &elasticity);
    return density;
}
std::span<const Vector3> EngineGrenades::fire_cells(void* entity) {
    cells_.clear();
    const auto& o = bindings_.offsets;
    const auto origin = field<Vector3>(entity, o.origin);
    const auto count = field<std::int32_t>(entity, o.fire_count);
    for (std::int32_t ordinal = 0; ordinal < count; ++ordinal) {
        if (!field<std::uint8_t>(entity, o.fire_active + ordinal))
            continue;
        cells_.push_back({float(field<std::int32_t>(entity, o.fire_x + ordinal * 4)) + origin.x,
                          float(field<std::int32_t>(entity, o.fire_y + ordinal * 4)) + origin.y,
                          float(field<std::int32_t>(entity, o.fire_z + ordinal * 4)) + origin.z});
    }
    return cells_;
}
bool EngineGrenades::snapshot(void* entity, GrenadeWarningSnapshot& result) {
    if (!entity)
        return false;
    const auto& o = bindings_.offsets;
    result = {};
    result.identity = identity(entity);
    result.native_class = identity(client_class(entity));
    result.reference_handle = *engine::invoke<const std::uint32_t*>(entity, 2);
    result.index = index(entity);
    result.player = player(entity);
    result.dormant = dormant(entity);
    result.origin = field<Vector3>(entity, o.origin);
    result.virtual_origin = live_origin(entity);
    result.flags = field<std::uint32_t>(entity, o.effects);
    if (result.player) {
        result.team = field<std::int32_t>(entity, o.team);
        result.survival_team = field<std::int32_t>(entity, o.survival_team);
        if (result.dormant && bindings_.player_resource_slot) {
            auto* resource = field<void*>(bindings_.player_resource_slot, 0);
            if (resource && field<std::uint8_t>(resource, o.cached_team_valid + result.index)) {
                result.cached_team_valid = true;
                result.cached_team =
                    field<std::int32_t>(resource, o.cached_team + result.index * 4);
            }
        }
        result.velocity = field<Vector3>(entity, o.velocity);
        return true;
    }
    const auto native_class = pointer(result.native_class);
    if (native_class == bindings_.base_projectile_class ||
        native_class == bindings_.molotov_projectile_class) {
        result.velocity = field<Vector3>(entity, o.velocity);
        result.initial_velocity = field<Vector3>(entity, o.initial_velocity);
        result.simulation_time = field<float>(entity, o.simulation_time);
        result.explosion_effect_tick = field<std::int32_t>(entity, o.explosion_effect_tick);
        result.thrower_handle = field<std::uint32_t>(entity, o.thrower_handle);
        result.collision_group = field<std::int32_t>(entity, o.collision_group);
        result.projectile_flags = field<std::uint16_t>(entity, o.projectile_flags);
        if (native_class == bindings_.base_projectile_class)
            result.damage = field<float>(entity, o.damage);
    } else if (native_class == bindings_.inferno_class) {
        result.owner_handle = field<std::uint32_t>(entity, o.owner_handle);
        result.inferno_start_tick = field<std::int32_t>(entity, o.inferno_start_tick);
        result.fire_cells = fire_cells(entity);
    }
    return true;
}
bool EngineGrenades::snapshot_for_handle(std::uint32_t handle, GrenadeWarningSnapshot& result) {
    return snapshot(by_handle(handle), result);
}
bool EngineGrenades::snapshot_for_index(std::uint32_t index, GrenadeWarningSnapshot& result) {
    return snapshot(by_index(std::int32_t(index)), result);
}
std::span<void* const> EngineGrenades::collect_smokes(Vector3 center, float radius,
                                                      std::uint32_t mask, bool coarse) {
    const auto filter_class = bindings_.smoke_projectile_class;
    std::array<void*, 16> collected;
    SmokeCollector collector{SmokeCollector::table(), collected.data(), 16, filter_class, 0, this};
    engine::invoke<void>(bindings_.spatial_partition, 13, mask, &center, radius, coarse,
                         &collector);
    std::memcpy(smoke_entities_.data(), collected.data(),
                std::size_t(collector.count) * sizeof(void*));
    collected_smoke_class_ = filter_class;
    return {smoke_entities_.data(), std::size_t(collector.count)};
}
std::span<const GrenadeWarningPartitionEntity>
EngineGrenades::partition_entities(Vector3 center, float radius, std::uint32_t mask, bool coarse) {
    const auto entities = collect_smokes(center, radius, mask, coarse);
    for (std::size_t ordinal = 0; ordinal < entities.size(); ++ordinal) {
        auto* entity = entities[ordinal];
        auto& view = smoke_views_[ordinal];
        view.resolved = true;
        view.base_entity_present = true;
        view.native_class = identity(collected_smoke_class_);
        view.smoke_active = field<std::uint8_t>(entity, bindings_.offsets.smoke_active) != 0;
        if (view.smoke_active)
            view.origin = live_origin(entity);
    }
    return {smoke_views_.data(), entities.size()};
}
#undef GRENADE_MEMBER
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_engine_offsets.cpp
// ============================================================================

namespace skeet::esp {
// Native43337BC0/43341683/433BB853 operands. Property use sites perform the
// actual decode through these proxies; construction reads no offset words.
GrenadeEngineOffsets::GrenadeEngineOffsets(const engine::NativeOffsetGlobals& g)
    : origin{g, 0x434670CC, 0x43468BCC, 0xA9EE6062, 0},
      velocity{g, 0x4346A8E4, 0x434688CC, 0x36375C23, 0},
      initial_velocity{g, 0x434683E8, 0x434693C0, 0x434E9E95, 0},
      simulation_time{g, 0x43469198, 0x4346A538, 0xD5FD2E2C, 0},
      effects{g, 0x43468B50, 0x43467B48, 0x38CA5228, 0},
      explosion_effect_tick{g, 0x43467210, 0x43467630, 0x01E80986, 0},
      thrower_handle{g, 0x43468C18, 0x4346A888, 0xDC743513, 0},
      owner_handle{g, 0x43467FCC, 0x43467938, 0x8D1BD8D0, 0},
      damage{g, 0x434674EC, 0x4346894C, 0x727317C4, 0},
      collision_group{g, 0x434683C8, 0x4346758C, 0xE4E2D1A7, 0},
      projectile_flags{g, 0x43467488, 0x43467DEC, 0x7418868F, 0},
      inferno_start_tick{g, 0x4346A854, 0x434678B4, 0x42088943, 0},
      team{g, 0x434688B0, 0x4346A518, 0x8B5586F6, 0},
      survival_team{g, 0x434678AC, 0x43467B7C, 0x430C8993, 0},
      cached_team_valid{g, 0x43467CB8, 0x4346A814, 0x9571D5F5, 0},
      cached_team{g, 0x434684F0, 0x4346A558, 0xC1C49A34, 0},
      physics_object{g, 0x4346720C, 0x43468298, 0xA249061A, 0xFFFFFFF4},
      fire_count{g, 0x43468E28, 0x4346893C, 0x3584E3A5, 0},
      fire_active{g, 0x43467D4C, 0x4346711C, 0xFC1EF3A1, 0},
      fire_x{g, 0x43468A58, 0x43467574, 0x24A16FF6, 0},
      fire_y{g, 0x43469058, 0x4346892C, 0x94E8C7F7, 0},
      fire_z{g, 0x434671E8, 0x43468DDC, 0xD7600AAC, 0},
      smoke_active{g, 0x4346A530, 0x43467564, 0x58AE2423, 0} {}

GrenadeEngineOffsets capture_grenade_engine_offsets(const engine::NativeOffsetGlobals& globals) {
    return GrenadeEngineOffsets(globals);
}
} //namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_environment.cpp
// ============================================================================

namespace skeet::esp {
namespace {
float convar_float(const void* object) noexcept {
    for (;;) {
        const void* parent;
        std::memcpy(&parent, static_cast<const std::byte*>(object) + 0x1C, sizeof(parent));
        if (parent == object)
            break;
        object = parent;
    }
    std::uint32_t encoded;
    std::memcpy(&encoded, static_cast<const std::byte*>(object) + 0x2C, sizeof(encoded));
    return std::bit_cast<float>(encoded ^ std::uint32_t(reinterpret_cast<std::uintptr_t>(object)));
}
}
GrenadeEnvironmentOffsets::GrenadeEnvironmentOffsets(const engine::NativeOffsetGlobals& g)
    : fire_origin_a9ee6062{g, 0x434670CC, 0x43468BCC, 0xA9EE6062, 0},
      fire_count_3584e3a5{g, 0x43468E28, 0x4346893C, 0x3584E3A5, 0},
      fire_active_fc1ef3a1{g, 0x43467D4C, 0x4346711C, 0xFC1EF3A1, 0},
      fire_x_24a16ff6{g, 0x43468A58, 0x43467574, 0x24A16FF6, 0},
      fire_y_94e8c7f7{g, 0x43469058, 0x4346892C, 0x94E8C7F7, 0},
      fire_z_d7600aac{g, 0x434671E8, 0x43468DDC, 0xD7600AAC, 0},
      collision_flags_b7e9c94e{g, 0x43467044, 0x4346A3BC, 0xB7E9C94E, 0},
      movement_type_7501cb91{g, 0x43468BD4, 0x4346A678, 0x7501CB91, 0},
      state_f0374295{g, 0x4346A360, 0x43468314, 0xF0374295, 0},
      owner_8d1bd8d0{g, 0x43467FCC, 0x43467938, 0x8D1BD8D0, 0},
      activity_8dbc9874{g, 0x43468BD0, 0x43469278, 0x8DBC9874, 0},
      activity_68140cd8{g, 0x43467948, 0x4346919C, 0x68140CD8, 0} {}

GrenadeEnvironmentOffsets
capture_grenade_environment_offsets(const engine::NativeOffsetGlobals& globals) {
    return GrenadeEnvironmentOffsets(globals);
}
float GrenadeEnvironmentBindings::gravity() const noexcept {
    return convar_float(gravity_convar.get());
}
float GrenadeEnvironmentBindings::incendiary_duration() const noexcept {
    return convar_float(incendiary_duration_convar.get());
}
float GrenadeEnvironmentBindings::incendiary_max_slope() const noexcept {
    return convar_float(incendiary_max_slope_convar.get());
}
std::uint32_t GrenadeEnvironmentBindings::world_entity() const {
    return std::uint32_t(
        reinterpret_cast<std::uintptr_t>(engine::invoke<void*>(entity_list, 3, std::int32_t(0))));
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_live.cpp
// ============================================================================

namespace skeet::esp {
using namespace grenade_live_detail;
GrenadeLiveScene::Storage::Storage(const GrenadeLiveBindings& b)
    : bindings(b), raw(b.queries.grenades()), offsets(b.queries.offsets()),
      auxiliary_context{b.player_adjustments,
                        b.convars.game_type,
                        b.convars.teammates_are_enemies,
                        b.local_player,
                        raw.bindings().entity_list,
                        raw,
                        b.math.trigonometry,
                        b.arc_tangent,
                        b.maximum_players,
                        raw.bindings().smoke_projectile_class,
                        GrenadeAuxiliaryLiveOffsets(b.queries.offsets()),
                        b.trace_flag} {}
bool GrenadeLiveScene::Storage::enemy(void* entity) const {
    auto* local = bindings.local_player;
    if (!local || !entity || local == entity)
        return false;
    const auto& offsets = raw.bindings().offsets;
    if (convar_integer(bindings.convars.game_type) == 6) {
        const auto offset = std::ptrdiff_t(offsets.survival_team);
        const auto team = field<std::int32_t>(local, offset);
        return team == -1 || team != field<std::int32_t>(entity, offset);
    }
    const auto offset = std::ptrdiff_t(offsets.team);
    const auto team = field<std::int32_t>(local, offset);
    const auto other = field<std::int32_t>(entity, offset);
    if (team == 2) {
        if (other == 3)
            return true;
    } else if (team == 3) {
        if (other == 2)
            return true;
    } else
        return false;
    return convar_integer(bindings.convars.teammates_are_enemies) != 0;
}
bool GrenadeLiveScene::Storage::game_flag(std::ptrdiff_t offset) const {
    auto* slot = bindings.game_rules_slot;
    if (!slot)
        return false;
    auto* rules = field<void*>(slot, 0);
    return rules && field<std::uint8_t>(rules, offset) != 0;
}
std::int32_t GrenadeLiveScene::Storage::team(void* entity) {
    const auto& o = raw.bindings().offsets;
    if (!raw.dormant(entity)) {
        if (convar_integer(bindings.convars.game_type) == 6)
            return field<std::int32_t>(entity, o.survival_team);
    } else {
        const auto index = raw.index(entity);
        auto* slot = raw.bindings().player_resource_slot;
        if (slot) {
            auto* resource = field<void*>(slot, 0);
            if (resource && field<std::uint8_t>(resource, o.cached_team_valid + index))
                return field<std::int32_t>(resource, o.cached_team + index * 4);
        }
    }
    return field<std::int32_t>(entity, o.team);
}
void* GrenadeLiveScene::Storage::thrower(void* projectile, std::uint32_t type) {
    const auto& o = raw.bindings().offsets;
    if (type > 2)
        return nullptr;
    const auto handle =
        field<std::uint32_t>(projectile, type < 2 ? o.thrower_handle : o.owner_handle);
    if (handle == 0xffffffffu)
        return nullptr;
    auto* owner = raw.by_handle(handle);
    return owner && raw.player(owner) ? owner : nullptr;
}
bool GrenadeLiveScene::Storage::admitted(std::uint32_t type, void* owner, void* selected) {
    if (!owner || !raw.player(owner))
        return true;
    const auto& c = bindings.convars;
    if (owner == selected)
        return type != 0 || convar_float(c.self_he_damage) != 0.0f;
    const auto owner_team = team(owner);
    const auto local_team = team(selected);
    if (owner_team != local_team || convar_integer(bindings.convars.teammates_are_enemies))
        return true;
    if (!convar_integer(c.friendly_fire))
        return false;
    return convar_float(type == 0 ? c.teammate_he_damage : c.teammate_fire_damage) != 0.0f;
}
float GrenadeLiveScene::Storage::damage_scale(std::uint32_t handle, std::uint32_t type) {
    auto* projectile = handle == 0xffffffffu ? nullptr : raw.by_handle(handle);
    auto* local = bindings.local_player;
    if (!projectile || !local)
        return 1.0f;
    auto* owner = thrower(projectile, type);
    const auto friendly = convar_integer(bindings.convars.friendly_fire) != 0;
    const auto& o = raw.bindings().offsets;
    bool teammate = false;
    if (owner && owner != local &&
        field<std::int32_t>(local, o.team) == field<std::int32_t>(owner, o.team)) {
        const auto owner_index = raw.index(owner);
        const auto local_index = raw.index(local);
        if (owner_index == local_index)
            teammate = true;
        else {
            const auto owner_team = field<std::int32_t>(owner, o.team);
            const auto local_team = field<std::int32_t>(local, o.team);
            teammate = (!convar_integer(bindings.convars.teammates_are_enemies) &&
                        owner_team == local_team);
        }
    }
    if (teammate && (!friendly || game_flag(offsets.game_player_override)))
        return 0.0f;
    if (owner) {
        const auto owner_index = raw.index(owner);
        const auto local_index = raw.index(local);
        if (owner_index != local_index)
            (void)convar_integer(bindings.convars.teammates_are_enemies);
        if (owner == local && type == 0)
            return convar_float(bindings.convars.self_he_damage);
    }
    if (!teammate)
        return 1.0f;
    const auto scale = convar_float(type == 0 ? bindings.convars.teammate_he_damage
                                              : bindings.convars.teammate_fire_damage);
    return game_flag(offsets.teammate_damage_disabled) ? 0.0f : scale;
}
GrenadeLiveScene::GrenadeLiveScene(const GrenadeLiveBindings& bindings)
    : storage_(std::make_unique<Storage>(bindings)) {}
GrenadeLiveScene::~GrenadeLiveScene() = default;
GrenadeTraceProvider GrenadeLiveScene::trace() noexcept {
    return storage_->raw.provider(storage_->bindings.trace_flag);
}
const GrenadeEnvironmentBindings& GrenadeLiveScene::environment() const noexcept {
    return storage_->bindings.queries.environment();
}
GrenadeWarningEngine& GrenadeLiveScene::engine() noexcept {
    return storage_->raw;
}
MovementCollisionEngine& GrenadeLiveScene::movement() noexcept {
    return storage_->bindings.queries.movement();
}
const GrenadeDamageMath& GrenadeLiveScene::damage_math() const noexcept {
    return storage_->bindings.math;
}
const GrenadeAuxiliaryLiveContext& GrenadeLiveScene::auxiliary() {
    return storage_->auxiliary_context;
}
GrenadeDamageSelection GrenadeLiveScene::select_damage(Vector3 position,
                                                       const IntegerConfig& threshold) {
    return storage_->select_damage(position, threshold);
}
void GrenadeLiveScene::assess_warning(GrenadeWarningRecord& record, void* player,
                                      std::uint32_t& buttons) {
    storage_->assess(record, player, buttons);
}
void GrenadeLiveScene::update_remote_warnings(GrenadeWarningOwner& owner,
                                              const GrenadeEspConfig& config, void* player,
                                              std::uintptr_t incoming) {
    storage_->update(owner, config, player, incoming);
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_live_damage.cpp
// ============================================================================

namespace skeet::esp {
using namespace grenade_live_detail;
GrenadeDamageResult GrenadeLiveScene::Storage::damage(void* entity, Vector3 explosion,
                                                      Vector3 selected_origin) {
    const auto& b = bindings;
    const auto& o = raw.bindings().offsets;
    const auto duck = field<float>(entity, offsets.duck_amount);
    const auto height = grenade_player_view_height(duck) * 0.85f;
    const Vector3 distance_point{selected_origin.x, selected_origin.y, height + selected_origin.z};
    if (!(squared(difference(distance_point, explosion)) < 122500.0f)) {
        GrenadeDamageResult result;
        result.sample_position = explosion;
        result.distance_point = distance_point;
        return result;
    }
    Vector3 eye;
    if (field<std::uint8_t>(entity, offsets.alternate_eye)) {
        const auto origin = raw.live_origin(entity);
        const auto offset = field<Vector3>(entity, offsets.view_offset);
        eye = {offset.x + origin.x, origin.y + offset.y, origin.z + offset.z};
    } else {
        const auto origin = field<Vector3>(entity, o.origin);
        const auto eye_duck = field<float>(entity, offsets.duck_amount);
        eye = {origin.x, origin.y, grenade_player_view_height(eye_duck) + origin.z};
    }
    auto top = field<Vector3>(entity, o.origin);
    const auto eye_offset = eye.z - top.z;
    top.z += 71.0f;
    const auto angles = *engine::invoke<const Vector3*>(entity, 11);
    const auto body = field<Vector3>(entity, o.origin);
    auto result = evaluate_he_damage_live_geometry(
        explosion, identity(entity), distance_point,
        make_grenade_damage_samples(body, eye, eye_offset, top, angles, b.math), entity, o.origin,
        raw, b.math);
    if (result.exposure > 0.0f && field<std::int32_t>(entity, offsets.armor) > 0)
        result.damage *= 0.6f;
    return result;
}
GrenadeDamageSelection GrenadeLiveScene::Storage::select_damage(Vector3 explosion,
                                                                const IntegerConfig& threshold) {
    GrenadeDamageSelection selection;
    const auto maximum = bindings.maximum_players;
    for (std::int32_t ordinal = 1; ordinal < maximum; ++ordinal) {
        auto* entity = raw.by_index(ordinal);
        if (!entity || !raw.player(entity))
            continue;
        const auto& o = offsets;
        if (field<std::uint8_t>(entity, o.life_state) ||
            field<std::uint8_t>(entity, o.alternate_eye) ||
            field<std::uint8_t>(entity, o.rejection_cde290d2) ||
            field<std::uint8_t>(entity, o.rejection_af649dac) || !enemy(entity))
            continue;
        Vector3 origin;
        if (!raw.dormant(entity))
            origin = field<Vector3>(entity, raw.bindings().offsets.origin);
        else {
            auto* history = bindings.players.get(std::uint32_t(ordinal));
            if (0.0f >= history->geometry.alpha)
                continue;
            origin = history->geometry.origin;
        }
        const auto result = damage(entity, explosion, origin);
        const auto armor = field<std::int32_t>(entity, offsets.armor);
        const auto required = threshold.value;
        const auto health = engine::invoke<std::int32_t>(entity, 122);
        if (consider_grenade_damage(selection, result.damage, armor, health, required, ordinal))
            break;
    }
    return selection;
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_live_hud.cpp
// ============================================================================

namespace skeet::esp {
using namespace grenade_live_detail;
void GrenadeLiveScene::Storage::draw_warnings(ui::RenderQueue& queue,
                                              const GrenadeEspConfig& config,
                                              GrenadeWarningOwner& warnings) {
    if (!config.proximity_warning.value)
        return;
    const auto& b = bindings;
    const auto& o = raw.bindings().offsets;
    for (auto& record : warnings.records) {
        auto* local = b.local_player;
        if (!local || field<std::uint8_t>(local, offsets.life_state))
            continue;
        if (field<std::uint8_t>(local, offsets.movement_type) == 10) {
            const auto mode = field<std::int32_t>(local, offsets.observer_mode);
            if (mode == 4 || mode == 5)
                continue;
        }
        auto* entity = record.handle == 0xffffffffu ? nullptr : raw.by_handle(record.handle);
        if (!entity || (field<std::uint32_t>(entity, o.effects) & 0x20u)) {
            record.handle = 0xffffffffu;
            continue;
        }
        if ((record.type < 2 && field<std::int32_t>(entity, o.explosion_effect_tick) != 0) ||
            record.suppressed)
            continue;
        float distance;
        const auto local_origin = raw.live_origin(local);
        if (record.type == 2) {
            auto* inferno = record.handle == 0xffffffffu ? nullptr : raw.by_handle(record.handle);
            if (!inferno)
                continue;
            const auto center = field<Vector3>(inferno, o.origin);
            const auto count = field<std::int32_t>(inferno, o.fire_count);
            distance = std::sqrt(squared(difference(local_origin, center)));
            for (std::int32_t cell = 0; cell < count; ++cell) {
                if (!field<std::uint8_t>(inferno, o.fire_active + cell))
                    continue;
                const Vector3 point{
                    float(field<std::int32_t>(inferno, o.fire_x + cell * 4)) + center.x,
                    float(field<std::int32_t>(inferno, o.fire_y + cell * 4)) + center.y,
                    float(field<std::int32_t>(inferno, o.fire_z + cell * 4)) + center.z};
                const auto next = std::sqrt(squared(difference(point, local_origin)));
                distance = _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(next), _mm_set_ss(distance)));
            }
        } else
            distance = std::sqrt(squared(difference(record.position, local_origin)));
        const GrenadeWarningLiveFrame frame{local,           b.renderer,
                                            b.tick_count,    b.queries.environment().tick_interval,
                                            b.interpolation, b.explosive_texture,
                                            b.fire_texture};
        draw_grenade_warning(queue, b.projection, record, distance, frame);
    }
}
void GrenadeLiveScene::Storage::draw(ui::RenderQueue& queue,
                                     const misc::GrenadePredictionConfig& settings,
                                     const GrenadeEspConfig& config, GrenadeTrajectoryOwner& visual,
                                     misc::GrenadeCommandState& command,
                                     GrenadeWarningOwner& warnings) {
    auto& smooth = command.trajectory_velocity;
    const auto target = command.live_velocity;
    smooth.x += (target.x - smooth.x) * 0.2f;
    smooth.y += (target.y - smooth.y) * 0.2f;
    smooth.z += (target.z - smooth.z) * 0.2f;
    const auto& b = bindings;
    const auto& o = raw.bindings().offsets;
    if (engine::invoke<bool>(b.engine_client, 82) || engine::invoke<bool>(b.engine_client, 93) ||
        game_flag(offsets.game_player_override)) {
        warnings.records.clear();
        return;
    }
    auto trajectory = [&] {
        if (!config.trajectory.value)
            return;
        auto* selected = b.local_player;
        if (selected && field<std::uint8_t>(selected, offsets.life_state) &&
            field<std::uint8_t>(selected, offsets.movement_type) == 10) {
            const auto mode = field<std::int32_t>(selected, offsets.observer_mode);
            if (mode == 4 || mode == 5) {
                const auto handle = field<std::uint32_t>(selected, offsets.observer_target);
                if (handle != 0xffffffffu) {
                    auto* observed = raw.by_handle(handle);
                    if (observed && raw.player(observed))
                        selected = observed;
                }
            }
        }
        if (!selected || field<std::uint8_t>(selected, offsets.movement_type) == 10 ||
            field<std::uint8_t>(selected, offsets.life_state))
            return;
        const auto local = selected == b.local_player;
        if (!local && std::abs(field<float>(selected, offsets.view_angles)) > 50.0f)
            return;
        const auto handle = field<std::uint32_t>(selected, offsets.active_weapon);
        if (handle == 0xffffffffu)
            return;
        auto* weapon = raw.by_handle(handle);
        if (!weapon)
            return;
        const auto kind =
            static_cast<GrenadeKind>(field<std::int16_t>(weapon, offsets.weapon_definition));
        switch (kind) {
        case GrenadeKind::flash:
        case GrenadeKind::explosive:
        case GrenadeKind::smoke:
        case GrenadeKind::molotov:
        case GrenadeKind::decoy:
        case GrenadeKind::incendiary:
        case GrenadeKind::tactical:
        case GrenadeKind::diversion:
            break;
        default:
            return;
        }
        if (local) {
            if (field<std::int32_t>(weapon, offsets.weapon_state) == 190)
                return;
        } else if (field<float>(weapon, offsets.weapon_throw_time) > 0.0f)
            return;
        const auto strength = field<float>(weapon, offsets.weapon_strength);
        visual.path.points.clear();
        visual.path.bounces.clear();
        const auto cached = field<std::uint8_t>(selected, offsets.alternate_eye) != 0;
        Vector3 eye, velocity;
        float start_time = 0.0f;
        if (!cached) {
            velocity = field<Vector3>(selected, o.velocity);
            eye = raw.live_origin(selected);
            eye.z += grenade_player_view_height(field<float>(selected, offsets.duck_amount));
        } else {
            const auto origin = raw.live_origin(selected);
            const auto offset = field<Vector3>(selected, offsets.view_offset);
            eye = {origin.x + offset.x, origin.y + offset.y, origin.z + offset.z};
            velocity = command.trajectory_velocity;
            const auto expected = std::bit_cast<std::int32_t>(std::uint32_t(b.command_count) +
                                                              std::uint32_t(b.last_outgoing));
            if ((velocity.x * velocity.x + velocity.y * velocity.y) + velocity.z * velocity.z >=
                    80.0f &&
                command.captured_command.number == expected) {
                const auto flags = field<std::uint32_t>(selected, offsets.player_flags);
                const auto position = raw.live_origin(selected);
                MovementSimulation simulation{};
                initialize_live_movement_simulation(
                    simulation, selected, {position.x, position.y, position.z},
                    {velocity.x, velocity.y, velocity.z}, flags, 0, b.movement_capture);
                auto count =
                    truncate(float(std::ceil(0.1f / b.queries.environment().tick_interval)));
                if (command.phase4_eligible)
                    count = 2;
                while (count-- > 0)
                    simulate_live_movement_tick(simulation, b.queries.movement(),
                                                b.movement_environment, selected,
                                                command.captured_command);
                eye = {simulation.position.x, simulation.position.y,
                       simulation.position.z + simulation.view_offset_z};
                velocity = {simulation.velocity.x, simulation.velocity.y, simulation.velocity.z};
            }
            start_time = b.queries.environment().tick_interval;
        }
        if (std::sqrt((velocity.x * velocity.x + velocity.y * velocity.y) +
                      velocity.z * velocity.z) < 10.0f)
            velocity = {};
        Vector3 angles;
        engine::invoke<void>(b.engine_client, 18, &angles);
        const auto speed = engine::invoke<float>(weapon, 452);
        const auto compensate =
            field<std::uint8_t>(selected, offsets.alternate_eye) && settings.super_toss.value;
        Vector3 recoil{};
        if (compensate) {
            const auto value =
                client::capture_command_recoil(selected, offsets.recoil, b.convars.recoil_scale);
            recoil = {value[0], value[1], value[2]};
        }
        GrenadeThrowInput input{
            eye,        velocity,           angles, recoil, strength, speed, 1.0f,
            compensate, identity(selected), false};
        const auto provider = raw.provider(b.trace_flag);
        visual.launch = prepare_grenade_throw(input, provider);
        visual.state.owner_entity = identity(selected);
        simulate_grenade(visual.state, kind, visual.launch.position, visual.launch.velocity,
                         start_time, 0.0f, b.queries.environment(), provider, &b.projection,
                         &visual.path);
        finalize_grenade(visual.state, provider);
        visual.drawing_position = visual.state.position;
        GrenadeScreenPoint endpoint;
        const auto visible = project_grenade_point(b.projection, visual.drawing_position, endpoint);
        GrenadeAuxiliaryResult presentation;
        if (kind == GrenadeKind::explosive || kind == GrenadeKind::flash)
            visual.state.position.z += 1.0f;
        if (kind == GrenadeKind::explosive) {
            const auto selected_damage =
                select_damage(visual.state.position, settings.minimum_damage);
            presentation.magnitude = truncate(selected_damage.damage);
            presentation.highlight = selected_damage.reached_threshold;
            if (presentation.magnitude > 0)
                presentation.annotation = detail::decimal(presentation.magnitude) + u" dmg";
        } else {
            presentation = evaluate_grenade_auxiliary(visual.state, auxiliary_context);
        }
        if (visible && presentation.magnitude > 0 && !presentation.annotation.empty())
            queue.text(truncate(endpoint.x - 4.0f), truncate(endpoint.y - 8.0f),
                       presentation.highlight ? 0xff15c28fu : 0xc8ffffffu, 0x18, 0,
                       presentation.annotation);
        if (presentation.draw_path)
            draw_active_grenade_path(queue, config, visual.path, visible ? &endpoint : nullptr,
                                     presentation.magnitude);
    };
    trajectory();
    draw_warnings(queue, config, warnings);
}
void GrenadeLiveScene::draw_prediction(ui::RenderQueue& queue,
                                       const misc::GrenadePredictionConfig& settings,
                                       const GrenadeEspConfig& config,
                                       GrenadeTrajectoryOwner& visual,
                                       misc::GrenadeCommandState& state,
                                       GrenadeWarningOwner& warnings) {
    storage_->draw(queue, settings, config, visual, state, warnings);
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_live_offsets.cpp
// ============================================================================

namespace skeet::esp {
GrenadeLiveOffsets::GrenadeLiveOffsets(const engine::NativeOffsetGlobals& g)
    : armor(g, 0x43467908, 0x434682e4, 0x47a6f50c, 0),
      view_offset(g, 0x43468a34, 0x43468c00, 0xc246315e, 0),
      life_state(g, 0x43468e78, 0x434684e4, 0x9d920e35, 0),
      movement_type(g, 0x43468bd4, 0x4346a678, 0x7501cb91, 0),
      observer_mode(g, 0x4346a6cc, 0x43468ef8, 0x2f4e97c4, 0),
      observer_target(g, 0x43468f84, 0x43467e64, 0x3b704b41, 0),
      view_angles(g, 0x43468ea0, 0x4346a74c, 0xe23a9be1, 0),
      alternate_eye(g, 0x43468bf8, 0x43467b8c, 0xd070684c, 0),
      player_flags(g, 0x434688d0, 0x43468c24, 0xe2fbe695, 0),
      weapon_definition(g, 0x4346908c, 0x43468ba0, 0x5c9f0fbb, 0),
      weapon_throw_time(g, 0x43467aa4, 0x43467bf4, 0x3b93667b, 0),
      weapon_strength(g, 0x43467d1c, 0x43468e34, 0x1b9654d9, 0),
      weapon_state(g, 0x4346828c, 0x43468d2c, 0x9e2078a1, 0),
      recoil(g, 0x434682b8, 0x43467d90, 0x1bae1168, 0),
      game_player_override(g, 0x43467910, 0x43467c08, 0xd720aea6, 0),
      teammate_damage_disabled(g, 0x43468d1c, 0x43468500, 0xf57beaa9, 0),
      rejection_cde290d2(g, 0x4346a5dc, 0x43468430, 0xcde290d2, 0),
      rejection_af649dac(g, 0x43467dd0, 0x4346a610, 0xaf649dac, 0),
      duck_amount(g, 0x434679f8, 0x434675f8, 0x3309c0cf, 0),
      active_weapon(g, 0x43467a74, 0x434678c0, 0xcd812654, 0) {}
GrenadeLiveOffsets capture_grenade_live_offsets(const engine::NativeOffsetGlobals& globals) {
    return GrenadeLiveOffsets(globals);
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_live_warnings.cpp
// ============================================================================

namespace skeet::esp {
using namespace grenade_live_detail;
void GrenadeLiveScene::Storage::assess(GrenadeWarningRecord& record, void* selected,
                                       std::uint32_t& buttons) {
    const auto& o = raw.bindings().offsets;
    if (record.type == 0) {
        const auto remaining = record.end_time - bindings.current_time;
        auto origin = raw.live_origin(selected);
        const auto velocity = field<Vector3>(selected, o.velocity);
        if ((velocity.y * velocity.y + velocity.x * velocity.x) > 400.0f) {
            const auto time = clamp(remaining, 0.2f, 1.6f);
            const Vector3 predicted{time * velocity.x + origin.x, time * velocity.y + origin.y,
                                    time * velocity.z + origin.z};
            if (squared(difference(origin, record.position)) >
                squared(difference(predicted, record.position))) {
                const auto delta = difference(predicted, origin);
                MovementTraceFlagGuard guard(bindings.trace_flag);
                const auto hit =
                    raw.trace({origin, delta, {}, squared(delta) != 0, true}, 1,
                              {GrenadeDamageFilterKind::ignore_entity, identity(selected)});
                if (!hit.start_solid)
                    origin = hit.end;
            }
        }
        auto value = damage(selected, record.position, origin).damage;
        if (value > 0.0f)
            value *= damage_scale(record.handle, record.type);
        if (record.damage > value && remaining > 0.0f && remaining < 0.03125f)
            buttons = (buttons & 0xfffffbffu) | 0x200u;
        record.damage = value;
    } else if (record.type == 1) {
        bool covered = false;
        for (auto* smoke : raw.collect_smokes(record.position, 166.0f, 0x80, false)) {
            if (!field<std::uint8_t>(smoke, o.smoke_active))
                continue;
            if (grenade_warning_smoke_overlap(record.position, raw.live_origin(smoke))) {
                covered = true;
                break;
            }
        }
        record.suppressed = covered;
        const auto origin = raw.live_origin(selected);
        record.damage = squared(difference(record.position, origin)) < 22500.0f ? 100.0f : 0.0f;
    } else if (record.type == 2) {
        const auto origin = raw.live_origin(selected);
        auto* inferno = record.handle == 0xffffffffu ? nullptr : raw.by_handle(record.handle);
        record.damage = 0.0f;
        if (!inferno)
            return;
        const auto center = field<Vector3>(inferno, o.origin);
        const auto count = field<std::int32_t>(inferno, o.fire_count);
        auto closest = std::sqrt(squared(difference(origin, center)));
        for (std::int32_t cell = 0; cell < count; ++cell) {
            if (!field<std::uint8_t>(inferno, o.fire_active + cell))
                continue;
            const Vector3 point{float(field<std::int32_t>(inferno, o.fire_x + cell * 4)) + center.x,
                                float(field<std::int32_t>(inferno, o.fire_y + cell * 4)) + center.y,
                                float(field<std::int32_t>(inferno, o.fire_z + cell * 4)) +
                                    center.z};
            const auto next = std::sqrt(squared(difference(point, origin)));
            closest = _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(next), _mm_set_ss(closest)));
        }
        if (closest < 150.0f)
            record.damage = 100.0f;
    }
}
void GrenadeLiveScene::Storage::update(GrenadeWarningOwner& owner, const GrenadeEspConfig& config,
                                       void* selected, std::uintptr_t incoming) {
    const auto& b = bindings;
    const auto& o = raw.bindings().offsets;
    std::erase_if(owner.records, [&](const GrenadeWarningRecord& record) {
        if (record.type < 2) {
            auto* entity = record.handle == 0xffffffffu ? nullptr : raw.by_handle(record.handle);
            if (!entity || field<std::int32_t>(entity, o.explosion_effect_tick) != 0)
                return true;
        }
        const auto expiration =
            truncate((record.end_time + 1.0f) / b.queries.environment().tick_interval + 0.5f);
        return b.tick_count > expiration;
    });
    if (!selected || (!config.proximity_warning.value && !b.grenade_entities_enabled))
        return;
    const auto existing = owner.records.size();
    for (std::size_t ordinal = 0; ordinal < existing; ++ordinal) {
        auto& record = owner.records[ordinal];
        auto* entity = record.handle == 0xffffffffu ? nullptr : raw.by_handle(record.handle);
        if (!entity || raw.dormant(entity) || record.type > 1 ||
            field<std::int32_t>(entity, o.explosion_effect_tick) != 0 ||
            (field<std::uint32_t>(entity, o.effects) & 0x20u))
            continue;
        const auto thrower_handle = field<std::uint32_t>(entity, o.thrower_handle);
        if (thrower_handle == 0xffffffffu || !raw.by_handle(thrower_handle))
            continue;
        owner.projectile_points.clear();
        owner.projectile_points.push_back(raw.live_origin(entity));
        const auto velocity = field<Vector3>(entity, o.velocity);
        const auto initial = field<Vector3>(entity, o.initial_velocity);
        const auto current = squared(difference(velocity, initial)) < 1.0f
                                 ? float(b.tick_count) * b.queries.environment().tick_interval
                                 : field<float>(entity, o.simulation_time);
        if (!(squared(velocity) > 900.0f))
            continue;
        GrenadeState prediction{};
        prediction.owner_entity = identity(entity);
        const auto bounce = field<std::uint16_t>(entity, o.projectile_flags);
        const auto group = field<std::int32_t>(entity, o.collision_group);
        simulate_grenade(prediction, record.kind, field<Vector3>(entity, o.origin),
                         field<Vector3>(entity, o.velocity), current, record.creation_time,
                         b.queries.environment(), raw.provider(b.trace_flag), nullptr, nullptr,
                         group, bounce);
        finalize_grenade(prediction, raw.provider(b.trace_flag));
        if (record.kind == GrenadeKind::explosive)
            prediction.position.z += 1.0f;
        if (squared(difference(prediction.position, record.position)) > 4096.0f)
            record.position = prediction.position;
    }
    auto* packet = reinterpret_cast<const void*>(incoming);
    for (std::int32_t ordinal = 0; ordinal < field<std::int32_t>(packet, 0x4044); ++ordinal) {
        const auto offset = std::ptrdiff_t(0x44) + std::ptrdiff_t(ordinal) * 8;
        if (field<std::uint32_t>(packet, offset + 4) != 0)
            continue;
        const auto index = field<std::uint32_t>(packet, offset);
        bool found = false;
        for (const auto& record : owner.records)
            if ((record.handle == 0xffffffffu ? 0x1fffu : record.handle & 0xffffu) == index) {
                found = true;
                break;
            }
        if (found || std::bit_cast<std::int32_t>(index) <= 64 || (index & 0xffff0000u))
            continue;
        auto* entity = raw.by_index(std::bit_cast<std::int32_t>(index));
        if (!entity)
            continue;
        const auto klass = raw.client_class(entity);
        std::uint32_t type;
        GrenadeKind kind;
        float lifetime;
        if (klass == raw.bindings().base_projectile_class) {
            if (field<float>(entity, o.damage) != 350.0f)
                continue;
            type = 0;
            kind = GrenadeKind::explosive;
            lifetime = 1.5f;
        } else if (klass == raw.bindings().molotov_projectile_class) {
            type = 1;
            kind = GrenadeKind::molotov;
            lifetime = b.queries.environment().incendiary_duration();
        } else if (klass == raw.bindings().inferno_class) {
            const auto handle = field<std::uint32_t>(entity, o.owner_handle);
            auto* actor = handle == 0xffffffffu ? nullptr : raw.by_handle(handle);
            if (!admitted(2, actor, selected))
                continue;
            const auto start = float(field<std::int32_t>(entity, o.inferno_start_tick)) *
                               b.queries.environment().tick_interval;
            const auto reference = *engine::invoke<const std::uint32_t*>(entity, 2);
            const auto origin = field<Vector3>(entity, o.origin);
            owner.records.push_back(
                {reference, origin, 0.0f, 2, GrenadeKind::molotov, false, start, start + 7.0f});
            continue;
        } else
            continue;
        if (field<std::int32_t>(entity, o.explosion_effect_tick) != 0)
            continue;
        const auto owner_handle = field<std::uint32_t>(entity, o.thrower_handle);
        if (owner_handle == 0xffffffffu)
            continue;
        auto* actor = raw.by_handle(owner_handle);
        if (!admitted(type, actor, selected))
            continue;
        const auto initial_z = field<float>(entity, o.initial_velocity + 8);
        const auto current_z = field<float>(entity, o.velocity + 8);
        const auto elapsed =
            grenade_projectile_elapsed(initial_z, current_z, b.queries.environment().gravity(),
                                       b.queries.environment().tick_interval);
        if (!(elapsed < lifetime))
            continue;
        const auto reference = *engine::invoke<const std::uint32_t*>(entity, 2);
        const auto creation = field<float>(entity, o.simulation_time) - elapsed;
        const auto engine_time = engine::invoke<float>(b.engine_client, 14);
        const auto rounded = static_cast<float>(
            static_cast<long double>(engine_time) / b.engine_clock_interval + 0.5L);
        const auto current = float(truncate(rounded)) * b.queries.environment().tick_interval;
        if (actor && raw.player(actor))
            (void)raw.dormant(actor);
        GrenadeState prediction{};
        prediction.owner_entity = identity(entity);
        const auto bounce = field<std::uint16_t>(entity, o.projectile_flags);
        const auto group = field<std::int32_t>(entity, o.collision_group);
        simulate_grenade(prediction, kind, field<Vector3>(entity, o.origin),
                         field<Vector3>(entity, o.velocity), current, creation,
                         b.queries.environment(), raw.provider(b.trace_flag), nullptr, nullptr,
                         group, bounce);
        GrenadeWarningRecord record{reference, {},    0.0f,     type,
                                    kind,      false, creation, prediction.time};
        finalize_grenade(prediction, raw.provider(b.trace_flag));
        if (kind == GrenadeKind::explosive)
            prediction.position.z += 1.0f;
        else if (!prediction.fire_ground)
            continue;
        record.position = prediction.position;
        assess(record, selected, b.command_buttons);
        owner.records.push_back(record);
    }
    owner.command_buttons = b.command_buttons;
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_prediction.cpp
// ============================================================================

namespace skeet::esp {
namespace {
std::int32_t ticks(float time, float interval) {
    return static_cast<std::int32_t>(time / interval + 0.5f);
}
bool rest_detonation(GrenadeKind kind) {
    return kind == GrenadeKind::smoke || kind == GrenadeKind::decoy ||
           kind == GrenadeKind::tactical;
}
float duration(const GrenadeEnvironment& env) noexcept {
    return env.incendiary_duration;
}
float duration(const GrenadeEnvironmentBindings& env) noexcept {
    return env.incendiary_duration();
}
float gravity(const GrenadeEnvironment& env) noexcept {
    return env.gravity;
}
float gravity(const GrenadeEnvironmentBindings& env) noexcept {
    return env.gravity();
}
std::uint32_t world(const GrenadeEnvironment& env) noexcept {
    return env.world_entity;
}
std::uint32_t world(const GrenadeEnvironmentBindings& env) {
    return env.world_entity();
}
GrenadeTrace move_tick(GrenadeState& state, Vector3 delta, GrenadeTraceProvider trace,
                       const GrenadeEnvironment&) {
    return grenade_detail::move(state, delta, trace);
}
GrenadeTrace move_tick(GrenadeState& state, Vector3 delta, GrenadeTraceProvider trace,
                       const GrenadeEnvironmentBindings& env) {
    return grenade_detail::move(state, delta, trace, env);
}
}
template <class Environment>
void simulate_grenade_impl(GrenadeState& state, GrenadeKind kind, Vector3 origin, Vector3 velocity,
                           float current, float creation, const Environment& env,
                           GrenadeTraceProvider trace, const Projection* projection,
                           GrenadePath* path, std::int32_t collision_group,
                           std::uint16_t projectile_flags, std::vector<Vector3>* world_path) {
    MovementTraceFlagGuard trace_guard(trace.trace_flag);
    const auto owner = state.owner_entity;
    state = {};
    state.owner_entity = owner;
    state.ignored_handle = 0xffffffffu;
    state.collision_group = collision_group;
    state.bounces = std::bit_cast<std::int16_t>(projectile_flags);
    state.kind = kind;
    state.position = origin;
    state.velocity = velocity;
    state.time = current;
    if (rest_detonation(kind))
        state.detonate_time = std::numeric_limits<float>::max();
    else if (kind == GrenadeKind::molotov || kind == GrenadeKind::incendiary)
        state.detonate_time = duration(env) + creation;
    else
        state.detonate_time = creation + 1.5f;
    float think = current == 0.0f ? 0.2f : creation;
    if (current != 0.0f)
        while (think < current && think <= state.detonate_time)
            think = static_cast<float>(ticks(think + 0.2f, env.tick_interval)) * env.tick_interval;
    state.next_think_tick = ticks(think, env.tick_interval);
    auto remaining = ticks(5.0f, env.tick_interval);
    while (remaining-- != 0) {
        if (std::abs(state.velocity.x) < 0.01f && std::abs(state.velocity.y) < 0.01f &&
            std::abs(state.velocity.z) < 0.01f)
            break;
        if (state.next_think_tick <= ticks(state.time, env.tick_interval)) {
            if (state.time > state.detonate_time)
                break;
            if (rest_detonation(state.kind)) {
                const auto speed = std::sqrt(
                    (state.velocity.x * state.velocity.x + state.velocity.y * state.velocity.y) +
                    state.velocity.z * state.velocity.z);
                if (speed <= 0.1f)
                    break;
            }
            state.next_think_tick = ticks(state.time + 0.2f, env.tick_interval);
        }
        step_grenade(state, env, trace, projection, path);
        if (path && projection)
            record_grenade_point(*path, *projection, state.position, false);
        // Derived callback4331FC5B used by43380299. Its +64 threshold is zero;
        // the owner supplies the initial point and handles finalized endpoints.
        if (world_path && 0.0f <= state.time)
            world_path->push_back(state.position);
        state.time += env.frame_interval;
    }
}

template <class Environment>
void step_grenade_impl(GrenadeState& state, const Environment& env, GrenadeTraceProvider provider,
                       const Projection* projection, GrenadePath* path) {
    const auto interval = env.frame_interval;
    Vector3 delta{state.velocity.x * interval, state.velocity.y * interval, 0};
    const auto old_z = state.velocity.z;
    state.velocity.z -= (gravity(env) * 0.4f) * interval;
    delta.z = ((state.velocity.z + old_z) * 0.5f) * interval;
    auto hit = move_tick(state, delta, provider, env);
    state.sky = state.hit_world = false;
    if (hit.all_solid)
        state.velocity = {};
    else if (hit.fraction != 1.0f) {
        state.sky = (hit.surface_flags & 4) != 0;
        const auto world_entity = world(env);
        state.hit_world = hit.entity.identity == world_entity;
        state.bounces =
            std::bit_cast<std::int16_t>(std::uint16_t(std::uint16_t(state.bounces) + 1U));
        state.last_plane = hit.normal;
        state.collided = true;
        if (path && projection)
            record_grenade_point(*path, *projection, hit.end, true);
        grenade_detail::collide(state, hit, delta, env, provider);
    }
}
void simulate_grenade(GrenadeState& state, GrenadeKind kind, Vector3 origin, Vector3 velocity,
                      float current, float creation, const GrenadeEnvironment& env,
                      GrenadeTraceProvider trace, const Projection* projection, GrenadePath* path,
                      std::int32_t group, std::uint16_t flags, std::vector<Vector3>* world_path) {
    simulate_grenade_impl(state, kind, origin, velocity, current, creation, env, trace, projection,
                          path, group, flags, world_path);
}
void simulate_grenade(GrenadeState& state, GrenadeKind kind, Vector3 origin, Vector3 velocity,
                      float current, float creation, const GrenadeEnvironmentBindings& env,
                      GrenadeTraceProvider trace, const Projection* projection, GrenadePath* path,
                      std::int32_t group, std::uint16_t flags, std::vector<Vector3>* world_path) {
    simulate_grenade_impl(state, kind, origin, velocity, current, creation, env, trace, projection,
                          path, group, flags, world_path);
}
void step_grenade(GrenadeState& state, const GrenadeEnvironment& env, GrenadeTraceProvider trace,
                  const Projection* projection, GrenadePath* path) {
    step_grenade_impl(state, env, trace, projection, path);
}
void step_grenade(GrenadeState& state, const GrenadeEnvironmentBindings& env,
                  GrenadeTraceProvider trace, const Projection* projection, GrenadePath* path) {
    step_grenade_impl(state, env, trace, projection, path);
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_prediction_collision.cpp
// ============================================================================

namespace skeet::esp {
namespace {
float length_squared(Vector3 v) {
    return (v.y * v.y + v.x * v.x) + v.z * v.z;
}
Vector3 scaled(Vector3 v, float scale) {
    return {v.x * scale, v.y * scale, v.z * scale};
}
/* [dedup difference] */
/* [removed duplicate field] */
void* entity_pointer(const GrenadeHitEntity& entity) noexcept {
    return reinterpret_cast<void*>(std::uintptr_t(entity.identity));
}
std::uint32_t class_identity(const void* type) noexcept {
    return std::uint32_t(reinterpret_cast<std::uintptr_t>(type));
}
GrenadeTrace line(GrenadeState& state, Vector3 start, Vector3 end, std::uint32_t mask,
                  GrenadeTraceProvider provider, std::int32_t group = 0) {
    auto delta = difference(end, start);
    return provider.trace(provider.context, {start, delta, {}, length_squared(delta) != 0, true},
                          mask, {state.owner_entity, 0xffffffffu, group});
}
bool fire_visible(GrenadeState& state, Vector3& origin, Vector3 cell, bool adjust,
                  GrenadeTraceProvider provider) { // 433AC2B0
    auto raised = cell;
    raised.z += 30.0f;
    if (line(state, raised, origin, 0x2004003u, provider).fraction >= 1.0f)
        return true;
    if (line(state, cell, origin, 0x2004003u, provider).fraction >= 1.0f)
        return true;
    if (!adjust)
        return false;
    raised = origin;
    raised.z += 30.0f;
    origin = line(state, origin, raised, 0x2004003u, provider).end;
    return fire_visible(state, origin, cell, false, provider);
}
bool extinguish_cell(GrenadeState& state, Vector3& origin, Vector3 cell,
                     GrenadeTraceProvider provider) {
    cell.z += 40.0f;
    const auto dx = origin.x - cell.x, dy = origin.y - cell.y;
    const auto horizontal = dy * dy + dx * dx;
    if (!(4900.0f > horizontal))
        return false;
    const auto height = std::abs(origin.z - (cell.z - 20.0f)) - 20.0f;
    if (height > 0.0f && height * height + horizontal >= 4900.0f)
        return false;
    if (!fire_visible(state, origin, cell, true, provider))
        return false;
    state.position = origin;
    state.extinguished = true;
    state.velocity = {};
    return true;
}
bool extinguish_fires(GrenadeState& state, Vector3& origin, const GrenadeEnvironment& env,
                      GrenadeTraceProvider provider) {
    for (const auto& fire : env.fires) {
        if (!(262144.0f > length_squared(difference(fire.origin, state.position))))
            continue;
        for (auto cell : fire.cells)
            if (extinguish_cell(state, origin, cell, provider))
                return true;
    }
    return false;
}
bool extinguish_fires(GrenadeState& state, Vector3& origin, const GrenadeEnvironmentBindings& env,
                      GrenadeTraceProvider provider) {
    //4336DC18..DD78 first collects at most64 live inferno pointers. Cell data
    // is read later; intervening visibility traces may mutate the entities.
    std::array<void*, 64> fires;
    std::size_t count = 0;
    const auto last = field<std::int32_t>(env.entity_list, 0x24);
    for (std::int32_t index = 65; index <= last;
         index = std::bit_cast<std::int32_t>(std::uint32_t(index) + 1U)) {
        if (std::uint32_t(index) & 0xFFFF0000U)
            continue;
        auto* entity = engine::invoke<void*>(env.entity_list, 3, index);
        if (!entity)
            continue;
        auto* network = static_cast<std::byte*>(entity) + 8;
        if (engine::invoke<bool>(network, 9))
            continue;
        const auto type = engine::invoke<std::uint32_t>(network, 2);
        if (type != class_identity(env.inferno_class))
            continue;
        const auto position = field<Vector3>(entity, env.offsets.fire_origin_a9ee6062);
        if (!(262144.0f > length_squared(difference(position, state.position))))
            continue;
        fires[count++] = entity;
        if (count == fires.size())
            break;
    }
    for (std::size_t ordinal = 0; ordinal < count; ++ordinal) {
        const auto* fire = fires[ordinal];
        //4336DDC1..DDD6 retains the field address at SP48 across visibility
        //traces. Its vector contents are read again for each active cell.
        const auto* origin_field =
            static_cast<const std::byte*>(fire) + std::ptrdiff_t(env.offsets.fire_origin_a9ee6062);
        const auto cells = field<std::int32_t>(fire, env.offsets.fire_count_3584e3a5);
        for (std::int32_t index = 0; index < cells; ++index) {
            if (!field<std::uint8_t>(fire, env.offsets.fire_active_fc1ef3a1 + index))
                continue;
            const auto offset = std::ptrdiff_t(index) * 4;
            const auto position = field<Vector3>(origin_field, 0);
            const Vector3 cell{
                float(field<std::int32_t>(fire, env.offsets.fire_x_24a16ff6 + offset)) + position.x,
                float(field<std::int32_t>(fire, env.offsets.fire_y_94e8c7f7 + offset)) + position.y,
                float(field<std::int32_t>(fire, env.offsets.fire_z_d7600aac + offset)) +
                    position.z};
            if (extinguish_cell(state, origin, cell, provider))
                return true;
        }
    }
    return false;
}
template <class Environment>
bool extinguish(GrenadeState& state, const Environment& env, GrenadeTraceProvider provider) {
    auto end = state.position;
    end.z -= 100.0f;
    const auto ground = line(state, state.position, end, 0x200400BU, provider, 1);
    auto origin = state.position;
    if (!ground.start_solid) {
        if (ground.fraction >= 1.0f)
            return false;
        if (ground.fraction > 0.001f)
            origin = ground.end;
    }
    return extinguish_fires(state, origin, env, provider);
}
bool slope_exempt(const GrenadeHitEntity& entity, const GrenadeEnvironment& env) {
    const auto special_physics =
        env.physics_class != 0 && entity.native_class == env.physics_class &&
        entity.solid_type == 1 && entity.movement_type == 7 && (entity.solid_flags & 0x800u) &&
        entity.owner_handle == 0xffffffffu &&
        (entity.collision_activity[0] != 0.0f || entity.collision_activity[1] != 0.0f);
    return entity.identity && entity.native_class &&
           ((entity.flags & 0x28u) || special_physics || entity.incendiary_contact_exempt ||
            entity.native_class == env.breakable_class ||
            entity.native_class == env.alternate_breakable_class);
}
bool special_physics(void* entity, const GrenadeEnvironmentBindings& env) {
    if (!env.physics_class)
        return false;
    auto* network = engine::invoke<void*>(entity, 4);
    if (!network)
        return false;
    const auto type = engine::invoke<std::uint32_t>(network, 2);
    if (!type || type != class_identity(env.physics_class))
        return false;
    auto* collision = engine::invoke<void*>(entity, 3);
    if (engine::invoke<std::int32_t>(collision, 11) != 1)
        return false;
    if (field<std::uint8_t>(entity, env.offsets.movement_type_7501cb91) != 7)
        return false;
    collision = engine::invoke<void*>(entity, 3);
    if (!(engine::invoke<std::uint32_t>(collision, 12) & 0x800U))
        return false;
    if (field<std::uint32_t>(entity, env.offsets.owner_8d1bd8d0) != UINT32_MAX)
        return false;
    return field<float>(entity, env.offsets.activity_8dbc9874) != 0.0f ||
           field<float>(entity, env.offsets.activity_68140cd8) != 0.0f;
}
bool slope_exempt(const GrenadeHitEntity& hit, const GrenadeEnvironmentBindings& env) {
    auto* entity = entity_pointer(hit);
    if (!entity)
        return false;
    auto* network = engine::invoke<void*>(entity, 4);
    if (!network)
        return false;
    const auto type = engine::invoke<std::uint32_t>(network, 2);
    if (!type)
        return false;
    if (field<std::uint32_t>(entity, env.offsets.collision_flags_b7e9c94e) & 0x28U)
        return true;
    if (special_physics(entity, env))
        return true;
    if (type == class_identity(env.breakable_class) ||
        type == class_identity(env.slope_exempt_class))
        return true;
    return engine::invoke<std::int32_t>(entity, 161) != 0;
}
bool collision_player(const GrenadeHitEntity& hit, const GrenadeEnvironment&) noexcept {
    return hit.player;
}
bool collision_player(const GrenadeHitEntity& hit, const GrenadeEnvironmentBindings&) {
    return engine::invoke<bool>(entity_pointer(hit), 158);
}
bool pass_through(const GrenadeHitEntity& entity, const GrenadeEnvironment& env) noexcept {
    if (entity.native_class == env.breakable_class)
        return entity.state == 0;
    if (entity.native_class == env.physics_class)
        return entity.movement_type == 7 && entity.solid_type == 1 &&
               (entity.solid_flags & 0x800U) == 0;
    return entity.native_class == env.alternate_breakable_class && entity.state == 0;
}
bool pass_through(const GrenadeHitEntity& hit, const GrenadeEnvironmentBindings& env) {
    auto* entity = entity_pointer(hit);
    const auto type = engine::invoke<std::uint32_t>(static_cast<std::byte*>(entity) + 8, 2);
    if (type == class_identity(env.breakable_class))
        return field<std::uint8_t>(entity, env.offsets.state_f0374295) == 0;
    if (type == class_identity(env.physics_class)) {
        if (field<std::uint8_t>(entity, env.offsets.movement_type_7501cb91) != 7)
            return false;
        auto* collision = engine::invoke<void*>(entity, 3);
        if (engine::invoke<std::int32_t>(collision, 11) != 1)
            return false;
        collision = engine::invoke<void*>(entity, 3);
        return (engine::invoke<std::uint32_t>(collision, 12) & 0x800U) == 0;
    }
    return type == class_identity(env.alternate_breakable_class) &&
           field<std::uint8_t>(entity, env.offsets.state_f0374295) == 0;
}
std::uint32_t handle(const GrenadeHitEntity& hit, const GrenadeEnvironment&) noexcept {
    return hit.handle;
}
std::uint32_t handle(const GrenadeHitEntity& hit, const GrenadeEnvironmentBindings&) {
    return engine::invoke<std::uint32_t>(static_cast<std::byte*>(entity_pointer(hit)) + 8, 10);
}
float slope(const GrenadeEnvironment& env) noexcept {
    return env.incendiary_max_slope;
}
float slope(const GrenadeEnvironmentBindings& env) noexcept {
    return env.incendiary_max_slope();
}
GrenadeTrace move_collision(GrenadeState& state, Vector3 delta, GrenadeTraceProvider trace,
                            const GrenadeEnvironment&) {
    return grenade_detail::move(state, delta, trace);
}
GrenadeTrace move_collision(GrenadeState& state, Vector3 delta, GrenadeTraceProvider trace,
                            const GrenadeEnvironmentBindings& env) {
    return grenade_detail::move(state, delta, trace, env);
}
}

namespace grenade_detail {
template <class Environment>
void collide_impl(GrenadeState& state, GrenadeTrace& hit, Vector3& displacement,
                  const Environment& env, GrenadeTraceProvider provider) {
    if (state.kind == GrenadeKind::smoke && extinguish(state, env, provider))
        return;
    if ((state.kind == GrenadeKind::molotov || state.kind == GrenadeKind::incendiary) &&
        !slope_exempt(hit.entity, env) &&
        hit.normal.z >= std::cos(slope(env) * std::bit_cast<float>(0x3c8efa35u))) {
        state.velocity = {};
        return;
    }
    float surface_elasticity = 1.0f;
    if (hit.entity.identity) {
        if (collision_player(hit.entity, env)) {
            surface_elasticity = 0.3f;
            if (state.ignored_handle != 0xffffffffu) {
                state.collision_group = 1;
                return;
            }
        } else {
            if (pass_through(hit.entity, env)) {
                state.velocity = scaled(state.velocity, 0.4f);
                state.ignored_handle = handle(hit.entity, env);
                return;
            }
        }
        state.ignored_handle = handle(hit.entity, env);
    }
    const auto elasticity = std::clamp(surface_elasticity * 0.45f, 0.0f, 0.9f);
    const auto backoff = ((hit.normal.y * state.velocity.y + hit.normal.x * state.velocity.x) +
                          hit.normal.z * state.velocity.z) *
                         2.0f;
    auto velocity = state.velocity;
    auto component = [backoff](float value, float normal) {
        auto result = value - normal * backoff;
        if (result > -0.1f && result < 0.1f)
            result = 0.0f;
        return result;
    };
    velocity = {component(velocity.x, hit.normal.x), component(velocity.y, hit.normal.y),
                component(velocity.z, hit.normal.z)};
    velocity = scaled(velocity, elasticity);
    displacement = velocity;
    const auto speed_squared = length_squared(velocity);
    // COMISS/JB admits unordered normal-Z; the later JAE rejects only an
    // ordered speed>=400. Keep those branch semantics in the shared physics.
    if (hit.entity.identity &&
        (!(0.7f >= hit.normal.z) || (hit.normal.z > 0.1f && !(speed_squared >= 400.0f)))) {
        if (speed_squared > 96000.0f) {
            const auto inverse =
                1.0f / (std::sqrt((velocity.y * velocity.y + velocity.z * velocity.z) +
                                  velocity.x * velocity.x) +
                        std::numeric_limits<float>::epsilon());
            const auto alignment =
                ((velocity.y * inverse) * hit.normal.y + (velocity.x * inverse) * hit.normal.x) +
                (velocity.z * inverse) * hit.normal.z;
            if (alignment > 0.5f)
                velocity = scaled(velocity, 1.5f - alignment);
        }
        state.velocity = speed_squared < 400.0f ? Vector3{} : velocity;
        if (!(400.0f > speed_squared)) {
            displacement = scaled(velocity, (1.0f - hit.fraction) * env.tick_interval);
            hit = move_collision(state, displacement, provider, env);
        }
    } else {
        state.velocity = velocity;
        displacement = scaled(velocity, (1.0f - hit.fraction) * env.tick_interval);
        hit = move_collision(state, displacement, provider, env);
    }
    if (state.bounces > 20) {
        state.velocity = {};
        state.next_think_tick = static_cast<std::int32_t>(state.time / env.tick_interval + 0.5f);
    }
}
void collide(GrenadeState& state, GrenadeTrace& hit, Vector3& displacement,
             const GrenadeEnvironment& env, GrenadeTraceProvider provider) {
    collide_impl(state, hit, displacement, env, provider);
}
void collide(GrenadeState& state, GrenadeTrace& hit, Vector3& displacement,
             const GrenadeEnvironmentBindings& env, GrenadeTraceProvider provider) {
    collide_impl(state, hit, displacement, env, provider);
}
} // namespace grenade_detail

void finalize_grenade(GrenadeState& state, GrenadeTraceProvider provider) {
    if (state.finalized)
        return;
    state.finalized = true;
    MovementTraceFlagGuard trace_guard(provider.trace_flag);
    if (state.kind == GrenadeKind::explosive) {
        auto start = state.position;
        start.z += 8.0f;
        auto end = start;
        end.z -= 32.0f;
        auto hit = line(state, start, end, 0x600400bu, provider);
        if (hit.start_solid) {
            start = state.position;
            end = start;
            end.z -= 32.0f;
            hit = line(state, start, end, 0x600400bu, provider);
        }
        if (hit.fraction != 1.0f)
            state.position = {hit.normal.x * 0.6f + hit.end.x, hit.normal.y * 0.6f + hit.end.y,
                              hit.normal.z * 0.6f + hit.end.z};
    } else if (state.kind == GrenadeKind::molotov || state.kind == GrenadeKind::incendiary) {
        state.fire_ground = false;
        if (state.sky)
            return;
        if (state.hit_world) {
            state.fire_ground = true;
            return;
        }
        auto start = state.position;
        start.z += 10.0f;
        auto end = state.position;
        end.z -= 128.0f;
        auto hit = line(state, start, end, 0x200400bu, provider);
        if (hit.fraction != 1.0f && (hit.surface_flags & 4) == 0) {
            state.fire_ground = state.collided = true;
            state.position = hit.end;
            state.last_plane = hit.normal;
        }
    }
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_prediction_damage_selection.cpp
// ============================================================================

namespace skeet::esp {
bool consider_grenade_damage(GrenadeDamageSelection& selection, float damage, std::int32_t armor,
                             std::int32_t health, std::int32_t threshold,
                             std::int32_t index) noexcept {
    const auto truncate = [](float value) { return _mm_cvttss_si32(_mm_set_ss(value)); };
    const auto needed = std::min(threshold, armor > 0 ? 50 : 80);
    if (truncate(damage - 3.0f) >= health) {
        selection = {damage, true, index};
        return true;
    }
    if (!(selection.damage >= damage)) {
        selection.damage = damage;
        selection.target_index = index;
        if (needed <= health && needed <= truncate(damage)) {
            selection.reached_threshold = true;
            return true;
        }
    }
    return false;
}
GrenadeDamageSelection select_grenade_damage(Vector3 explosion,
                                             std::span<const GrenadeDamageCandidate> candidates,
                                             std::int32_t maximum_players, std::int32_t threshold,
                                             const rage::TargetBindings& targeting,
                                             GrenadeDamageEngine& engine,
                                             const GrenadeDamageMath& math) {
    GrenadeDamageSelection selected;
    for (std::int32_t index = 1; index < maximum_players; ++index) {
        if (static_cast<std::size_t>(index) >= candidates.size())
            continue;
        const auto& candidate = candidates[index];
        if (!candidate.player || !candidate.entity || !candidate.is_player ||
            candidate.life_state || candidate.field_d070684c || candidate.field_cde290d2 ||
            candidate.field_af649dac || !rage::target_is_enemy(targeting, candidate.entity))
            continue;
        auto player = *candidate.player;
        if (candidate.dormant) {
            if (0.0f >= candidate.history_health)
                continue;
            player.distance_origin = candidate.history_origin;
        } else
            player.distance_origin = player.origin;
        const auto result = evaluate_he_damage(explosion, player, engine, math);
        if (consider_grenade_damage(selected, result.damage, player.armor, candidate.health,
                                    threshold, index))
            break;
    }
    return selected;
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_prediction_draw.cpp
// ============================================================================

namespace skeet::esp {
bool project_grenade_point(const Projection& p, Vector3 v, GrenadeScreenPoint& output) noexcept {
    const auto& m = p.matrix;
    const auto w = ((v.x * m[12] + v.y * m[13]) + v.z * m[14]) + m[15];
    if (0.001f > w)
        return false;
    const auto reciprocal = 1.0f / w;
    const auto x = ((v.x * m[0] + v.y * m[1]) + v.z * m[2]) + m[3];
    const auto y = ((v.x * m[4] + v.y * m[5]) + v.z * m[6]) + m[7];
    output = {(x * reciprocal) * p.horizontal_scale + p.horizontal_scale,
              p.vertical_scale - (y * reciprocal) * p.vertical_scale};
    return true;
}
void record_grenade_point(GrenadePath& path, const Projection& projection, Vector3 position,
                          bool bounce) {
    GrenadeScreenPoint point;
    if (!project_grenade_point(projection, position, point))
        return;
    path.points.push_back(point);
    if (bounce && path.bounces.size() < 20)
        path.bounces.push_back(point);
}
void draw_active_grenade_path(ui::RenderQueue& queue, const GrenadeEspConfig& config,
                              const GrenadePath& path, const GrenadeScreenPoint* endpoint,
                              std::int32_t damage) {
    if (path.points.empty())
        return;
    const auto color =
        damage > 0 ? config.trajectory_secondary_color.value : config.trajectory_color.value;
    auto truncate = [](float value) { return _mm_cvttss_si32(_mm_set_ss(value)); };
    // Native 433F22FC is a float vertex stream (opcode 17, primitive 4).
    // Integer polyline opcode 5 would discard the projected subpixel positions.
    if (path.points.size() > (0x07ffffffu - 24u) / 12u)
        throw std::length_error("Grenade path exceeds render command size");
    const auto count = static_cast<std::uint32_t>(path.points.size());
    auto* payload = queue.append(17, count * 12 + 20);
    const std::uint32_t primitive = 4;
    std::memcpy(payload, &primitive, 4);
    std::memcpy(payload + 4, &count, 4);
    for (const auto& point : path.points) {
        std::memcpy(payload + 8, &point, 8);
        std::memcpy(payload + 16, &color, 4);
        payload += 12;
    }
    for (const auto& bounce : path.bounces) {
        const auto x = truncate(bounce.x), y = truncate(bounce.y);
        queue.circle({x, y, 0x3c000000u, 3.0f, 0.0f, 1.0f});
        queue.circle({x, y, 0xa0ffffffu, 2.0f, 0.0f, 1.0f});
    }
    if (endpoint) {
        const auto x = truncate(endpoint->x), y = truncate(endpoint->y);
        queue.circle({x, y, 0x3c000000u, 3.0f, 0.0f, 1.0f});
        queue.circle({x, y, damage > 0 ? 0xff28ff00u : 0xff2800ffu, 2.0f, 0.0f, 1.0f});
    }
}
void draw_grenade_path(ui::RenderQueue& queue, const GrenadeEspConfig& config,
                       const GrenadePath& path, const Projection& projection, Vector3 position,
                       std::int32_t damage) {
    if (!config.trajectory.value || path.points.empty())
        return;
    GrenadeScreenPoint endpoint;
    const auto visible = project_grenade_point(projection, position, endpoint);
    draw_active_grenade_path(queue, config, path, visible ? &endpoint : nullptr, damage);
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_prediction_entity.cpp
// ============================================================================

namespace skeet::esp {
void draw_inferno_entity(ui::RenderQueue& queue, const GrenadeEspConfig& config,
                         const Projection& projection, const GrenadeEntityClasses& classes,
                         const GrenadeEntityLabels& labels, const GrenadeEntityState& state,
                         std::int32_t current_tick, float tick_interval, bool suspended,
                         effects::GlowEngine* glow, const InfernoTeamState& teams) {
    if (!teams.friendly_fire && teams.owner_present && teams.owner_team == teams.local_team)
        return;
    draw_grenade_entity(queue, config, projection, classes, labels, state, current_tick,
                        tick_interval, suspended, glow);
}
void draw_grenade_entity(ui::RenderQueue& queue, const GrenadeEspConfig& config,
                         const Projection& projection, const GrenadeEntityClasses& classes,
                         const GrenadeEntityLabels& labels, const GrenadeEntityState& state,
                         std::int32_t current_tick, float tick_interval, bool suspended,
                         effects::GlowEngine* glow) {
    if (!config.entities.value)
        return;
    std::u16string_view label;
    if (state.class_index == classes.base_projectile) {
        if (state.damage == 350.0f && state.damage_radius == 99.0f) {
            if (state.explosion_effect_tick != 0)
                return;
            label = labels.explosive;
        } else {
            if (state.damage != 100.0f || state.damage_radius != 100.0f)
                return;
            label = labels.flash;
        }
    } else if (state.class_index == classes.smoke_projectile)
        label = labels.smoke;
    else if (state.class_index == classes.decoy_projectile)
        label = labels.decoy;
    else if (state.class_index == classes.molotov_projectile ||
             state.class_index == classes.inferno)
        label = labels.fire;
    else
        label = labels.other;
    Point screen;
    if (!project_point(projection, state.origin, screen))
        return;
    auto color = 0xc8ffffffu;
    if (!suspended) {
        if (state.class_index != classes.inferno && glow && state.glow_index != -1) {
            const auto packed = config.entity_color.value;
            const auto scale = std::bit_cast<float>(0x3b808081u);
            effects::update_glow(*glow, state.entity, static_cast<std::uint32_t>(state.glow_index),
                                 {static_cast<float>(packed & 255) * scale,
                                  static_cast<float>((packed >> 8) & 255) * scale,
                                  static_cast<float>((packed >> 16) & 255) * scale, 1.0f});
        }
        float duration = 0;
        std::int32_t start = 0;
        if (state.class_index == classes.smoke_projectile && state.smoke_effect) {
            duration = 18.0f;
            start = state.smoke_start_tick;
        } else if (state.class_index == classes.inferno) {
            duration = 7.0f;
            start = state.inferno_start_tick;
        }
        if (duration != 0) {
            const auto elapsed =
                std::bit_cast<std::int32_t>(std::uint32_t(current_tick) - std::uint32_t(start));
            const auto fraction =
                (duration - static_cast<float>(elapsed) * tick_interval) / duration;
            const auto bounded = _mm_cvtss_f32(
                _mm_min_ss(_mm_max_ss(_mm_set_ss(fraction), _mm_set_ss(0.1f)), _mm_set_ss(1.0f)));
            const auto alpha =
                std::bit_cast<std::uint32_t>(std::sqrt(bounded) * 255.0f + 8388608.0f) & 255u;
            color = (alpha << 24) | 0xffffffu;
        }
    }
    queue.text(screen.x, std::bit_cast<std::int32_t>(std::uint32_t(screen.y) + 12u), color, 0x14, 0,
               label);
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_prediction_handler.cpp
// ============================================================================

namespace skeet::esp {
namespace {
#if defined(_M_IX86) || defined(__i386__)
void __fastcall grenade_prediction_entry(void* endpoint, ui::RenderQueue* queue, void*,
                                         std::int32_t*) {
#else
void grenade_prediction_entry(void* endpoint, ui::RenderQueue* queue, void*, std::int32_t*) {
#endif
    auto& handler =
        *static_cast<GrenadePredictionHudHandler*>(static_cast<hud::HudFrameCallback*>(endpoint));
    render_grenade_prediction(*queue, handler.config, handler.owner, handler.frame, handler.scene);
}
const std::array<hud::HudFrameEntry, 1> grenade_prediction_table{grenade_prediction_entry};
}
GrenadePredictionHudHandler::GrenadePredictionHudHandler(engine::OwnerRegistryBinding registry,
                                                         const GrenadeEspConfig& config_input,
                                                         GrenadeTrajectoryOwner& owner_input,
                                                         const GrenadeTrajectoryFrame& frame_input,
                                                         const GrenadePredictionScene& scene_input)
    : hud::HudFrameCallback{nullptr}, config(config_input), owner(owner_input), frame(frame_input),
      scene(scene_input) {
    hud::register_hud_frame(registry, this); //43486787 precedes secondary-vtable setup.
    vtable = grenade_prediction_table.data();
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_prediction_launch.cpp
// ============================================================================

namespace skeet::esp {
namespace {
constexpr auto radians = std::bit_cast<float>(0x3c8efa35u);
constexpr auto degrees = std::bit_cast<float>(0x42652ee0u);
float length(Vector3 v) {
    return std::sqrt((v.x * v.x + v.y * v.y) + v.z * v.z);
}
Vector3 forward(Vector3 angles) {
    const auto yaw = angles.y * radians, pitch = angles.x * radians;
    const auto sy = std::sin(yaw), cy = std::cos(yaw);
    const auto sp = std::sin(pitch), cp = std::cos(pitch);
    return {cy * cp, cp * sy, -sp};
}
/* [removed duplicate clamp] */
}
void compensate_grenade_movement(Vector3 velocity, float speed, Vector3& angles) noexcept {
    const auto direction = forward(angles);
    const auto inverse_direction =
        1.0f / (length(direction) + std::numeric_limits<float>::epsilon());
    const Vector3 unit{inverse_direction * direction.x, inverse_direction * direction.y,
                       inverse_direction * direction.z};
    const Vector3 opposite{-velocity.x, -velocity.y, -velocity.z};
    const auto velocity_length = length(velocity);
    const auto inverse_opposite = 1.0f / (length(opposite) + std::numeric_limits<float>::epsilon());
    const auto alignment =
        ((opposite.y * inverse_opposite) * unit.y + (opposite.x * inverse_opposite) * unit.x) +
        (opposite.z * inverse_opposite) * unit.z;
    const auto travel =
        (std::sqrt((((25.0f * alignment) * alignment) * velocity_length) * velocity_length +
                   (16.0f * speed) * speed - (25.0f * velocity_length) * velocity_length) -
         (5.0f * velocity_length) * alignment) *
        0.25f;
    const auto reciprocal_speed = 1.0f / speed;
    const Vector3 result{(travel * unit.x - velocity.x * 1.25f) * reciprocal_speed,
                         (travel * unit.y - velocity.y * 1.25f) * reciprocal_speed,
                         (travel * unit.z - velocity.z * 1.25f) * reciprocal_speed};
    float pitch, yaw = 0;
    if (result.y == 0.0f && result.x == 0.0f)
        pitch = result.z <= 0 ? 90.0f : 270.0f;
    else {
        yaw = static_cast<float>(
                  std::atan2(static_cast<double>(result.y), static_cast<double>(result.x))) *
              degrees;
        const auto horizontal = std::sqrt(result.x * result.x + result.y * result.y);
        pitch = static_cast<float>(
                    std::atan2(static_cast<double>(-result.z), static_cast<double>(horizontal))) *
                degrees;
    }
    if (std::isfinite(pitch) && std::isfinite(yaw)) {
        angles.x = pitch;
        angles.y = yaw;
    }
}
GrenadeLaunch prepare_grenade_throw(const GrenadeThrowInput& input, GrenadeTraceProvider provider) {
    auto velocity = input.player_velocity;
    if (input.zero_slow_movement && length(velocity) < 10.0f)
        velocity = {};
    auto angles = input.view_angles;
    const auto strength = clamp(input.strength, 0, 1);
    const auto speed = (strength * 0.7f + 0.3f) * clamp(input.throw_velocity * 0.9f, 15, 750);
    if (input.compensate_movement) {
        angles.x -= input.aim_punch.x * input.recoil_scale;
        angles.y -= input.aim_punch.y * input.recoil_scale;
        angles.z -= input.aim_punch.z * input.recoil_scale;
        if ((velocity.x * velocity.x + velocity.y * velocity.y) + velocity.z * velocity.z > 4.0f)
            compensate_grenade_movement(velocity, speed, angles);
    }
    if (angles.x > 90.0f)
        angles.x -= 360.0f;
    else if (angles.x < -90.0f)
        angles.x += 360.0f;
    angles.x -= ((90.0f - std::abs(angles.x)) * 10.0f) / 90.0f;
    MovementTraceFlagGuard trace_guard(provider.trace_flag);
    const auto direction = forward(angles);
    Vector3 origin{input.eye_position.x, input.eye_position.y,
                   (input.eye_position.z - 12.0f) + strength * 12.0f};
    const Vector3 delta{(direction.x * 22.0f + origin.x) - origin.x,
                        (direction.y * 22.0f + origin.y) - origin.y,
                        (direction.z * 22.0f + origin.z) - origin.z};
    const auto hit =
        provider.trace(provider.context,
                       {origin,
                        delta,
                        {2, 2, 2},
                        (delta.y * delta.y + delta.x * delta.x) + delta.z * delta.z != 0,
                        false},
                       0x208400bu, {input.owner_entity, 0xffffffffu, 13});
    origin = {hit.end.x - direction.x * 6.0f, hit.end.y - direction.y * 6.0f,
              hit.end.z - direction.z * 6.0f};
    velocity = {speed * direction.x + velocity.x * 1.25f, speed * direction.y + velocity.y * 1.25f,
                speed * direction.z + velocity.z * 1.25f};
    return {origin, velocity, angles, strength, speed};
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_prediction_owner.cpp
// ============================================================================

namespace skeet::esp {
bool begin_grenade_trajectory(GrenadeTrajectoryOwner& owner, const GrenadeEspConfig& config,
                              const GrenadeTrajectoryFrame& frame) noexcept {
    owner.smoothed_velocity.x += (frame.motion_target.x - owner.smoothed_velocity.x) * 0.2f;
    owner.smoothed_velocity.y += (frame.motion_target.y - owner.smoothed_velocity.y) * 0.2f;
    owner.smoothed_velocity.z += (frame.motion_target.z - owner.smoothed_velocity.z) * 0.2f;
    if (frame.engine_reject_148 || frame.engine_reject_174 || frame.game_player_override)
        return false;
    if (!config.trajectory.value || !frame.player_entity || frame.movement_type == 10 ||
        frame.life_state != 0)
        return false;
    if (!frame.selected_is_local && std::abs(frame.field_e23a9be1) > 50.0f)
        return false;
    if (!frame.weapon_present)
        return false;
    switch (frame.kind) {
    case GrenadeKind::flash:
    case GrenadeKind::explosive:
    case GrenadeKind::smoke:
    case GrenadeKind::molotov:
    case GrenadeKind::decoy:
    case GrenadeKind::incendiary:
    case GrenadeKind::tactical:
    case GrenadeKind::diversion:
        break;
    default:
        return false;
    }
    if (frame.selected_is_local) {
        if (frame.field_9e2078a1 == 190)
            return false;
    } else if (frame.throw_time > 0.0f)
        return false;
    return true;
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_prediction_runtime.cpp
// ============================================================================

namespace skeet::esp {
bool predict_grenade_trajectory(GrenadeTrajectoryOwner& owner, const GrenadeEspConfig& config,
                                const GrenadeTrajectoryFrame& frame,
                                const GrenadeMovementContext& movement,
                                const GrenadeEnvironment& environment, GrenadeTraceProvider trace,
                                const Projection& projection) {
    if (!begin_grenade_trajectory(owner, config, frame))
        return false;
    owner.path.points.clear();
    owner.path.bounces.clear();
    auto input = frame.throw_input;
    input.owner_entity = frame.player_entity;
    input.compensate_movement = frame.cached_motion && input.compensate_movement;
    if (frame.cached_motion) {
        input.player_velocity = owner.smoothed_velocity;
        const auto speed_squared = (input.player_velocity.x * input.player_velocity.x +
                                    input.player_velocity.y * input.player_velocity.y) +
                                   input.player_velocity.z * input.player_velocity.z;
        const auto prediction_tick = std::bit_cast<std::int32_t>(
            std::uint32_t(frame.prediction_tick) + std::uint32_t(frame.prediction_shift));
        if (speed_squared >= 80.0f && frame.motion_tick == prediction_tick) {
            MovementSimulation simulation{};
            initialize_movement_simulation(
                simulation, frame.player_entity,
                {movement.origin.x, movement.origin.y, movement.origin.z},
                {input.player_velocity.x, input.player_velocity.y, input.player_velocity.z},
                movement.player_flags, 0, *movement.player, *movement.rules);
            auto ticks =
                frame.two_tick_preroll
                    ? 2
                    : static_cast<std::int32_t>(std::ceil(0.1f / environment.tick_interval));
            while (ticks-- > 0)
                simulate_movement_tick(simulation, *movement.collision, *movement.environment,
                                       frame.player_entity, *movement.command,
                                       *movement.command_angles);
            input.eye_position = {simulation.position.x, simulation.position.y,
                                  simulation.position.z + simulation.view_offset_z};
            input.player_velocity = {simulation.velocity.x, simulation.velocity.y,
                                     simulation.velocity.z};
        }
    }
    owner.launch = prepare_grenade_throw(input, trace);
    owner.state.owner_entity = frame.player_entity;
    simulate_grenade(owner.state, frame.kind, owner.launch.position, owner.launch.velocity,
                     frame.cached_motion ? environment.tick_interval : 0.0f, 0.0f, environment,
                     trace, &projection, &owner.path);
    finalize_grenade(owner.state, trace);
    owner.drawing_position = owner.state.position;
    if (frame.kind == GrenadeKind::explosive || frame.kind == GrenadeKind::flash)
        owner.state.position.z += 1.0f;
    return true;
}
GrenadePredictionDrawResult render_grenade_prediction(ui::RenderQueue& queue,
                                                      const GrenadeEspConfig& config,
                                                      GrenadeTrajectoryOwner& owner,
                                                      const GrenadeTrajectoryFrame& frame,
                                                      const GrenadePredictionScene& scene) {
    GrenadePredictionDrawResult result;
    result.trajectory = predict_grenade_trajectory(
        owner, config, frame, scene.movement, scene.environment, scene.trace, scene.projection);
    if (frame.engine_reject_148 || frame.engine_reject_174 || frame.game_player_override)
        return result;
    if (result.trajectory) {
        GrenadeAuxiliaryResult presentation;
        if (frame.kind == GrenadeKind::explosive) {
            result.damage = select_grenade_damage(
                owner.state.position, scene.damage_candidates, scene.maximum_players,
                scene.damage_threshold, scene.targeting, scene.damage_engine, scene.damage_math);
            presentation.magnitude = _mm_cvttss_si32(_mm_set_ss(result.damage.damage));
            presentation.highlight = result.damage.reached_threshold;
            if (presentation.magnitude > 0)
                presentation.annotation = detail::decimal(presentation.magnitude) + u" dmg";
        } else
            presentation = evaluate_grenade_auxiliary(owner.state, scene.auxiliary);
        GrenadeScreenPoint endpoint;
        if (presentation.magnitude > 0 && !presentation.annotation.empty() &&
            project_grenade_point(scene.projection, owner.drawing_position, endpoint)) {
            queue.text(_mm_cvttss_si32(_mm_set_ss(endpoint.x - 4.0f)),
                       _mm_cvttss_si32(_mm_set_ss(endpoint.y - 8.0f)),
                       presentation.highlight ? 0xff15c28fu : 0xc8ffffffu, 0x18, 0,
                       presentation.annotation);
        }
        if (presentation.draw_path)
            draw_grenade_path(queue, config, owner.path, scene.projection, owner.drawing_position,
                              presentation.magnitude);
    }
    draw_grenade_warnings(queue, config, scene.projection, scene.warning_records,
                          scene.warning_entities, scene.viewer, scene.explosive_texture,
                          scene.fire_texture);
    return result;
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_prediction_trace.cpp
// ============================================================================

namespace skeet::esp::grenade_detail {
namespace {
struct SnapshotTraceEntities {};
bool is_player(const GrenadeHitEntity& entity, const SnapshotTraceEntities&) noexcept {
    return entity.player;
}
bool is_player(const GrenadeHitEntity& entity, const GrenadeEnvironmentBindings&) {
    return engine::invoke<bool>(reinterpret_cast<void*>(std::uintptr_t(entity.identity)), 158);
}
GrenadeRay ray(Vector3 start, Vector3 displacement, bool hull) {
    const Vector3 delta{(start.x + displacement.x) - start.x, (start.y + displacement.y) - start.y,
                        (start.z + displacement.z) - start.z};
    return {start, delta, hull ? Vector3{2, 2, 2} : Vector3{},
            (delta.y * delta.y + delta.x * delta.x) + delta.z * delta.z != 0.0f, !hull};
}
}
template <class Environment>
GrenadeTrace move_impl(GrenadeState& state, Vector3 displacement, GrenadeTraceProvider provider,
                       const Environment& environment) {
    const auto start = state.position;
    auto mask = state.collision_group == 1 ? 0x8400bu : 0x4208608bu;
    auto input = ray(start, displacement, true);
    auto filter =
        GrenadeTraceFilter{state.owner_entity, state.ignored_handle, state.collision_group,
                           std::uint16_t(state.bounces), state.kind};
    auto hit = provider.trace(provider.context, input, mask, filter);
    if (hit.start_solid && (hit.contents & 0x80000u))
        hit = provider.trace(provider.context, input, mask & 0xfff7ffffu, filter);
    if (hit.fraction < 1.0f || hit.all_solid || hit.start_solid) {
        if (hit.entity.identity && is_player(hit.entity, environment)) {
            input = ray(start, displacement, false);
            hit = provider.trace(provider.context, input, mask, filter);
        }
        if (hit.start_solid) {
            state.collision_group = 3;
            filter.collision_group = 3;
            const Vector3 from{start.x - displacement.x, start.y - displacement.y,
                               start.z - displacement.z};
            const Vector3 delta{(displacement.x + start.x) - from.x,
                                (displacement.y + start.y) - from.y,
                                (displacement.z + start.z) - from.z};
            input = {from,
                     delta,
                     {},
                     (delta.y * delta.y + delta.x * delta.x) + delta.z * delta.z != 0,
                     true};
            hit = provider.trace(provider.context, input, 0x8400bu, filter);
        }
    }
    if (hit.fraction != 0.0f)
        state.position = hit.end;
    return hit;
}
GrenadeTrace move(GrenadeState& state, Vector3 displacement, GrenadeTraceProvider provider) {
    return move_impl(state, displacement, provider, SnapshotTraceEntities{});
}
GrenadeTrace move(GrenadeState& state, Vector3 displacement, GrenadeTraceProvider provider,
                  const GrenadeEnvironmentBindings& environment) {
    return move_impl(state, displacement, provider, environment);
}
} // namespace skeet::esp::grenade_detail

// ============================================================================
// Subcomponent: grenade_prediction_warning.cpp
// ============================================================================

namespace skeet::esp {
namespace {
/* [removed duplicate clamp] */
/* [removed duplicate truncate] */
std::int32_t nearest(float value) {
    return _mm_cvtss_si32(_mm_set_ss(value));
}
std::int32_t edge(std::int32_t value, std::int32_t size) {
    const auto maximum = std::bit_cast<std::int32_t>(std::uint32_t(size) - 50u);
    if (maximum < 50)
        return maximum;
    return std::clamp(value, 50, maximum);
}
float separation(Vector3 a, Vector3 b) {
    const auto x = a.x - b.x, y = a.y - b.y, z = a.z - b.z;
    return std::sqrt((x * x + y * y) + z * z);
}
std::uint32_t damage_color(float amount) { // 433CF469 -> existing native HSV helpers
    if (amount <= 0)
        return 0xbe323232u;
    if (amount >= 1)
        return 0xbe0000ffu;
    const auto first = ui::rgb_to_hsv(0xbe323232u), second = ui::rgb_to_hsv(0xbe0000ffu);
    return ui::hsv_to_rgb({(second.hue - first.hue) * amount + first.hue,
                           (second.saturation - first.saturation) * amount + first.saturation,
                           (second.value - first.value) * amount + first.value});
}
struct Vertex {
    float x, y;
    std::uint32_t color;
};
void damage_fan(ui::RenderQueue& queue, Point point, float fraction, float warning_radius) {
    const auto color = damage_color(std::sqrt(fraction)), outer = color & 0xffffffu;
    const auto angle = std::bit_cast<float>(0x3dcf8c3du);
    const auto cosine = std::cos(angle), sine = std::sin(angle);
    const auto radius = warning_radius - 4.0f;
    const auto cx = static_cast<float>(point.x), cy = static_cast<float>(point.y);
    auto x = std::cos(0.0f), y = std::sin(0.0f);
    std::array<Vertex, 128> vertices{};
    vertices[0] = {cx, cy, color};
    vertices[1] = {cx, cy, color};
    vertices[2] = {x * radius + cx, y * radius + cy, outer};
    float last_x = vertices[2].x, last_y = vertices[2].y;
    for (unsigned i = 0; i != 62; ++i) {
        const auto next_x = x * cosine - y * sine;
        y = y * cosine + x * sine;
        x = next_x;
        last_x = x * radius + cx;
        last_y = y * radius + cy;
        vertices[3 + i * 2] = {cx, cy, color};
        vertices[4 + i * 2] = {last_x, last_y, outer};
    }
    vertices[127] = {last_x, last_y, outer};
    auto* payload =
        queue.append(17, static_cast<std::uint32_t>(vertices.size() * sizeof(Vertex) + 20));
    const std::uint32_t header[]{3, 128};
    std::memcpy(payload, header, 8);
    std::memcpy(payload + 8, vertices.data(), sizeof(vertices));
}
}
bool project_grenade_warning(const Projection& projection, Vector3 position, Point& point,
                             bool& in_front) noexcept {
    const auto& m = projection.matrix;
    auto w = (position.x * m[12] + m[13] * position.y) + position.z * m[14] + m[15];
    if (w == 0.0f) {
        point = {truncate(projection.horizontal_scale), truncate(projection.vertical_scale * 2.0f)};
        return false;
    }
    in_front = true;
    if (w < 0.0f) {
        w = -w;
        in_front = false;
    }
    const auto reciprocal = 1.0f / w;
    const auto horizontal = ((position.y * m[1] + position.x * m[0]) + position.z * m[2]) + m[3];
    const auto vertical = ((position.y * m[5] + position.x * m[4]) + position.z * m[6]) + m[7];
    point = {nearest((reciprocal * horizontal - -1.0f) * projection.horizontal_scale),
             nearest((1.0f - vertical * reciprocal) * projection.vertical_scale)};
    return true;
}
namespace {
struct FixedWarningView {
    const GrenadeWarningFrame& frame;
    GrenadeWarningTexture image;
    std::int32_t width() const {
        return frame.width;
    }
    std::int32_t height() const {
        return frame.height;
    }
    std::int32_t health() const {
        return frame.health;
    }
    float now() const {
        return (float(frame.tick_count) * frame.tick_interval - frame.tick_interval) +
               frame.interpolation * frame.tick_interval;
    }
    GrenadeWarningTexture texture(GrenadeKind) const {
        return image;
    }
};
struct LiveWarningView {
    const GrenadeWarningLiveFrame& frame;
    std::int32_t width() const {
        return frame.renderer.screen_width;
    }
    std::int32_t height() const {
        return frame.renderer.screen_height;
    }
    std::int32_t health() const {
        return engine::invoke<std::int32_t>(frame.player, 122);
    }
    float now() const {
        return (float(frame.tick_count) * frame.tick_interval - frame.tick_interval) +
               frame.interpolation * frame.tick_interval;
    }
    GrenadeWarningTexture texture(GrenadeKind kind) const {
        auto image = kind == GrenadeKind::explosive ? frame.explosive_texture : frame.fire_texture;
        if (image.texture != 0xffffffffu)
            frame.renderer.texture_size(image.texture, &image.width, &image.height);
        return image;
    }
};
template <class View>
void warning(ui::RenderQueue& queue, const Projection& projection,
             const GrenadeWarningRecord& record, float distance, const View& view) {
    if (!(distance <= 400.0f))
        return;
    const auto width = view.width(), height = view.height();
    Point point{};
    bool front = false;
    if (!project_grenade_warning(projection, record.position, point, front))
        return;
    if (!front)
        point.y = height;
    point = {edge(point.x, width), edge(point.y, height)};
    const auto radius = std::round(50.0f - ((clamp(distance, 36, 200) - 36.0f) * 20.0f) / 164.0f);
    queue.circle({point.x, point.y, 0xd2000000u, radius, 0, 1});
    if (record.damage > 0) {
        const auto health = view.health();
        if (health > 0) {
            const auto fraction =
                clamp(record.damage / std::min(50.0f, static_cast<float>(health)), 0, 1);
            if (fraction > 0)
                damage_fan(queue, point, fraction, radius);
        }
    }
    const auto duration = record.end_time - record.creation_time;
    if (duration > 0 && duration < 8.0f) {
        const auto now = view.now();
        const auto fraction = clamp((now - record.creation_time) / duration, 0, 1);
        queue.circle_outline({point.x, point.y, 0xc8ffffffu, radius - 1.0f, 0, 1.0f - fraction, 2});
    }
    const auto texture = view.texture(record.kind);
    if (texture.texture == 0xffffffffu) {
        queue.text(point.x, detail::sub(point.y, 10), 0xc828ffffu, 0x12, 0, u"!");
        point.y = detail::add(point.y, 10);
    } else {
        queue.texture({detail::sub(point.x, texture.width / 2),
                       detail::sub(detail::sub(point.y, texture.height / 2), 10), texture.texture,
                       0xc8ffffffu, 0, 0, texture.width, texture.height, 0, 1});
        point.y = detail::add(point.y, texture.height / 2);
    }
    const auto value =
        record.kind == GrenadeKind::explosive ? truncate(record.damage) : nearest(distance / 12.0f);
    queue.text(point.x, point.y, 0xc8ffffffu, 0x11, 0, detail::decimal(value));
}
}
void draw_grenade_warning(ui::RenderQueue& queue, const Projection& projection,
                          const GrenadeWarningRecord& record, float distance,
                          const GrenadeWarningFrame& frame, GrenadeWarningTexture image) {
    warning(queue, projection, record, distance, FixedWarningView{frame, image});
}
void draw_grenade_warning(ui::RenderQueue& queue, const Projection& projection,
                          const GrenadeWarningRecord& record, float distance,
                          const GrenadeWarningLiveFrame& frame) {
    warning(queue, projection, record, distance, LiveWarningView{frame});
}
void draw_grenade_warnings(ui::RenderQueue& queue, const GrenadeEspConfig& config,
                           const Projection& projection, std::span<GrenadeWarningRecord> records,
                           std::span<const GrenadeWarningEntity> entities,
                           const GrenadeWarningFrame& frame, GrenadeWarningTexture explosive,
                           GrenadeWarningTexture fire) {
    if (!config.proximity_warning.value || !frame.local_present || frame.life_state != 0)
        return;
    if (frame.movement_type == 10 && (frame.observer_mode == 4 || frame.observer_mode == 5))
        return;
    for (std::size_t i = 0; i < records.size(); ++i) {
        auto& record = records[i];
        if (record.handle == 0xffffffffu || i >= entities.size() || !entities[i].present ||
            entities[i].effects_bit5) {
            record.handle = 0xffffffffu;
            continue;
        }
        const auto& entity = entities[i];
        if ((record.type <= 1 && entity.explosion_effect_tick != 0) || record.suppressed)
            continue;
        auto distance =
            separation(record.type == 2 ? entity.origin : record.position, frame.local_position);
        if (record.type == 2)
            for (const auto& cell : entity.fire_cells) {
                const auto next = separation(cell, frame.local_position);
                distance = _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(next), _mm_set_ss(distance)));
            }
        draw_grenade_warning(queue, projection, record, distance, frame,
                             record.kind == GrenadeKind::explosive ? explosive : fire);
    }
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_warning_owner.cpp
// ============================================================================

namespace skeet::esp {
namespace {
/* [removed duplicate truncate] */
/* [removed duplicate subtract] */
/* [removed duplicate squared] */
/* [removed duplicate clamp] */
bool resolve_thrower(const GrenadeWarningSnapshot& projectile, std::uint32_t type,
                     GrenadeWarningEngine& engine, GrenadeWarningSnapshot& thrower) {
    const auto handle = type < 2 ? projectile.thrower_handle : projectile.owner_handle;
    return handle != 0xffffffffu && engine.snapshot_for_handle(handle, thrower) && thrower.player;
}
Vector3 damage_distance_origin(Vector3 explosion, float remaining,
                               const GrenadeWarningSnapshot& local, GrenadeWarningEngine& engine,
                               GrenadeTraceProvider provider) {
    auto origin = local.virtual_origin;
    if (!((local.velocity.y * local.velocity.y + local.velocity.x * local.velocity.x) > 400.0f))
        return origin;
    const auto time = clamp(remaining, 0.2f, 1.6f);
    const Vector3 predicted{time * local.velocity.x + origin.x, time * local.velocity.y + origin.y,
                            time * local.velocity.z + origin.z};
    if (!(squared(subtract(origin, explosion)) > squared(subtract(predicted, explosion))))
        return origin;
    const auto delta = subtract(predicted, origin);
    MovementTraceFlagGuard guard(provider.trace_flag);
    const auto result = engine.trace({origin, delta, {}, squared(delta) != 0, true}, 1,
                                     {GrenadeDamageFilterKind::ignore_entity, local.identity});
    if (!result.start_solid)
        origin = result.end;
    return origin;
}
void refresh_existing(GrenadeWarningOwner& owner, const GrenadeWarningUpdateFrame& frame,
                      const GrenadeEnvironment& environment, GrenadeWarningEngine& engine,
                      GrenadeTraceProvider trace) {
    for (auto& record : owner.records) {
        GrenadeWarningSnapshot projectile, thrower;
        if (record.handle == 0xffffffffu ||
            !engine.snapshot_for_handle(record.handle, projectile) || projectile.dormant ||
            record.type > 1 || projectile.explosion_effect_tick != 0 ||
            (projectile.flags & 0x20u) || projectile.thrower_handle == 0xffffffffu ||
            !engine.snapshot_for_handle(projectile.thrower_handle, thrower))
            continue;
        owner.projectile_points.clear();
        owner.projectile_points.push_back(projectile.virtual_origin);
        if (!(squared(projectile.velocity) > 900.0f))
            continue;
        const auto current =
            squared(subtract(projectile.velocity, projectile.initial_velocity)) < 1.0f
                ? static_cast<float>(frame.tick_count) * environment.tick_interval
                : projectile.simulation_time;
        GrenadeState state{};
        state.owner_entity = projectile.identity;
        simulate_grenade(state, record.kind, projectile.origin, projectile.velocity, current,
                         record.creation_time, environment, trace, nullptr, nullptr,
                         projectile.collision_group, projectile.projectile_flags);
        finalize_grenade(state, trace);
        if (record.kind == GrenadeKind::explosive)
            state.position.z += 1.0f;
        if (squared(subtract(state.position, record.position)) > 4096.0f)
            record.position = state.position;
    }
}
bool already_recorded(const GrenadeWarningOwner& owner, std::uint32_t index) {
    return std::any_of(owner.records.begin(), owner.records.end(), [index](const auto& record) {
        return (record.handle == 0xffffffffu ? 0x1fffu : record.handle & 0xffffu) == index;
    });
}
}

std::int32_t grenade_warning_team(const GrenadeWarningSnapshot& player,
                                  std::int32_t game_type) noexcept {
    if (player.dormant)
        return player.cached_team_valid ? player.cached_team : player.team;
    return game_type == 6 ? player.survival_team : player.team;
}
bool grenade_warning_expired(const GrenadeWarningRecord& record, bool present,
                             std::int32_t explosion_tick, std::int32_t tick_count,
                             float interval) noexcept {
    if (record.type < 2 && (record.handle == 0xffffffffu || !present || explosion_tick != 0))
        return true;
    return truncate((record.end_time + 1.0f) / interval + 0.5f) < tick_count;
}
bool grenade_warning_admitted(std::uint32_t type, const GrenadeWarningSnapshot* thrower,
                              const GrenadeWarningSnapshot& local,
                              const GrenadeWarningRules& rules) noexcept {
    if (!thrower || !thrower->player)
        return true;
    if (thrower->identity == local.identity)
        return type != 0 || rules.self_he_damage != 0.0f;
    if (grenade_warning_team(*thrower, rules.game_type) !=
            grenade_warning_team(local, rules.game_type) ||
        rules.teammates_are_enemies)
        return true;
    if (!rules.friendly_fire)
        return false;
    return (type == 0 ? rules.teammate_he_damage : rules.teammate_fire_damage) != 0.0f;
}
float grenade_warning_damage_scale(std::uint32_t type, const GrenadeWarningSnapshot* thrower,
                                   const GrenadeWarningSnapshot& local,
                                   const GrenadeWarningRules& rules) noexcept {
    const auto teammate = thrower && thrower->identity != local.identity &&
                          thrower->team == local.team &&
                          (thrower->index == local.index || !rules.teammates_are_enemies);
    if (teammate && (!rules.friendly_fire || rules.game_player_override))
        return 0.0f;
    if (thrower && thrower->identity == local.identity && type == 0)
        return rules.self_he_damage;
    if (!teammate)
        return 1.0f;
    if (rules.teammate_damage_disabled)
        return 0.0f;
    return type == 0 ? rules.teammate_he_damage : rules.teammate_fire_damage;
}
float grenade_projectile_elapsed(float initial_z, float current_z, float gravity,
                                 float interval) noexcept {
    return static_cast<float>(
               truncate((initial_z - current_z) / ((gravity * 0.4f) * interval) + 0.5f)) *
           interval;
}
bool grenade_warning_smoke_overlap(Vector3 fire, Vector3 smoke) noexcept {
    const auto height = fire.z - smoke.z;
    if (height < -64.0f || height > 120.0f)
        return false;
    auto radius_squared = 22500.0f;
    if (!(height > 72.0f)) {
        if (height < 18.0f) {
            const auto fraction = clamp((height - 12.0f) / -76.0f, 0.0f, 1.0f);
            radius_squared -=
                clamp((fraction + 0.0f) * (fraction + 0.0f) - 0.0f, 0.0f, 1.0f) * 6116.0f;
        }
    } else {
        const auto fraction = clamp((height - 72.0f) / 48.0f, 0.0f, 1.0f);
        radius_squared -=
            clamp((fraction + 0.0f) * (fraction + 0.0f) - 0.0f, 0.0f, 1.0f) * 12500.0f;
    }
    const auto dx = fire.x - smoke.x, dy = fire.y - smoke.y;
    return dy * dy + dx * dx <= radius_squared;
}
void assess_grenade_warning_record(GrenadeWarningRecord& record,
                                   const GrenadeWarningUpdateFrame& frame,
                                   GrenadeWarningEngine& engine, GrenadeTraceProvider trace,
                                   const GrenadeDamageMath& math, std::uint32_t& command_buttons) {
    if (record.type == 0) {
        const auto remaining = record.end_time - frame.current_time;
        auto player = *frame.local_damage;
        player.distance_origin =
            damage_distance_origin(record.position, remaining, *frame.local, engine, trace);
        auto damage = evaluate_he_damage(record.position, player, engine, math).damage;
        if (damage > 0.0f && frame.actual_local) {
            GrenadeWarningSnapshot current_projectile, current_thrower;
            if (record.handle != 0xffffffffu &&
                engine.snapshot_for_handle(record.handle, current_projectile)) {
                const auto current_resolved =
                    resolve_thrower(current_projectile, record.type, engine, current_thrower);
                damage *= grenade_warning_damage_scale(
                    record.type, current_resolved ? &current_thrower : nullptr, *frame.actual_local,
                    frame.rules);
            }
        }
        if (record.damage > damage && remaining > 0.0f && remaining < 0.03125f)
            command_buttons = (command_buttons & 0xfffffbffu) | 0x200u;
        record.damage = damage;
    } else if (record.type == 1) {
        record.suppressed = false;
        std::int32_t collected = 0;
        for (const auto& smoke : engine.partition_entities(record.position, 166.0f, 0x80, false)) {
            if (!smoke.resolved || !smoke.base_entity_present ||
                (frame.classes.smoke_projectile &&
                 smoke.native_class != frame.classes.smoke_projectile))
                continue;
            if (collected++ == 16)
                break;
            if (smoke.smoke_active &&
                grenade_warning_smoke_overlap(record.position, smoke.origin)) {
                record.suppressed = true;
                break;
            }
        }
        record.damage = squared(subtract(record.position, frame.local->virtual_origin)) < 22500.0f
                            ? 100.0f
                            : 0.0f;
    } else if (record.type == 2) {
        record.damage = 0.0f;
        GrenadeWarningSnapshot inferno;
        if (record.handle == 0xffffffffu || !engine.snapshot_for_handle(record.handle, inferno))
            return;
        auto distance = std::sqrt(squared(subtract(frame.local->virtual_origin, inferno.origin)));
        for (const auto& cell : inferno.fire_cells) {
            const auto next = std::sqrt(squared(subtract(cell, frame.local->virtual_origin)));
            distance = _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(next), _mm_set_ss(distance)));
        }
        if (distance < 150.0f)
            record.damage = 100.0f;
    }
}
void update_grenade_warning_records(GrenadeWarningOwner& owner, const GrenadeEspConfig& config,
                                    const GrenadeWarningUpdateFrame& frame,
                                    const GrenadeEnvironment& environment,
                                    GrenadeWarningEngine& engine, GrenadeTraceProvider trace,
                                    const GrenadeDamageMath& math) {
    owner.command_buttons = frame.command_buttons;
    std::erase_if(owner.records, [&](const auto& record) {
        GrenadeWarningSnapshot projectile;
        const auto present = record.type < 2 && record.handle != 0xffffffffu &&
                             engine.snapshot_for_handle(record.handle, projectile);
        return grenade_warning_expired(record, present, projectile.explosion_effect_tick,
                                       frame.tick_count, environment.tick_interval);
    });
    if (!frame.local || (!config.proximity_warning.value && !frame.grenade_entities_enabled))
        return;
    refresh_existing(owner, frame, environment, engine, trace);
    for (const auto& update : frame.packet_updates) {
        if (update.state != 0 || already_recorded(owner, update.index) ||
            static_cast<std::int32_t>(update.index) <= 64 || (update.index & 0xffff0000u))
            continue;
        GrenadeWarningSnapshot projectile, thrower;
        if (!engine.snapshot_for_index(update.index, projectile))
            continue;
        std::uint32_t type;
        GrenadeKind kind;
        float lifetime;
        if (projectile.native_class == frame.classes.base_projectile) {
            if (projectile.damage != 350.0f)
                continue;
            type = 0;
            kind = GrenadeKind::explosive;
            lifetime = 1.5f;
        } else if (projectile.native_class == frame.classes.molotov_projectile) {
            type = 1;
            kind = GrenadeKind::molotov;
            lifetime = environment.incendiary_duration;
        } else if (projectile.native_class == frame.classes.inferno) {
            const auto resolved = resolve_thrower(projectile, 2, engine, thrower);
            if (!grenade_warning_admitted(2, resolved ? &thrower : nullptr, *frame.local,
                                          frame.rules))
                continue;
            const auto start =
                static_cast<float>(projectile.inferno_start_tick) * environment.tick_interval;
            owner.records.push_back({projectile.reference_handle, projectile.origin, 0, 2,
                                     GrenadeKind::molotov, false, start, start + 7.0f});
            continue;
        } else
            continue;
        if (projectile.explosion_effect_tick != 0 || projectile.thrower_handle == 0xffffffffu)
            continue;
        const auto resolved = resolve_thrower(projectile, type, engine, thrower);
        if (!grenade_warning_admitted(type, resolved ? &thrower : nullptr, *frame.local,
                                      frame.rules))
            continue;
        const auto elapsed =
            grenade_projectile_elapsed(projectile.initial_velocity.z, projectile.velocity.z,
                                       environment.gravity, environment.tick_interval);
        if (!(elapsed < lifetime))
            continue;
        const auto creation = projectile.simulation_time - elapsed;
        // Engine virtual+38 is x87. The native owner stores the rounded quotient
        // to a float before CVTTSS2SI, then multiplies by the global tick interval.
        const auto clock_ticks = static_cast<float>(
            static_cast<long double>(frame.engine_time) / frame.engine_clock_interval + 0.5L);
        const auto current = static_cast<float>(truncate(clock_ticks)) * environment.tick_interval;
        GrenadeState state{};
        state.owner_entity = projectile.identity;
        simulate_grenade(state, kind, projectile.origin, projectile.velocity, current, creation,
                         environment, trace, nullptr, nullptr, projectile.collision_group,
                         projectile.projectile_flags);
        GrenadeWarningRecord record{
            projectile.reference_handle, {}, 0, type, kind, false, creation, state.time};
        finalize_grenade(state, trace);
        if (kind == GrenadeKind::explosive)
            state.position.z += 1.0f;
        else if (kind == GrenadeKind::molotov && !state.fire_ground)
            continue;
        record.position = state.position;
        assess_grenade_warning_record(record, frame, engine, trace, math, owner.command_buttons);
        owner.records.push_back(record);
    }
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: grenade_warning_runtime.cpp
// ============================================================================

namespace skeet::esp {
void draw_grenade_warning_records(ui::RenderQueue& queue, const GrenadeEspConfig& config,
                                  const Projection& projection, GrenadeWarningOwner& owner,
                                  GrenadeWarningEngine& engine, const GrenadeWarningFrame& frame,
                                  GrenadeWarningTexture explosive, GrenadeWarningTexture fire) {
    if (!config.proximity_warning.value || !frame.local_present || frame.life_state != 0)
        return;
    if (frame.movement_type == 10 && (frame.observer_mode == 4 || frame.observer_mode == 5))
        return;
    for (auto& record : owner.records) {
        GrenadeWarningSnapshot snapshot;
        GrenadeWarningEntity entity{};
        if (record.handle != 0xffffffffu && engine.snapshot_for_handle(record.handle, snapshot)) {
            entity = {true, bool(snapshot.flags & 0x20u), snapshot.explosion_effect_tick,
                      snapshot.origin, snapshot.fire_cells};
        }
        draw_grenade_warnings(queue, config, projection, {&record, 1}, {&entity, 1}, frame,
                              explosive, fire);
    }
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: hitbox_highlight.cpp
// ============================================================================

namespace skeet::esp {
namespace {
/* [removed duplicate Vertex] */
float length(const RageVector& value) noexcept {
    return std::sqrt((value[0] * value[0] + value[1] * value[1]) + value[2] * value[2]);
}
RageVector normalize(const RageVector& value) noexcept {
    const float inverse = 1.0f / (length(value) + std::numeric_limits<float>::epsilon());
    return {value[0] * inverse, value[1] * inverse, value[2] * inverse};
}
bool project(const Projection& projection, const RageVector& position, bool support,
             Vertex& result) noexcept {
    const auto& m = projection.matrix;
    const float w = (support ? (position[1] * m[13] + position[0] * m[12])
                             : (position[0] * m[12] + position[1] * m[13])) +
                    position[2] * m[14] + m[15];
    if (w < 0.001f)
        return false;
    const float inverse = 1.0f / w;
    const float horizontal = (support ? (position[1] * m[1] + position[0] * m[0])
                                      : (position[0] * m[0] + position[1] * m[1])) +
                             position[2] * m[2] + m[3];
    const float vertical = (support ? (position[1] * m[5] + position[0] * m[4])
                                    : (position[0] * m[4] + position[1] * m[5])) +
                           position[2] * m[6] + m[7];
    result.x = (horizontal * inverse) * projection.horizontal_scale + projection.horizontal_scale;
    result.y = projection.vertical_scale - (vertical * inverse) * projection.vertical_scale;
    return true;
}
}
void draw_capsule_highlight(ui::RenderQueue& queue, const Projection& projection,
                            const RageVector& origin, const RageCapsule& capsule,
                            std::uint32_t color, const RageTrigonometry& trig) {
    const RageVector center{(capsule.first[0] + capsule.second[0]) * 0.5f,
                            (capsule.second[1] + capsule.first[1]) * 0.5f,
                            (capsule.second[2] + capsule.first[2]) * 0.5f};
    const auto axis =
        normalize({center[0] - origin[0], center[1] - origin[1], center[2] - origin[2]});
    const float sine = trig.sine(0.5235987901687622f), cosine = trig.cosine(0.5235987901687622f);
    const float complement = 1.0f - cosine;
    const float xy = (axis[1] * axis[0]) * complement;
    const float xz = (axis[0] * axis[2]) * complement;
    const float yz = (axis[1] * axis[2]) * complement;
    Vertex projected_center{0, 0, color};
    if (!project(projection, center, false, projected_center))
        return;
    RageVector side{0.0f, -1.0f, 0.0f};
    if (!(axis[0] == 0.0f && axis[1] == 0.0f))
        side = normalize(
            {axis[1] - axis[2] * 0.0f, axis[2] * 0.0f - axis[0], axis[0] * 0.0f - axis[1] * 0.0f});
    std::array<Vertex, 12> perimeter;
    for (auto& point : perimeter) {
        const RageVector segment{capsule.second[0] - capsule.first[0],
                                 capsule.second[1] - capsule.first[1],
                                 capsule.second[2] - capsule.first[2]};
        const float dot = (segment[1] * side[1] + segment[0] * side[0]) + segment[2] * side[2];
        const auto& endpoint = dot >= 0.0f ? capsule.second : capsule.first;
        const float radius = capsule.radius / length(side);
        const RageVector support{side[0] * radius + endpoint[0], side[1] * radius + endpoint[1],
                                 side[2] * radius + endpoint[2]};
        point.color = color & 0xffffff;
        if (!project(projection, support, true, point))
            return;
        const float x = side[0], y = side[1], z = side[2];
        side[0] = ((xy - axis[2] * sine) * y +
                   ((1.0f - axis[0] * axis[0]) * cosine + axis[0] * axis[0]) * x) +
                  (axis[1] * sine + xz) * z;
        side[2] = z * ((1.0f - axis[2] * axis[2]) * cosine + axis[2] * axis[2]) +
                  ((xz - axis[1] * sine) * x + (axis[0] * sine + yz) * y);
        side[1] = ((axis[2] * sine + xy) * x +
                   ((1.0f - axis[1] * axis[1]) * cosine + axis[1] * axis[1]) * y) +
                  (yz - axis[0] * sine) * z;
    }
    std::array<Vertex, 36> vertices;
    for (unsigned i = 0; i < 12; ++i) {
        vertices[i * 3] = perimeter[i];
        vertices[i * 3 + 1] = projected_center;
        vertices[i * 3 + 2] = perimeter[(i + 1) % 12];
    }
    const std::uint32_t count = 36;
    auto* payload = queue.append(7, count * sizeof(Vertex) + 16);
    std::memcpy(payload, &count, 4);
    std::memcpy(payload + 4, vertices.data(), sizeof(vertices));
}
void draw_hitbox_highlight(ui::RenderQueue& queue, const AimbotVisualizationConfig& config,
                           HitboxHighlightState& state, const RuntimePlayer& local,
                           const HitboxHighlightContext& context) {
    if (!context.active)
        return;
    if (local.life_state) {
        state.target_handle = -1;
        return;
    }
    if (!config.enabled.value && !config.safe_point.value)
        return;
    if (state.target_handle == -1)
        return;
    const auto target = context.by_handle(context.engine_context, state.target_handle);
    if (!target.player ||
        !rage::target_is_eligible(context.targets, target.player->engine_entity) ||
        target.player->move_type == 8 || target.player->move_type == 10 ||
        (target.movement_flags & 0x40)) {
        state.target_handle = -1;
        return;
    }
    if (!target.model || !target.model->studio_header || target.hitboxes.empty())
        return;
    for (const auto& entry : state.entries) {
        if (!entry.flags)
            continue;
        if (entry.hitbox >= std::int32_t(target.hitboxes.size()))
            return;
        const auto& hitbox = target.hitboxes[std::size_t(entry.hitbox)];
        if (!(hitbox.radius > 0.0f))
            return;
        HitboxMatrices matrices{};
        matrices[0] = target.bones[target.model->hitbox_bones[std::size_t(entry.hitbox)]];
        const auto transformed =
            prepare_rage_scan_hitboxes({&hitbox, 1}, matrices, context.trigonometry);
        draw_capsule_highlight(queue, context.projection, context.view_origin,
                               transformed[0].capsule,
                               entry.flags & 2 ? config.safe_point_color.value : config.color.value,
                               context.trigonometry);
    }
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: hitbox_highlight_engine.cpp
// ============================================================================

namespace skeet::esp {
namespace {
/* [removed duplicate field] */
std::ptrdiff_t offset(const engine::NativeOffsetStorage& storage, std::uint32_t pointer,
                      std::uint32_t key, std::uint32_t mask) {
    const auto& globals = storage.globals;
    return static_cast<std::ptrdiff_t>(*globals.pointers.at(pointer) ^ globals.keys.at(key) ^ mask);
}
bool eligible(const RuntimePlayer* target, const HitboxHighlightEngineContext& c) {
    // Complete live43358962. In particular the local entity and game ConVars
    // are read only after target engine queries, rather than borrowed from a
    // previously assembled TargetBindings snapshot.
    auto& players = c.players.get();
    if (!target || !players.is_player(*target) || players.life(*target) ||
        players.health(*target) <= 0 || players.dormant(*target) || players.suppressed(*target) ||
        players.eligibility(*target))
        return false;
    const auto index = players.index(*target);
    if (c.player_adjustments.records[std::uint32_t(index)].adjustments.whitelist)
        return false;
    if (field<std::uint8_t>(target->engine_entity,
                            offset(c.offsets, 0x43467dd0, 0x4346a610, 0xaf649dac)))
        return false;
    return live_enemy(players.local(), target, players);
}
}

EnginePlayers& HitboxHighlightPlayers::get() const {
    if (const auto* fixture = std::get_if<std::reference_wrapper<EnginePlayers>>(&source_))
        return fixture->get();
    auto* owner = std::get<std::reference_wrapper<EspOwnerRuntime* const>>(source_).get();
    if (!owner)
        throw std::logic_error("RAGE highlight requires the actual published ESP player owner");
    return owner->player_engine();
}

std::uint8_t HitboxHighlightEngineContext::life_state(const void* local) const {
    return field<std::uint8_t>(local, offset(offsets, 0x43468e78, 0x434684e4, 0x9d920e35));
}

void draw_live_hitbox_highlight(ui::RenderQueue& queue, const AimbotVisualizationConfig& config,
                                HitboxHighlightState& state, void* local,
                                const HitboxHighlightEngineContext& c) {
    if (!c.active)
        return;
    if (c.life_state(local)) {
        state.target_handle = -1;
        return;
    }
    if (!config.enabled.value && !config.safe_point.value)
        return;
    if (state.target_handle == -1)
        return;
    auto& players = c.players.get();
    auto* target = players.by_handle(state.target_handle);
    if (!eligible(target, c)) {
        state.target_handle = -1;
        return;
    }
    const auto movement = players.movement(*target);
    if (movement == 8 || movement == 10 ||
        (field<std::uint8_t>(target->engine_entity,
                             offset(c.offsets, 0x434688d0, 0x43468c24, 0xe2fbe695)) &
         0x40u)) {
        state.target_handle = -1;
        return;
    }
    const auto* wrapper = field<const void*>(target->engine_entity,
                                             offset(c.offsets, 0x43467e8c, 0x43467900, 0x32e4daf1));
    if (!wrapper)
        return;
    const auto* header = field<const std::byte*>(wrapper, 0);
    if (!header)
        return;
    const auto set_index = field<std::uint32_t>(
        target->engine_entity, offset(c.offsets, 0x43468240, 0x4346a84c, 0xc2b6fb10));
    const auto relative =
        std::bit_cast<std::int32_t>(set_index * 12u + field<std::uint32_t>(header, 0xb0));
    const auto* set = header + relative;
    if (!set || field<std::int32_t>(set, 4) == 0)
        return;
    for (const auto& entry : state.entries) {
        if (!entry.flags)
            continue;
        if (field<std::int32_t>(set, 4) <= entry.hitbox)
            return;
        const auto descriptor_offset = std::bit_cast<std::int32_t>(
            std::uint32_t(entry.hitbox) * 0x44u + field<std::uint32_t>(set, 8));
        const auto* descriptor = set + descriptor_offset;
        const auto radius = field<float>(descriptor, 0x30);
        if (!(radius > 0.0f))
            return; //433D146B COMISS / JBE rejects unordered too
        // The three imported trig pairs precede the live bone-cache lookup.
        // Keep that ordering rather than constructing a target/model snapshot.
        constexpr float radians = 0.01745329238474369f;
        const auto yaw = field<float>(descriptor, 0x28) * radians;
        const auto sy = c.trigonometry.sine(yaw), cy = c.trigonometry.cosine(yaw);
        const auto pitch = field<float>(descriptor, 0x24) * radians;
        const auto sp = c.trigonometry.sine(pitch), cp = c.trigonometry.cosine(pitch);
        const auto roll = field<float>(descriptor, 0x2c) * radians;
        const auto sr = c.trigonometry.sine(roll), cr = c.trigonometry.cosine(roll);
        const auto crcy = cr * cy, crsy = cr * sy, srcy = sr * cy, srsy = sr * sy;
        const Matrix3x4 rotation{{{cp * cy, srcy * sp - crsy, crcy * sp + srsy, 0.0f},
                                  {cp * sy, srsy * sp + crcy, crsy * sp - srcy, 0.0f},
                                  {-sp, sr * cp, cr * cp, 0.0f}}};
        const auto* cache = field<const Matrix3x4*>(
            target->engine_entity, offset(c.offsets, 0x43469064, 0x4346a484, 0x17978090));
        const auto& bone = cache[field<std::uint32_t>(descriptor, 0)];
        Matrix3x4 transform;
        for (unsigned row = 0; row < 3; ++row)
            for (unsigned column = 0; column < 4; ++column) {
                const auto combined = (bone.rows[row][2] * rotation.rows[2][column] +
                                       bone.rows[row][1] * rotation.rows[1][column]) +
                                      bone.rows[row][0] * rotation.rows[0][column];
                transform.rows[row][column] = combined + (column == 3 ? bone.rows[row][3] : 0.0f);
            }
        // Native copies the descriptor only after composing the matrix.
        const auto minimum = field<RageVector>(descriptor, 8);
        const auto maximum = field<RageVector>(descriptor, 0x14);
        RageCapsule capsule{{}, {}, field<float>(descriptor, 0x30)};
        for (unsigned row = 0; row < 3; ++row) {
            const auto& matrix = transform.rows[row];
            capsule.first[row] =
                ((minimum[1] * matrix[1] + matrix[0] * minimum[0]) + matrix[2] * minimum[2]) +
                matrix[3];
            const auto pair = row == 2 ? matrix[1] * maximum[1] + matrix[0] * maximum[0]
                                       : maximum[0] * matrix[0] + maximum[1] * matrix[1];
            capsule.second[row] = (pair + matrix[2] * maximum[2]) + matrix[3];
        }
        draw_capsule_highlight(queue, c.projection, c.view_origin, capsule,
                               (entry.flags & 2u) ? config.safe_point_color.value
                                                  : config.color.value,
                               c.trigonometry);
    }
}
} //namespace skeet::esp

// ============================================================================
// Subcomponent: indicators.cpp
// ============================================================================
#ifndef NOMINMAX
#define NOMINMAX
#endif

namespace skeet::esp {
using namespace detail;
namespace {
i32 rounded(float value) noexcept {
    return _mm_cvtss_si32(_mm_set_ss(value));
}
}

void draw_indicator(ui::RenderQueue& queue, i32& cursor, std::string_view utf8, std::uint32_t color,
                    const IndicatorLayout& layout, i32 texture, float fraction) {
    if (utf8.empty())
        return;
    if (utf8.size() > std::size_t(std::numeric_limits<int>::max()))
        throw std::length_error("Indicator UTF-8 input exceeds the native length field");
    std::array<wchar_t, 8193> wide{};
    const auto converted = MultiByteToWideChar(CP_UTF8, 0, utf8.data(),
                                               static_cast<int>(utf8.size()), wide.data(), 8192);
    if (converted <= 0)
        return;
    const auto length = std::min(converted, 8191);
    wide[length] = 0;
    static_assert(sizeof(wchar_t) == sizeof(char16_t));
    const std::u16string_view text{reinterpret_cast<const char16_t*>(wide.data()),
                                   std::size_t(length)};
    const auto padding_y = rounded(layout.scale * 2.0f);
    const auto spacing = rounded(layout.scale * 4.0f);
    const auto edge = rounded(layout.scale * 25.0f);
    const auto size = ui::measure_text(queue, text, 10);
    Point texture_size{};
    if (texture != -1) {
        if (layout.texture_source)
            layout.texture_source->texture_size(std::uint32_t(texture), &texture_size.x,
                                                &texture_size.y);
        else if (std::uint32_t(texture) < layout.textures.size())
            texture_size = layout.textures[std::uint32_t(texture)];
        if (texture_size.y > size.y) {
            texture_size.x =
                truncate(float(texture_size.x) / float(texture_size.y) * float(size.y) + 0.5f);
            texture_size.y = size.y;
        }
    }
    const bool circle = fraction >= 0.0f;
    i32 diameter = size.x;
    if (circle) {
        if (!queue.fonts)
            throw std::logic_error("Indicator circle requires a font backend");
        diameter =
            sub(ui::font_ascent(*queue.fonts, 2), ui::font_internal_leading(*queue.fonts, 2));
    }
    const auto row_height = add(size.y, mul(padding_y, 2));
    //4340063D queries this queue's screen after text/texture/font metrics;
    //43400644 consumes its height before subtracting the live row cursor.
    const auto screen_height = queue.screen().y;
    const auto y = sub(screen_height, cursor);
    auto texture_x = size.x;
    auto texture_y = size.y;
    auto text_x = edge;
    if (texture != -1) {
        texture_x = add(edge, spacing);
        texture_y = add(y, sub(row_height, texture_size.y) / 2);
        text_x = add(add(texture_size.x, texture_x), spacing);
    }
    text_x = add(text_x, spacing);
    const auto text_y = add(y, sub(row_height, size.y) / 2);
    auto width = add(add(text_x, size.x), spacing);
    i32 circle_x = texture_size.x;
    i32 circle_y = texture_size.y;
    if (circle) {
        circle_x = add(add(width, spacing), diameter / 2);
        circle_y = add(add(y, sub(row_height, diameter) / 2), diameter / 2);
        width = add(add(width, diameter), mul(spacing, 2));
    }
    width = add(width, edge);
    if (color >= 0x01000000u && utf8.front() != '\n') {
        const auto half = width / 2;
        queue.horizontal_gradient({0, y, half, row_height, 0, 0x33000000u});
        queue.horizontal_gradient({half, y, sub(width, half), row_height, 0x33000000u, 0});
        if (texture != -1)
            queue.texture({texture_x, texture_y, std::uint32_t(texture), color, 0, 0,
                           texture_size.x, texture_size.y, 0, 1.0f});
        queue.text(text_x, text_y, color, 10, 0, text);
        if (circle) {
            const auto radius = float(diameter / 2);
            const auto inner = rounded(layout.scale * 3.0f);
            const auto outer = add(inner, rounded(layout.scale * 2.0f));
            queue.circle_outline(
                {circle_x, circle_y, 0xC8000000u, radius + 1.0f, 0.0f, 1.0f, float(outer)});
            if (fraction > 0.0f)
                queue.circle_outline(
                    {circle_x, circle_y, 0xC8FFFFFFu, radius, 0.0f, fraction, float(inner)});
        }
    }
    cursor = add(cursor, add(row_height, rounded(layout.scale * 8.0f)));
}

} // namespace skeet::esp

// ============================================================================
// Subcomponent: line_of_sight.cpp
// ============================================================================

#pragma STDC FENV_ACCESS ON

namespace skeet::esp {
namespace {
/* [removed duplicate field] */
bool interpolation_enabled(const void* variable) { //43407DE2
    while (field<const void*>(variable, 0x1c) != variable)
        variable = field<const void*>(variable, 0x1c);
    const auto encoded = field<std::uint32_t>(variable, 0x2c);
    const auto bits = encoded ^ std::uint32_t(reinterpret_cast<std::uintptr_t>(variable));
    return std::bit_cast<float>(bits) != 0.0f;
}
}
void draw_line_of_sight(const LineOfSightContext& c) {
    const auto& b = c.bindings;
    if (!c.config.line_of_sight.value || !engine::invoke<bool>(b.engine, 26) || c.suppressed ||
        !c.frame.activation || engine::invoke<bool>(b.engine, 82) ||
        engine::invoke<bool>(b.engine, 93) || !interpolation_enabled(b.interpolation_cvar))
        return;
    auto& players = c.players;
    auto* local = players.local();
    if (!local)
        return;
    const auto maximum = *b.maximum_players;
    for (std::int32_t index = 1; index <= maximum; ++index) {
        auto* player = players.by_index(index);
        if (!player || !players.is_player(*player) || players.life(*player) != 0 ||
            players.health(*player) <= 0 || players.dormant(*player) ||
            players.suppressed(*player) || players.eligibility(*player) ||
            c.adjustments.records[std::uint32_t(index)].adjustments.disable_visuals)
            continue;
        if (!c.config.teammates.value && !live_enemy(local, player, players))
            continue;
        auto start = players.origin(*player);
        start.z = grenade_player_view_height(field<float>(player->engine_entity, b.duck_amount)) +
                  start.z;
        const auto angles = field<Vector3>(player->engine_entity, b.eye_angles);
        constexpr float radians = 0.01745329238474369f;
        const auto yaw = angles.y * radians;
        const auto sy = b.trigonometry.sine(yaw);
        const auto cy = b.trigonometry.cosine(yaw);
        const auto pitch = angles.x * radians;
        const auto sp = b.trigonometry.sine(pitch);
        const auto cp = b.trigonometry.cosine(pitch);
        const auto x = (cp * cy) * 2048.0f + start.x;
        const auto y = (cp * sy) * 2048.0f + start.y;
        const auto z = start.z - sp * 2048.0f;
        const MovementVector delta{x - start.x, y - start.y, z - start.z};
        const bool swept = (delta.y * delta.y + delta.x * delta.x) + delta.z * delta.z != 0.0f;
        const MovementCollisionRay ray{
            {start.x, start.y, start.z}, delta, {0, 0, 0}, {0, 0, 0}, true, swept};
        MovementCollisionTrace hit{};
        trace_movement_world(b.collision, ray, 1, hit, b.trace_flag);
        if (b.debug_overlay) {
            const auto color = c.config.sight_color.value;
            engine::invoke<void>(b.debug_overlay, 20, &start, &hit.end, std::int32_t(color & 255u),
                                 std::int32_t((color >> 8) & 255u),
                                 std::int32_t((color >> 16) & 255u), std::int32_t(color >> 24),
                                 std::int32_t(0), 0.0f);
        }
    }
}
LineOfSightHandler::LineOfSightHandler(engine::OwnerRegistryBinding registry, SoundOverlay& sounds,
                                       EspFrameEvents& events,
                                       events::EventRegistry& event_registry,
                                       const WorldEspConfig& world,
                                       const LineOfSightContext& context)
    : sounds_(sounds), events_(events), event_registry_(event_registry), world_(world),
      context_(context) {
    engine::register_frame_update(registry, this);
    vtable = entries_;
}
#if defined(_M_IX86) || defined(__i386__)
void __fastcall LineOfSightHandler::update(void* object, std::uint8_t refresh) {
#else
void LineOfSightHandler::update(void* object, std::uint8_t refresh) {
#endif
    auto& self =
        *static_cast<LineOfSightHandler*>(static_cast<engine::FrameUpdateEndpoint*>(object));
    self.sounds_.synchronize_listeners(refresh != 0, self.context_.config, self.world_,
                                       self.events_.dormant_listeners(), self.event_registry_);
    draw_line_of_sight(self.context_);
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: map_entity_parser.cpp
// ============================================================================

namespace skeet::esp {
MapEntityParserEndpoint* register_map_entity_parser(engine::OwnerRegistryBinding registry,
                                                    MapEntityParserEndpoint* endpoint) {
    registry.append(endpoint);
    return endpoint;
}
void dispatch_map_scalar(const engine::OwnerRegistry& registry, float value) {
    const auto callbacks = registry.entries();
    for (auto* object : callbacks) {
        auto* endpoint = static_cast<MapEntityParserEndpoint*>(object);
        endpoint->vtable->scalar(object, value);
    }
}
void dispatch_map_entity_text(const engine::OwnerRegistry& registry, void* engine) {
    const auto* value = engine::invoke<const char*>(engine, 99);
    if (!value)
        return;
    const auto callbacks = registry.entries();
    for (auto* object : callbacks) {
        auto* endpoint = static_cast<MapEntityParserEndpoint*>(object);
        endpoint->vtable->text(object, value);
    }
}
EspMapEntityParser::EspMapEntityParser(engine::OwnerRegistryBinding registry, EspFrameOwner& owner,
                                       const std::string& prefix)
    : owner_(owner), prefix_(prefix) {
    register_map_entity_parser(registry, this);
    vtable = &entries_;
}
#if defined(_M_IX86) || defined(__i386__)
void __vectorcall EspMapEntityParser::scalar(void*, float) {} //433CCE8E
void __fastcall EspMapEntityParser::text(void* object, const char* value) {
#else
void EspMapEntityParser::scalar(void*, float) {}
void EspMapEntityParser::text(void* object, const char* value) {
#endif
    auto& self = *static_cast<EspMapEntityParser*>(static_cast<MapEntityParserEndpoint*>(object));
    const auto* found = std::strstr(value, self.prefix_.c_str());
    if (!found)
        return;
    const auto parsed = std::uint32_t(std::strtoul(found + 14, nullptr, 10));
    if (!parsed || parsed == 0xffffffffu)
        return;
    const auto signed_value = std::bit_cast<std::int32_t>(parsed);
    self.owner_.bomb_peak_damage =
        float(double(signed_value) + (parsed >> 31 ? 4294967296.0 : 0.0));
}
} //namespace skeet::esp

// ============================================================================
// Subcomponent: model_color_config.cpp
// ============================================================================

namespace skeet::esp {
namespace {
void group(ConfigLink& registry, ModelColorGroup& c, std::uint32_t enabled_hash,
           std::uint32_t primary_hash, std::uint32_t primary, std::uint32_t material_hash,
           std::int32_t material, std::uint32_t secondary_hash, std::uint32_t secondary) noexcept {
    initialize_boolean_config(registry, c.enabled, enabled_hash);
    initialize_color_config(registry, c.primary_color, primary_hash, primary);
    initialize_integer_config(registry, c.material, material_hash, material);
    initialize_color_config(registry, c.secondary_color, secondary_hash, secondary);
}
void occluded_group(ConfigLink& registry, OccludedModelColorGroup& c, std::uint32_t enabled_hash,
                    std::uint32_t primary_hash, std::uint32_t primary, std::uint32_t behind_hash,
                    std::uint32_t behind_color_hash, std::uint32_t behind_color,
                    std::uint32_t material_hash, std::int32_t material,
                    std::uint32_t secondary_hash, std::uint32_t secondary) noexcept {
    initialize_boolean_config(registry, c.enabled, enabled_hash);
    initialize_color_config(registry, c.primary_color, primary_hash, primary);
    initialize_boolean_config(registry, c.behind_wall, behind_hash);
    initialize_color_config(registry, c.behind_wall_color, behind_color_hash, behind_color);
    initialize_integer_config(registry, c.material, material_hash, material);
    initialize_color_config(registry, c.secondary_color, secondary_hash, secondary);
}
}

void initialize_colored_models_settings(ConfigRegistries& registries,
                                        ColoredModelsSettings& s) noexcept {
    auto& r = registries.descriptors;
    initialize_boolean_config(r, s.props, 0xE217C264u);
    initialize_color_config(r, s.props_color, 0x84830790u, 0xFFFFFFFFu);
    occluded_group(r, s.player, 0xC51E0D3Du, 0xFE06341Du, 0xFF3CC896u, 0xF6B67DE5u, 0xD4699245u,
                   0xB2B4783Cu, 0x044F7095u, 0, 0x9FF240E4u, 0xFF00B2FFu);
    occluded_group(r, s.teammate, 0x1570AF7Cu, 0x2CEFC91Cu, 0xFF323232u, 0x57BF65ECu, 0x04AD8938u,
                   0xB2FFFFFFu, 0xE85380FAu, 4, 0xF3D8F0D1u, 0xFF3B2D09u);
    group(r, s.local_player, 0x6670DB4Du, 0x9201E7EDu, 0xFF323232u, 0x3B28B0E5u, 4, 0x9436ECB4u,
          0xFF3B2D09u);
    group(r, s.local_fake, 0x96DB2E1Cu, 0xA98CEAE8u, 0x78000000u, 0x3FC208C1u, 1, 0xF50A4300u,
          0xFF00B2FFu);
    initialize_integer_config(r, s.local_transparency, 0x3EA36C3Au, 0);
    group(r, s.on_shot, 0x5E0E2558u, 0xC2DBF0A4u, 0xFF323232u, 0x17C5B91Au, 5, 0x8B122392u,
          0xFFFFFFFFu);
    initialize_integer_config(r, s.on_shot_duration, 0x1A07B48Fu, 20);
    group(r, s.hands, 0xE8407B64u, 0x5A93F490u, 0xFFFFFFFFu, 0xB7360BA6u, 5, 0x2926F5DEu,
          0xFFA49E54u);
    group(r, s.weapon_viewmodel, 0xDA411D58u, 0x75A868A4u, 0xFF6E1032u, 0x2A1FA11Au, 3, 0x1C678B92u,
          0xFFA49E54u);
    group(r, s.weapons, 0x92EBAD93u, 0x83BE3FA7u, 0xFF6E1032u, 0x118A8443u, 3, 0x6D66C717u,
          0xFFA49E54u);
    initialize_boolean_config(r, s.shadow, 0x67F8885Cu);
    initialize_color_config(r, s.shadow_color, 0xC2140C28u, 0x78000000u);
    initialize_boolean_config(r, s.ragdolls, 0x8CCAF8DCu); // static 0x4346A680
    initialize_boolean_config(r, s.disable_model_occlusion, 0x8E0781AEu); // static 0x43467ADC
}

} // namespace skeet::esp

// ============================================================================
// Subcomponent: offscreen.cpp
// ============================================================================

namespace skeet::esp {
using namespace detail;

void draw_out_of_fov_arrow(ui::RenderQueue& q, Point screen, i32 size_value, i32 distance_value,
                           std::uint32_t color, float yaw, float time) {
    if (std::isnan(yaw))
        yaw = 0.0f;
    else {
        if (std::fabs(yaw) > 180.0f) {
            const auto turns = round_half_away_from_zero(std::fabs(yaw / 360.0f)) * 360.0f;
            yaw = yaw < 0.0f ? yaw + turns : yaw - turns;
        }
        if (yaw > 0.0f)
            yaw = sse_max(90.0f, yaw);
        else if (yaw < 0.0f)
            yaw = sse_min(-90.0f, yaw);
    }
    const float angle = yaw * std::bit_cast<float>(0x3C8EFA35u);
    // imports.h callsite evidence: 0x7750C6D0 is sinf; 0x7750A770 is cosf.
    const float cosine = std::cos(angle), negative_sine = -std::sin(angle);
    const float cx = float(screen.x) * 0.5f, cy = float(screen.y) * 0.5f;
    const float size = float(size_value);
    const float radius = (float(distance_value) * 0.01f) * (cy - size);
    const float xr = (cx / cy) * radius;
    const float half_size = size * 0.5f;
    const auto x0 = truncate((negative_sine * xr + cx) + cosine * half_size);
    const auto y0 = truncate((cy - cosine * radius) + negative_sine * half_size);
    const auto x1 = truncate(float(x0) - cosine * size);
    const auto y1 = truncate(float(y0) - negative_sine * size);
    const auto x2 = truncate((xr + size) * negative_sine + cx);
    const auto y2 = truncate(cy - (radius + size) * cosine);
    const float pulse = std::cos(time * 5.0f) * 117.0f + 137.0f;
    const float alpha = float(color >> 24) * std::bit_cast<float>(0x3B808081u);
    color = (color & 0xFFFFFFu) | ((std::uint32_t(truncate(pulse * alpha)) & 255) << 24);
    q.triangle({x0, y0, x1, y1, x2, y2, color});
}

void draw_offscreen_indicator(ui::RenderQueue& q, const OverlayConfig& c, const OffscreenInput& p,
                              LocalPlayerPredicate predicate) {
    if (!c.out_of_fov.value || !predicate.evaluate(predicate.context))
        return;
    draw_selected_offscreen_indicator(q, c, p);
}
void draw_selected_offscreen_indicator(ui::RenderQueue& q, const OverlayConfig& c,
                                       const OffscreenInput& p) {
    const float aspect = (float(p.screen.x) / float(p.screen.y)) * 0.75f;
    const float half_fov = convert_field_of_view(p.camera_fov, aspect) * 0.5f;
    const float yaw_radians = p.yaw * std::bit_cast<float>(0x3C8EFA35u);
    const float pitch_radians = p.pitch * std::bit_cast<float>(0x3C8EFA35u);
    const float pitch_cosine = std::cos(pitch_radians);
    const Vector3 axis{pitch_cosine * std::cos(yaw_radians), pitch_cosine * std::sin(yaw_radians),
                       -std::sin(pitch_radians)};
    if (half_fov >= angle_to_target(p.origin, p.target, axis))
        return;
    const float dx = p.target.x - p.origin.x, dy = p.target.y - p.origin.y;
    float target_yaw = 0.0f;
    if (dx != 0.0f || dy != 0.0f)
        target_yaw = float(std::atan2(double(dy), double(dx))) * std::bit_cast<float>(0x42652EE0u);
    draw_out_of_fov_arrow(q, p.screen, c.arrow_size.value, c.arrow_distance.value,
                          c.arrow_color.value, target_yaw - p.yaw, p.animation_time);
}

} // namespace skeet::esp

// ============================================================================
// Subcomponent: other_config.cpp
// ============================================================================

namespace skeet::esp {
namespace {
void colored(ConfigLink& registry, ColoredWorldItem& item, std::uint32_t enabled_hash,
             std::uint32_t color_hash, std::uint32_t color) noexcept {
    initialize_boolean_config(registry, item.enabled, enabled_hash);
    initialize_color_config(registry, item.color, color_hash, color);
}
}

// Selected descriptors of the native 0x43482632 owner, in constructor order.
void initialize_world_esp_config(ConfigLink& registry, WorldEspConfig& c) noexcept {
    initialize_boolean_config(registry, c.radar, 0xD7CD1B0Bu);
    initialize_boolean_config(registry, c.spectators, 0x38623779u);
    initialize_integer_config(registry, c.dropped_weapons, 0x6D3D6833u, 0);
    initialize_color_config(registry, c.dropped_weapon_color, 0xA10DC407u, 0xC8FFFFFFu);
    colored(registry, c.hostages, 0x559F78DCu, 0xA6B878A8u, 0xC8FC9F38u);
    colored(registry, c.bomb, 0xF2937E07u, 0x2ADA481Bu, 0xFF3CC896u);
    initialize_boolean_config(registry, c.upgrade_tablet, 0x553D3C3Fu);
    initialize_boolean_config(registry, c.danger_zone_items, 0x89653491u);
    colored(registry, c.drone_gun, 0x75A591C2u, 0xDB9A269Au, 0xFF3CC896u);
    colored(registry, c.blackhawk, 0x325AF11Eu, 0x174ABFD6u, 0xFF00E0FFu);
    colored(registry, c.drone, 0x6E6CDAD7u, 0xF407872Bu, 0xFF3CC896u);
    colored(registry, c.random_case, 0x5DA3152Cu, 0x082CE178u, 0xFFF0138Fu);
    colored(registry, c.tool_case, 0xF2A389D2u, 0x928C112Au, 0xFFFFDC00u);
    colored(registry, c.pistol_case, 0x3B6A149Eu, 0xCEE5C756u, 0xFF505050u);
    colored(registry, c.explosive_case, 0xA3B423BEu, 0x55436936u, 0xFF0082FFu);
    colored(registry, c.heavy_weapon_case, 0x52DAAA5Du, 0x72894F7Du, 0xFF0000FFu);
    colored(registry, c.light_weapon_case, 0x081D65D4u, 0xCF605580u, 0xFF3CC896u);
    colored(registry, c.dufflebag, 0xD13FFDF6u, 0x8B5C32AEu, 0xFF3B7FC8u);
    colored(registry, c.jammer, 0x6A586832u, 0x76DA764Au, 0xFF00006Eu);
    colored(registry, c.ammobox, 0xF8774353u, 0x406A4767u, 0xFF3B7FC8u);
    colored(registry, c.armor, 0xFFD5C89Du, 0xC11B6FBDu, 0xFFC8843Bu);
    colored(registry, c.parachute_pack, 0x2802A1F6u, 0xB424B6AEu, 0xFFFFDC00u);
    colored(registry, c.briefcase, 0x2B4CDE08u, 0xC8555DD4u, 0xFF3B7FC8u);
    colored(registry, c.tablet_upgrade_zone, 0x4FD3F678u, 0x58386184u, 0xFFFFF0AAu);
    colored(registry, c.tablet_upgrade_drone, 0x42694ACEu, 0x06D6A3C6u, 0xFFFFF0AAu);
    colored(registry, c.cash_stack, 0x2D915386u, 0xE5F28DFEu, 0xFF00FF00u);
}
void initialize_screen_overlay_config(ConfigLink& registry, ScreenOverlayConfig& c) noexcept {
    // 0x4348DC0B..49, 0x43482503..16, and static descriptor 0x4346A754.
    initialize_boolean_config(registry, c.recoil, 0x78A1B150u);
    initialize_boolean_config(registry, c.inaccuracy, 0xEDF6C582u);
    initialize_color_config(registry, c.inaccuracy_color, 0x2B61D25Au, 0x3C1E1E1Eu);
    initialize_boolean_config(registry, c.penetration_reticle, 0x1E7432A5u);
    initialize_boolean_config(registry, c.crosshair, 0xAD37A2EBu);
}
void initialize_grenade_esp_config(ConfigLink& registry, GrenadeEspConfig& c) noexcept {
    // Static descriptors 0x43467CC4/0x43468F50 and 0x4348682C..8B.
    initialize_boolean_config(registry, c.entities, 0x25B4B670u);
    initialize_color_config(registry, c.entity_color, 0xB8C2C5BCu, 0);
    initialize_boolean_config(registry, c.trajectory, 0xDC4C1BB9u);
    initialize_color_config(registry, c.trajectory_color, 0x8428F8F9u, 0xB33232FFu);
    initialize_color_config(registry, c.trajectory_secondary_color, 0xC2611C1Fu, 0xFF3BC896u);
    initialize_boolean_config(registry, c.proximity_warning, 0x85BDA341u);
}
void initialize_shared_esp_config(ConfigRegistries& registry, SharedEspConfig& c) noexcept {
    // 0x43485E26..65; static descriptors 0x43467F88/0x43468588.
    initialize_hotkey_config(registry, c.activation, 0x7174D684u, 0x1B, HotkeyMode::always);
    initialize_boolean_config(registry.descriptors, c.other_team, 0x6834D5DAu);
    initialize_boolean_config(registry.descriptors, c.enabled, 0x0E533025u);
    initialize_boolean_config(registry.descriptors, c.restrict_updates, 0xCBE162F8u);
}
void initialize_other_esp_settings(ConfigRegistries& registry,
                                   OtherEspSettings& settings) noexcept {
    initialize_world_esp_config(registry.descriptors, settings.world);
    initialize_screen_overlay_config(registry.descriptors, settings.overlays);
    initialize_grenade_esp_config(registry.descriptors, settings.grenades);
    initialize_shared_esp_config(registry, settings.shared);
    initialize_integer_config(registry.descriptors, settings.feature_indicators, 0x59A226C2u, 0);
}

} // namespace skeet::esp

// ============================================================================
// Subcomponent: owner_runtime.cpp
// ============================================================================

namespace skeet::esp {
EspOwnerRuntime::EspOwnerRuntime(EspOwnerBindings bindings, EspOwnerRuntime*& active)
    : bindings_(bindings), active_(&active), players_(bindings.player_engine),
      world_(bindings.world_engine), frame_(bindings.frame, players_, world_),
      sound_({bindings_.player, bindings_.sound.bindings, bindings_.frame.player_adjustments,
              bindings_.state.frame},
             players_, bindings_.state.players, bindings_.state.pickups),
      line_of_sight_(bindings_.line_of_sight, players_),
      events_(bindings.state, bindings.player, players_.runtime_engine(), bindings.event_frame),
      death_{bindings.death, bindings.state.players} {
    auto& r = bindings_.registries;
    auto& owner = bindings_.state;
    r.owners.append(this);
    hud_.emplace(r.hud, owner, frame_);
    histories_.emplace(r.histories, owner.pickups, bindings_.engine_globals,
                       bindings_.tick_interval);
    updates_.emplace(r.updates, owner.tablet, bindings_.tablet, bindings_.glow);
    packets_.emplace(r.packets, tracers_, owner.sounds, sound_, bindings_.effects,
                     bindings_.packet);
    sight_.emplace(r.frame_update, owner.sounds, events_, r.events, bindings_.world,
                   line_of_sight_);
    parser_.emplace(r.map_parser, owner, radius_prefix_);
    vtable = entries_; //43482698
    r.histories.publish(*histories_); //434826A8, after all seven registrations
    initialize_configuration();
    initialize_state();
    const auto templates = danger_zone_item_templates();
    owner.entity_models.bind_config(bindings_.world);
    for (std::size_t index = 0; index < item_labels_.size(); ++index) {
        item_labels_[index] = templates[index].label;
        auto& descriptor = owner.entity_models.descriptors[index];
        descriptor.label = item_labels_[index];
        descriptor.distance = 0;
        descriptor.closest_origin = {};
    }
    labels_ = {u"FRAG", u"FLASH", u"SMOKE", u"DECOY", u"MOLLY", u"NADE", u"BOMB"};
    owner.grenade_labels = {labels_[0], labels_[1], labels_[2], labels_[3], labels_[4], labels_[5]};
    owner.dropped_bomb_label = labels_[6];
    radius_prefix_ = "\"bombradius\" \"";
    events_.bind_before_death(r.events);
    death_.bind(r.events);
    events_.bind_after_death(r.events);
    initialize_classes();
}
void EspOwnerRuntime::initialize_configuration() {
    auto& registries = bindings_.registries.configuration;
    auto& r = registries.descriptors;
    auto& p = bindings_.player;
    auto& w = bindings_.world;
    auto boolean = [&](BooleanConfig& value, std::uint32_t hash) {
        initialize_boolean_config(r, value, hash);
    };
    auto color = [&](ColorConfig& value, std::uint32_t hash, std::uint32_t initial) {
        initialize_color_config(r, value, hash, initial);
    };
    auto integer = [&](IntegerConfig& value, std::uint32_t hash, std::int32_t initial) {
        initialize_integer_config(r, value, hash, initial);
    };
    boolean(bindings_.effects.bullet_tracers, 0x3A5285A6u);
    color(bindings_.effects.tracer_color, 0x635F47DEu, 0xFFFF8296u);
    bindings_.state.sounds.ring_model = -1;
    tracers_.beams = bindings_.beams; //434826E5..434826EA, before activation registration
    tracers_.first = 0;
    tracers_.count = 0;
    initialize_hotkey_config(registries, p.activation, 0x4700A298u, 0x10, HotkeyMode::always);
    boolean(p.bounding_box, 0xCFF9EBADu);
    color(p.box_color, 0x4D243E0Du, 0x82FFFFFFu);
    boolean(p.health_bar, 0x49733D3Cu);
    color(p.health_color, 0xD2E84B64u, 0xFF50E178u);
    boolean(p.name, 0xA1C152A1u);
    color(p.name_color, 0x25FEB6C1u, 0xC8FFFFFFu);
    boolean(p.weapon_text, 0xBB3FDF6Au);
    boolean(p.weapon_icon, 0x6A874B2Cu);
    color(p.weapon_icon_color, 0x00F64778u, 0xC8FFFFFFu);
    boolean(p.distance, 0x0F6D81D5u);
    boolean(p.glow, 0xB1D008B7u);
    color(p.glow_color, 0xECE872CBu, 0x99783CB4u);
    integer(field_8ce3b4b2_, 0x8CE3B4B2u, 0);
    boolean(p.visualize_sounds, 0xE8D2A207u);
    color(p.sound_color, 0x7E3B4C1Bu, 0xFF3F3FFAu);
    boolean(p.flags, 0x084A8B33u);
    boolean(p.ammo, 0x43F02F68u);
    color(p.ammo_color, 0x8BEC37F4u, 0xE6C88C50u);
    boolean(p.line_of_sight, 0x1DE914E6u);
    color(p.sight_color, 0x745EFA1Eu, 0xFF0000FFu);
    boolean(p.skeleton, 0x523E4B35u);
    color(p.skeleton_color, 0x0E995F95u, 0xFFAAFFFFu);
    boolean(p.teammates, 0x45D9D2ACu);
    boolean(w.radar, 0xD7CD1B0Bu);
    boolean(w.spectators, 0x38623779u);
    integer(w.dropped_weapons, 0x6D3D6833u, 0);
    color(w.dropped_weapon_color, 0xA10DC407u, 0xC8FFFFFFu);
    boolean(p.out_of_fov, 0x95A660ABu);
    integer(p.arrow_size, 0x4564BFC7u, 12);
    integer(p.arrow_distance, 0x09CBEDAEu, 100);
    color(p.arrow_color, 0x7D18A71Fu, 0xFFFFFFFFu);
    boolean(w.hostages.enabled, 0x559F78DCu);
    color(w.hostages.color, 0xA6B878A8u, 0xC8FC9F38u);
    boolean(w.bomb.enabled, 0xF2937E07u);
    color(w.bomb.color, 0x2ADA481Bu, 0xFF3CC896u);
    boolean(p.dormant, 0xC410C5D3u);
    boolean(w.upgrade_tablet, 0x553D3C3Fu);
    boolean(w.danger_zone_items, 0x89653491u);
    struct Item {
        ColoredWorldItem WorldEspConfig::* member;
        std::uint32_t enabled, color, value;
    };
    constexpr Item items[]{
        {&WorldEspConfig::drone_gun, 0x75A591C2u, 0xDB9A269Au, 0xFF3CC896u},
        {&WorldEspConfig::blackhawk, 0x325AF11Eu, 0x174ABFD6u, 0xFF00E0FFu},
        {&WorldEspConfig::drone, 0x6E6CDAD7u, 0xF407872Bu, 0xFF3CC896u},
        {&WorldEspConfig::random_case, 0x5DA3152Cu, 0x082CE178u, 0xFFF0138Fu},
        {&WorldEspConfig::tool_case, 0xF2A389D2u, 0x928C112Au, 0xFFFFDC00u},
        {&WorldEspConfig::pistol_case, 0x3B6A149Eu, 0xCEE5C756u, 0xFF505050u},
        {&WorldEspConfig::explosive_case, 0xA3B423BEu, 0x55436936u, 0xFF0082FFu},
        {&WorldEspConfig::heavy_weapon_case, 0x52DAAA5Du, 0x72894F7Du, 0xFF0000FFu},
        {&WorldEspConfig::light_weapon_case, 0x081D65D4u, 0xCF605580u, 0xFF3CC896u},
        {&WorldEspConfig::dufflebag, 0xD13FFDF6u, 0x8B5C32AEu, 0xFF3B7FC8u},
        {&WorldEspConfig::jammer, 0x6A586832u, 0x76DA764Au, 0xFF00006Eu},
        {&WorldEspConfig::ammobox, 0xF8774353u, 0x406A4767u, 0xFF3B7FC8u},
        {&WorldEspConfig::armor, 0xFFD5C89Du, 0xC11B6FBDu, 0xFFC8843Bu},
        {&WorldEspConfig::parachute_pack, 0x2802A1F6u, 0xB424B6AEu, 0xFFFFDC00u},
        {&WorldEspConfig::briefcase, 0x2B4CDE08u, 0xC8555DD4u, 0xFF3B7FC8u},
        {&WorldEspConfig::tablet_upgrade_zone, 0x4FD3F678u, 0x58386184u, 0xFFFFF0AAu},
        {&WorldEspConfig::tablet_upgrade_drone, 0x42694ACEu, 0x06D6A3C6u, 0xFFFFF0AAu},
        {&WorldEspConfig::cash_stack, 0x2D915386u, 0xE5F28DFEu, 0xFF00FF00u}};
    for (const auto& item : items) {
        if (item.member == &WorldEspConfig::parachute_pack) {
            boolean(field_5aadd6ed_.enabled, 0x5AADD6EDu);
            color(field_5aadd6ed_.color, 0xEC79024Du, 0xFF3CC896u);
            boolean(field_b1054809_.enabled, 0xB1054809u);
            color(field_b1054809_.color, 0xA7B7F909u, 0xFF3CC896u);
        }
        auto& setting = w.*item.member;
        boolean(setting.enabled, item.enabled);
        color(setting.color, item.color, item.value);
    }
}
void EspOwnerRuntime::initialize_state() {
    auto& owner = bindings_.state;
    owner.sounds.listener_enabled = {};
    tracers_.recent = {};
    owner.defuse_kit_model = -1;
    owner.frame.transient = 0;
    owner.frame.timer = 0;
    owner.bomb_peak_damage = 500.0f;
    owner.state_6b4 = 0;
    owner.grenade_classes = {};
    owner.notice_entity = 0; //calloc value; reset later changes this to-1
    bindings_.glow.weapons_active = false;
    bindings_.glow.bomb_handle = -1;
    owner.tablet = TabletUpgradeState{};
    owner.tablet.frame = 0; //constructor calloc, distinct from reset
    owner.pickups = PickupObservations{};
}
void EspOwnerRuntime::initialize_classes() {
    auto& owner = bindings_.state;
    auto* current = engine::invoke<EspClientClass*>(bindings_.client_classes, 10);
    for (std::uint32_t index = 0; current; current = current->next, ++index) {
        const auto hash =
            entity_class_hash(current->name ? std::string_view(current->name) : std::string_view{});
        const auto handler = entity_overlay_handler(hash);
        if (handler == EntityOverlayHandler::none)
            continue;
        if (class_handlers_.size() <= index)
            class_handlers_.resize(std::size_t(index) + 1);
        class_handlers_[index] = handler;
        switch (hash) {
        case 0xC09C6702u:
            owner.grenade_classes.base_projectile = std::int32_t(index);
            break;
        case 0x69629121u:
            owner.grenade_classes.decoy_projectile = std::int32_t(index);
            break;
        case 0xA239EA8Fu:
            owner.grenade_classes.molotov_projectile = std::int32_t(index);
            break;
        case 0xE31DC1B8u:
            owner.grenade_classes.smoke_projectile = std::int32_t(index);
            break;
        case 0xABE104C9u:
            owner.grenade_classes.inferno = std::int32_t(index);
            break;
        default:
            break;
        }
    }
    owner.class_handlers = class_handlers_;
}
EspOwnerRuntime::~EspOwnerRuntime() { //4338580A exact owned-field teardown order
    std::string{}.swap(radius_prefix_);
    for (auto index = labels_.size(); index; --index)
        std::u16string{}.swap(labels_[index - 1]);
    const auto listeners = events_.dormant_listeners();
    auto& owner = bindings_.state;
    for (auto index = listeners.size(); index; --index) {
        auto& enabled = owner.sounds.listener_enabled[index - 1];
        if (enabled) {
            enabled = 0;
            bindings_.registries.events.remove(listeners[index - 1]);
        }
    }
    for (auto index = item_labels_.size(); index; --index)
        std::u16string{}.swap(item_labels_[index - 1]);
    owner.entity_models.models.release_storage();
    std::vector<EntityOverlayHandler>{}.swap(class_handlers_);
}
void EspOwnerRuntime::bind_configuration() { //43485C12
    auto* value = find_config(bindings_.registries.configuration.descriptors, 0xFC225A4Fu);
    override_fov_ = &static_cast<IntegerConfig*>(value)->value;
}
void EspOwnerRuntime::reset() {
    reset_esp_frame_owner(bindings_.state, bindings_.glow);
}
void EspOwnerRuntime::initialize_models() {
    initialize_esp_entity_models(bindings_.state, bindings_.glow, bindings_.world,
                                 bindings_.network_string_tables);
}
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_ESP_PRIMARY __thiscall
#else
#define SKEET_ESP_PRIMARY
#endif
void SKEET_ESP_PRIMARY EspOwnerRuntime::destroy(void* object) { //43331A5C
    auto* self = static_cast<EspOwnerRuntime*>(object);
    destroy_esp_owner(*self->active_, self);
}
void SKEET_ESP_PRIMARY EspOwnerRuntime::configuration(void* object) {
    static_cast<EspOwnerRuntime*>(object)->bind_configuration();
}
void SKEET_ESP_PRIMARY EspOwnerRuntime::reset_entry(void* object) {
    static_cast<EspOwnerRuntime*>(object)->reset();
}
void SKEET_ESP_PRIMARY EspOwnerRuntime::models(void* object) {
    static_cast<EspOwnerRuntime*>(object)->initialize_models();
}
void SKEET_ESP_PRIMARY EspOwnerRuntime::unchanged(void*) {} //433CCE8E
#undef SKEET_ESP_PRIMARY
void initialize_esp_owner(EspOwnerRuntime*& active, EspOwnerBindings bindings) { //43481EAB
    constexpr auto bytes = sizeof(EspOwnerRuntime) > 0xF90 ? sizeof(EspOwnerRuntime) : 0xF90;
    auto* storage = engine::allocate_owner_storage(bindings.registries.owners.memory, bytes);
    if (!storage) {
        active = nullptr;
        return;
    }
    active = ::new (storage) EspOwnerRuntime(bindings, active);
}
void destroy_esp_owner(EspOwnerRuntime*& active, EspOwnerRuntime* object) { //43331A5C
    active = nullptr;
    if (!object)
        return;
    const auto memory = object->bindings_.registries.owners.memory;
    object->~EspOwnerRuntime();
    engine::release_owner_storage(memory, object);
}
} //namespace skeet::esp

// ============================================================================
// Subcomponent: packet_tracers.cpp
// ============================================================================

namespace skeet::esp {
namespace {
/* [removed duplicate field] */
/* [removed duplicate field] */
bool shotgun_patterns(const void* variable) {
    while (field<const void*>(variable, 0x1c) != variable)
        variable = field<const void*>(variable, 0x1c);
    return (field<std::uint32_t>(variable, 0x30) ^
            std::uint32_t(reinterpret_cast<std::uintptr_t>(variable))) != 0;
}
BallisticQuery packet_bullet(void* weapon_system, std::uint16_t definition) {
    client::CommandBullet bullet{};
    rage::capture_shot_weapon_bullet(weapon_system, definition, bullet);
    // Complete433D0015 construction, copying the seven433CCA5C words.
    BallisticQuery result;
    result.allow_penetration = 1;
    reset_ballistic_query(result);
    result.weapon_id = bullet.weapon_id;
    result.weapon_padding = bullet.padding;
    result.penetration = bullet.penetration;
    result.range = bullet.range;
    result.range_modifier = bullet.range_modifier;
    result.armor_ratio = bullet.armor_ratio;
    result.base_damage = bullet.damage;
    result.reserved = bullet.pellet_count;
    return result;
}
}

void process_esp_packet(const void* packet, PacketTracerState& state, SoundOverlay& sounds,
                        const SoundOverlayContext& sound, const EffectsSettings& settings,
                        const PacketTracerBindings& bindings) {
    update_dormant_radar(packet, bindings.offsets.radar, sound);
    if (!settings.bullet_tracers.value || !state.beams || sound.players.suspended() ||
        sounds.ring_model == -1)
        return;
    const auto& offsets = bindings.offsets;
    // The packet master supplies a valid decoded object. Its field is zero-based.
    const auto index = std::bit_cast<std::int32_t>(
        field<std::uint32_t>(packet, offsets.radar.zero_based_index) + 1u);
    auto* shooter = sound.players.by_index(index);
    if (!shooter)
        return;
    auto* local = sound.players.local();
    if (local == shooter || (local && !dormant_enemy(local, shooter, sound.players)))
        return;

    const auto origin = field<Vector3>(packet, offsets.radar.origin);
    const auto recoil = field<float>(packet, offsets.recoil_index);
    const auto seed = field<std::uint32_t>(packet, offsets.seed);
    const auto mode = field<std::int32_t>(packet, offsets.fire_mode);
    const auto definition = field<std::uint16_t>(packet, offsets.weapon_id);
    const auto inaccuracy = field<float>(packet, offsets.inaccuracy);
    const auto spread = field<float>(packet, offsets.spread);
    const rage::SpreadParameters parameters{definition,
                                            mode,
                                            spread,
                                            inaccuracy,
                                            recoil,
                                            1,
                                            shotgun_patterns(bindings.shotgun_patterns_convar)};
    const auto sample =
        rage::generate_spread_sample(parameters, seed, bindings.spread_tables, bindings.random,
                                     bindings.shotgun_pattern, bindings.trigonometry);
    auto angles = field<Vector3>(packet, offsets.angles);
    angles.z = 0.0f;
    const auto basis = legit::spread_basis({angles.x, angles.y, angles.z}, bindings.trigonometry);
    RageVector direction;
    for (std::size_t component = 0; component < direction.size(); ++component)
        direction[component] = (basis.first[component] * sample.first + basis.center[component]) +
                               basis.second[component] * sample.second;
    const auto squared =
        (direction[0] * direction[0] + direction[1] * direction[1]) + direction[2] * direction[2];
    const auto inverse = 1.0f / (std::sqrt(squared) + 0x1p-23f);
    for (auto& component : direction)
        component *= inverse;

    auto bullet = packet_bullet(bindings.weapon_system, definition);
    anti_aim::TargetTraceFilter native_filter(bindings.trace_filter_vtable, bindings.trace_entities,
                                              0x600400bu, 0x600400bu, 1.0f);
    const auto filter = native_filter.binding();
    rage::trace_target_direction(
        bindings.ballistics,
        std::uint32_t(reinterpret_cast<std::uintptr_t>(shooter->engine_entity)),
        {origin.x, origin.y, origin.z}, direction, filter, bullet);
    const auto endpoint = bullet.trace.end;
    const auto dx = endpoint[0] - origin.x;
    const auto dy = endpoint[1] - origin.y;
    const auto dz = endpoint[2] - origin.z;
    // COMISS/JA rejects ordered squared distances below1000; unordered passes.
    if (1000.0f > (dy * dy + dx * dx) + dz * dz)
        return;

    const auto color = settings.tracer_color.value;
    if (state.count == 3) {
        const auto replacing = (state.first + state.count - 1u) % 3u;
        const auto* wanted = state.recent[replacing];
        auto* beam = field<void*>(state.beams, 4);
        while (beam && beam != wanted)
            beam = field<void*>(beam, 0x2c);
        if (beam)
            field(beam, 0xc8, *bindings.prediction_time);
    }
    PacketBeamDescription description;
    description.start = origin;
    description.end = {endpoint[0], endpoint[1], endpoint[2]};
    description.model_index = sounds.ring_model;
    description.brightness = float(color >> 24);
    description.red = float(color & 255u);
    description.green = float((color >> 8) & 255u);
    description.blue = float((color >> 16) & 255u);
    auto* beam = engine::invoke<void*>(state.beams, 12, &description);
    if (!beam)
        return;
    field(beam, 0xdc, 2.0f);
    const auto expires = *bindings.prediction_time + 2.0f;
    field(beam, 0x34, field<std::uint32_t>(beam, 0x34) & 0xffffbfffu);
    field(beam, 0xc8, expires);
    if (state.count != 3)
        ++state.count;
    state.first = (state.first + 2u) % 3u;
    state.recent[state.first] = beam;
}

PacketTracerHandler::PacketTracerHandler(engine::OwnerRegistryBinding registry,
                                         PacketTracerState& state, SoundOverlay& sounds,
                                         const SoundOverlayContext& sound,
                                         const EffectsSettings& settings,
                                         const PacketTracerBindings& bindings)
    : state_(state), sounds_(sounds), sound_context_(sound), settings_(settings),
      bindings_(bindings) {
    registry.append(static_cast<EspPacketEndpoint*>(this));
    vtable = entries_;
}
#if defined(_M_IX86) || defined(__i386__)
void __fastcall PacketTracerHandler::update(void* object, const void* packet) {
#else
void PacketTracerHandler::update(void* object, const void* packet) {
#endif
    auto& self = *static_cast<PacketTracerHandler*>(static_cast<EspPacketEndpoint*>(object));
    process_esp_packet(packet, self.state_, self.sounds_, self.sound_context_, self.settings_,
                       self.bindings_);
}

} // namespace skeet::esp

// ============================================================================
// Subcomponent: planted_bomb.cpp
// ============================================================================

namespace skeet::esp {
using namespace detail;
namespace {
float site_distance(Vector3 origin, Vector3 site) noexcept {
    const float x = origin.x - site.x, y = origin.y - site.y, z = origin.z - site.z;
    return (y * y + x * x) + z * z;
}
}

void draw_planted_bomb(ui::RenderQueue& queue, const WorldEspConfig& config,
                       PlantedBombOwner& owner, std::int32_t& cursor, const PlantedBombInput& input,
                       const IndicatorLayout& layout) {
    draw_planted_bomb(queue, config,
                      {reinterpret_cast<std::byte*>(&owner.transient), owner.peak_damage}, cursor,
                      input, layout);
}
void draw_planted_bomb(ui::RenderQueue& queue, const WorldEspConfig& config,
                       PlantedBombBinding owner, std::int32_t& cursor,
                       const PlantedBombInput& input, const IndicatorLayout& layout) {
    std::memset(owner.transient_word, 0, 4);
    if (!config.bomb.enabled.value || !input.bomb.ticking)
        return;
    auto indicator = [&](std::string_view text, std::uint32_t color, i32 texture) {
        if (input.scripts)
            draw_indicator(*input.scripts, queue, cursor, text, color, layout, texture, -1.0f);
        else
            draw_indicator(queue, cursor, text, color, layout, texture, -1.0f);
    };
    const auto& bomb = input.bomb;
    const auto remaining = bomb.detonation_time - input.current_time;
    if (remaining > 0.0f) {
        if (bomb.defuse_end > 0.0f && bomb.defuser_state == 1) {
            const bool finishes_in_time = !(bomb.defuse_end > bomb.detonation_time);
            const auto height = truncate(
                (float(sub(layout.screen_height, 2)) * (bomb.defuse_end - input.current_time)) /
                10.0f);
            queue.rectangle({0, 0, 20, layout.screen_height, 0x73000000u});
            const auto color =
                (finishes_in_time ? 0x7350FF00u : 0x73503200u) | (finishes_in_time ? 0x32u : 0xFFu);
            queue.rectangle({1, sub(sub(layout.screen_height, height), 2), 18, height, color});
        }
        char site = 'A';
        if (input.sites && site_distance(bomb.origin, input.sites->first) >
                               site_distance(bomb.origin, input.sites->second))
            site = 'B';
        char label[48]{};
        const auto length =
            std::snprintf(label, sizeof(label), "%c - %.1fs", site, double(remaining));
        if (length > 0)
            indicator({label, std::size_t(length)}, 0xC8FFFFFFu, input.label_texture);
    }
    if (!(remaining > -1.0f) || input.player.life_state != 0)
        return;
    if (input.player.health(input.player.context) <= 0)
        return;
    const auto damage = truncate(bomb_damage(input.registered_bomb, input.player.damage_target,
                                             owner.peak_damage, input.trace));
    if (damage == 0)
        return;
    if (damage >= input.player.health(input.player.context)) {
        indicator("FATAL", 0xFF3200FFu, -1);
    } else {
        char label[32]{};
        const auto length = std::snprintf(label, sizeof(label), "-%d HP", damage);
        if (length > 0)
            indicator({label, std::size_t(length)}, 0xFF69F3FCu, -1);
    }
}

} // namespace skeet::esp

// ============================================================================
// Subcomponent: player_draw.cpp
// ============================================================================

#pragma STDC FENV_ACCESS ON

namespace skeet::esp {
using namespace detail;
namespace {
/* [removed duplicate field] */
PlayerWeaponMetadata weapon_metadata(const RuntimeWeapon& weapon, const PlayerDrawEngine& e) {
    const auto info = e.weapon_information(e.context, weapon);
    PlayerWeaponMetadata output;
    output.definition = weapon.definition;
    output.penetration = info.penetration;
    output.range = info.range;
    output.range_modifier = info.range_modifier;
    output.armor_ratio = e.weapon_armor_ratio(e.context, weapon);
    output.damage = float(e.weapon_damage(e.context, weapon));
    output.kind = info.kind;
    return output;
}

bool needs_box(const PlayerEspConfig& c) noexcept {
    return c.bounding_box.value || c.health_bar.value || c.name.value || c.weapon_text.value ||
           c.weapon_icon.value || c.distance.value || c.flags.value || c.ammo.value;
}
struct ViewPredicate {
    const PlayerRuntimeEngine* engine;
    const RuntimePlayer* player;
};
bool view_is_player(void* raw) {
    const auto& value = *static_cast<ViewPredicate*>(raw);
    return value.engine->is_player(value.engine->context, *value.player);
}
}

float player_reload_cycle(const RuntimePlayer& player, const PlayerReloadBindings& bindings) {
    const auto* descriptor = static_cast<const std::byte*>(player.engine_entity) + bindings.layers;
    if (field<std::int32_t>(descriptor, 0xc) < 1)
        return 1.0f;
    const auto* layer = field<const std::byte*>(descriptor, 0);
    if (field<float>(layer, 0x58) == 0.0f)
        return 1.0f;
    const auto sequence = field<std::int32_t>(layer, 0x50);
    const auto activity = bindings.sequence_activity
                              ? bindings.sequence_activity(player.engine_entity, sequence)
                              : -1;
    return activity == 0x3c7 ? field<float>(layer, 0x64) : 1.0f;
}

std::int32_t overlay_player_health(const RuntimePlayer* player, const PlayerDrawEngine& e,
                                   const PlayerDrawRules& rules) { // 433C652F
    if (!player)
        return 0;
    if (!e.players.dormant(e.players.context, *player))
        return e.player_health(e.context, *player);
    if (!rules.players.present)
        return 0;
    const auto index = e.players.index(e.players.context, *player);
    return rules.players.players[std::uint32_t(index)].connected
               ? rules.dormant_health[std::uint32_t(index)]
               : 0;
}
PlayerWeaponMetadata player_weapon_metadata(const RuntimePlayer& player,
                                            const PlayerDrawState& state,
                                            const PlayerDrawEngine& e) { // 43328CA6
    if (state.active_weapon_handle != -1) {
        if (auto* weapon = e.weapon_by_handle(e.context, state.active_weapon_handle))
            if (ammunition_weapon(weapon->definition))
                return weapon_metadata(*weapon, e);
    }
    if (player.eligibility_state != 0) {
        for (auto handle : state.inventory_handles) {
            if (handle == -1)
                continue;
            auto* weapon = e.weapon_by_handle(e.context, handle);
            if (weapon && e.weapon_is_weapon(e.context, *weapon) &&
                ammunition_weapon(weapon->definition))
                return weapon_metadata(*weapon, e);
        }
    }
    return {};
}
bool armored_hitgroup(i32 armor, bool helmet, i32 hitgroup) noexcept { // 433FC78E
    if (armor <= 0)
        return false;
    const auto bit = std::uint32_t(1) << (std::uint32_t(hitgroup) & 31);
    return (bit & 0x3D) || ((bit & 0x102) && helmet);
}
void set_player_spotted(const RuntimePlayer& player, PlayerDrawState& state, SpottedProxy proxy) {
    if (!proxy)
        return;
    SpottedProxyData packet;
    packet.words[2] = 1;
    packet.words[5] = 1;
    proxy(&packet, player.engine_entity, state.spotted_destination);
}
i32 player_health_number_threshold(const RuntimePlayer& target, const PlayerDrawState& target_state,
                                   const RuntimePlayer& view, const PlayerDrawState& view_state,
                                   const PlayerDrawEngine& e, client::PlayerStateCache& cache) {
    const auto metadata = player_weapon_metadata(view, view_state, e);
    if (metadata.definition != 64)
        return 92;
    const auto index = e.players.index(e.players.context, target);
    const auto& shared = *cache.get(std::uint32_t(index));
    const auto& g = shared.geometry;
    const auto view_origin = e.players.absolute_origin(e.players.context, view);
    const auto x = (view_origin.x + view_state.view_offset.x) - g.origin.x;
    const auto y = (view_origin.y + view_state.view_offset.y) - g.origin.y;
    const auto z = (view_origin.z + view_state.view_offset.z) -
                   (shared.cached_target_height * 0.5f + g.origin.z);
    const auto distance = std::sqrt((x * x + y * y) + z * z);
    const auto damage = float(std::pow(double(metadata.range_modifier), double(distance * 0.002f)) *
                              double(metadata.damage));
    const auto multiplier = armored_hitgroup(target_state.flags.armor, target_state.flags.helmet, 3)
                                ? (metadata.armor_ratio * 0.5f) * 1.25f
                                : 1.25f;
    return truncate(multiplier * damage);
}

std::int32_t bomb_carrier_index(const PlayerFlagGlobals& globals) noexcept {
    if (!globals.game_rules || !*globals.game_rules)
        return -1;
    const auto* rules = static_cast<const std::byte*>(*globals.game_rules);
    if (rules[globals.rules_guard] != std::byte{} || rules[globals.rules_state] != std::byte{} ||
        !globals.player_resource || !*globals.player_resource)
        return -1;
    std::int32_t index;
    std::memcpy(&index,
                static_cast<const std::byte*>(*globals.player_resource) + globals.carrier_index, 4);
    return index;
}

std::uint32_t player_overlay_flags(const RuntimePlayer& player, const PlayerDrawState& state,
                                   const PlayerDrawSnapshotContext& c) {
    const auto& e = c.engine;
    const auto index = e.players.index(e.players.context, player);
    const auto& cached = *c.cache.get(std::uint32_t(index));
    const auto& raw = state.flags;
    auto history = [](const client::PlayerState& entry) {
        RecordHistory result;
        result.count = entry.history.count;
        result.first = entry.history.first;
        if (result.count)
            result.ticks[result.first & 31] =
                entry.history.records[result.first & 31]->header.simulation_tick;
        if (result.count >= 2)
            result.ticks[(result.first + 1) & 31] =
                entry.history.records[(result.first + 1) & 31]->header.simulation_tick;
        const auto* bytes = reinterpret_cast<const std::byte*>(&entry.native);
        std::memcpy(&result.correction_index, bytes + 0x234, 4);
        std::memcpy(&result.correction_count, bytes + 0x238, 4);
        for (unsigned i = 0; i < 6; ++i) {
            std::memcpy(&result.corrections[i].counter, bytes + 0x23c + i * 32, 4);
            std::memcpy(&result.corrections[i].limit, bytes + 0x254 + i * 32, 4);
        }
        return result;
    };
    std::uint32_t flags = 0;
    if (c.rules.players.game_mode != 2) {
        flags = (raw.helmet ? 1u : 0u) | (raw.armor > 0 ? 2u : 0u);
        if (flags == 3)
            flags = 4;
    }
    if (!e.players.dormant(e.players.context, player)) {
        if (raw.scoped)
            flags |= 8;
        if (raw.flash_alpha > 20.0f)
            flags |= 0x10;
        if (raw.vip)
            flags |= 0x80;
        if (player.team == 3 && raw.defusing)
            flags |= 0x100;
        const auto fake_duck_index = e.players.index(e.players.context, player);
        if (fake_duck(history(*c.cache.get(std::uint32_t(fake_duck_index))), raw.duck_amount,
                      raw.duck_speed))
            flags |= 0x200;
        if (state.active_weapon_handle != -1) {
            if (const auto* weapon = e.weapon_by_handle(e.context, state.active_weapon_handle)) {
                if (weapon->ammunition != -1 && player_reload_cycle(player, c.reload) != 1.0f)
                    flags |= 0x20;
                if (e.weapon_kind(e.context, *weapon) == 9 && weapon->pin_pulled)
                    flags |= 0x400;
            }
        }
    }
    const auto carrier = bomb_carrier_index(c.flag_globals);
    if (e.players.index(e.players.context, player) == carrier)
        flags |= 0x40;
    const auto engine_time = e.players.time(e.players.context);
    const auto now = truncate(
        float(static_cast<long double>(engine_time) / static_cast<long double>(raw.tick_interval) +
              0.5L));
    const auto* bytes = reinterpret_cast<const std::byte*>(&cached.native);
    std::int32_t last_hit_tick;
    std::memcpy(&last_hit_tick, bytes + 0x100, 4);
    const auto difference = sub(last_hit_tick, now);
    const auto magnitude = difference < 0 ? sub(0, difference) : difference;
    if (magnitude <= 16)
        flags |= 0x800;
    if (*c.flag_globals.rage_enabled) {
        const auto handle = *c.flag_globals.on_shot_target;
        auto* target = handle == -1 ? nullptr : e.players.by_handle(e.players.context, handle);
        if (target == &player && e.players.local_player &&
            e.players.local_player->life_state == 0 && *c.flag_globals.on_shot_active)
            flags |= 0x1000;
        const auto records = history(cached);
        if (records.count) {
            const auto tick = truncate(raw.simulation_time / raw.simulation_tick_interval + 0.5f);
            if (tick < records.ticks[records.first & 31])
                flags |= 0x20000;
            else if (correction_pending(records))
                flags |= 0x2000;
        }
    }
    return flags;
}

void draw_prepared_player(ui::RenderQueue& queue, const PlayerEspConfig& config,
                          const WorldEspConfig& world, const PreparedPlayer& prepared,
                          PlayerDrawState& state, const PlayerDrawState& view_state,
                          const PlayerDrawSnapshotContext& c) {
    if (prepared.stage != PreparedPlayerStage::overlay)
        return;
    auto& player = *prepared.player;
    auto& geometry = *prepared.geometry;
    const auto& engine = c.engine;
    if (!prepared.dormant && !prepared.teammate) {
        ViewPredicate predicate{&engine.players, prepared.view_player};
        draw_offscreen_indicator(queue, config,
                                 {c.screen, c.view_origin, geometry.origin, c.view_pitch,
                                  c.view_yaw, c.animation_time, c.camera_fov},
                                 {&predicate, view_is_player});
    }
    if (needs_box(config)) {
        RuntimeWeapon* weapon = nullptr;
        if (state.active_weapon_handle != -1)
            weapon = engine.weapon_by_handle(engine.context, state.active_weapon_handle);
        Rectangle box;
        if (player_box(c.projection, geometry.upper, geometry.lower, box)) {
            if (config.health_bar.value) {
                const auto health = overlay_player_health(&player, engine, c.rules);
                const auto threshold = player_health_number_threshold(
                    player, state, *prepared.view_player, view_state, engine, c.cache);
                draw_health(queue, box, health, threshold, config.health_color.value);
            }
            const auto small_height = ui::font_height(*queue.fonts, 4);
            const auto normal_height = ui::font_height(*queue.fonts, 0);
            if (config.name.value) {
                std::string_view name;
                if (engine.player_info(engine.context, prepared.entity_index, name)) {
                    const auto wide = player_name(name);
                    if (!wide.empty())
                        draw_name(queue, box, wide, config.name_color.value, normal_height);
                }
            }
            Point side{add(add(box.x, box.width), 2), box.y};
            auto baseline = add(box.y, box.height);
            if (c.money && !prepared.dormant) {
                auto value = std::u16string(u"$") + decimal(state.money);
                queue.text(side.x, side.y, 0xD215C27Bu, 4, 0, value);
                side.y = add(side.y, small_height);
            }
            float reload = 1.0f;
            if (!prepared.dormant && weapon && weapon->ammunition != -1)
                reload = player_reload_cycle(player, c.reload);
            if (config.flags.value) {
                draw_flags(queue, side, small_height, player_overlay_flags(player, state, c),
                           c.flag_entries);
                if (c.scripts && c.script_flags)
                    script::draw_flags(*c.scripts, *c.script_flags, queue, prepared.entity_index,
                                       side, small_height);
            }
            if (weapon && config.ammo.value && !prepared.dormant && weapon->ammunition != -1) {
                const auto capacity = engine.weapon_capacity(engine.context, *weapon);
                if (capacity > 0)
                    baseline = draw_ammo(queue, box, baseline, weapon->ammunition, capacity, reload,
                                         config.ammo_color.value);
            }
            if (config.distance.value) {
                draw_distance(
                    queue, add(box.x, box.width / 2), baseline, c.view_origin.x - geometry.origin.x,
                    c.view_origin.y - geometry.origin.y, c.view_origin.z - geometry.origin.z);
                baseline = add(baseline, small_height);
            }
            auto icon_baseline = baseline;
            if (config.weapon_text.value) {
                draw_weapon_text(queue, engine.names, prepared.cache_state->cached_definition,
                                 {add(box.x, box.width / 2), add(baseline, 6)}, 0xC8FFFFFFu);
                icon_baseline = add(baseline, small_height);
            }
            if (config.weapon_icon.value && !c.suspended && box.width > 1)
                draw_weapon_icon(queue, engine.icons, prepared.cache_state->cached_definition,
                                 {add(box.x, box.width / 2), add(icon_baseline, 5)},
                                 config.weapon_icon_color.value);
            if (config.bounding_box.value)
                draw_box(queue, box, config.box_color.value);
        }
    }
    if (!prepared.dormant) {
        auto& skeleton = state.skeleton;
        if (config.skeleton.value && skeleton.model_present && skeleton.studio_present) {
            const bool current =
                skeleton.counter_present && skeleton.entity_counter == skeleton.global_counter;
            if (current || engine.setup_bones(engine.context, player, -1, 0x300, c.current_time))
                draw_skeleton(queue, c.projection, skeleton.bones, skeleton.transforms,
                              config.skeleton_color.value);
        }
        if (world.radar.value && !c.suspended && !state.spotted)
            set_player_spotted(player, state, engine.spotted_proxy);
    }
    finish_player_overlay(queue);
}

} // namespace skeet::esp

// ============================================================================
// Subcomponent: player_engine.cpp
// ============================================================================

namespace skeet::esp {
namespace {
/* [removed duplicate field] */
std::int32_t convar(const void* value) {
    while (value != field<const void*>(value, 0x1c))
        value = field<const void*>(value, 0x1c);
    return std::bit_cast<std::int32_t>(field<std::uint32_t>(value, 0x30) ^
                                       std::uint32_t(reinterpret_cast<std::uintptr_t>(value)));
}
}
EnginePlayers::EnginePlayers(PlayerEngineBindings bindings) : bindings_(bindings) {
    runtime_.context = this;
    runtime_.by_index = [](void* c, std::int32_t i) {
        return static_cast<EnginePlayers*>(c)->by_index(i);
    };
    runtime_.by_handle = [](void* c, std::int32_t h) {
        return static_cast<EnginePlayers*>(c)->by_handle(h);
    };
    runtime_.is_player = [](void* c, const RuntimePlayer& p) {
        return static_cast<EnginePlayers*>(c)->is_player(p);
    };
    runtime_.index = [](void* c, const RuntimePlayer& p) {
        return static_cast<EnginePlayers*>(c)->index(p);
    };
    runtime_.dormant = [](void* c, const RuntimePlayer& p) {
        return static_cast<EnginePlayers*>(c)->dormant(p);
    };
    runtime_.absolute_origin = [](void* c, const RuntimePlayer& p) {
        return static_cast<EnginePlayers*>(c)->origin(p);
    };
    runtime_.minimum_bounds = [](void* c, const RuntimePlayer& p) {
        return static_cast<EnginePlayers*>(c)->minimum(p);
    };
    runtime_.maximum_bounds = [](void* c, const RuntimePlayer& p) {
        return static_cast<EnginePlayers*>(c)->maximum(p);
    };
    runtime_.time = [](void* c) { return static_cast<EnginePlayers*>(c)->time(); };
}
std::ptrdiff_t EnginePlayers::offset(std::uint32_t pointer, std::uint32_t key,
                                     std::uint32_t mask) const {
    const auto& globals = bindings_.offsets.globals;
    return static_cast<std::ptrdiff_t>(*globals.pointers.at(pointer) ^ globals.keys.at(key) ^ mask);
}
RuntimePlayer* EnginePlayers::view(void* entity) {
    if (!entity)
        return nullptr;
    auto& result = players_[entity];
    result.engine_entity = entity;
    return &result;
}
RuntimePlayer* EnginePlayers::local() {
    return view(bindings_.local_player);
}
RuntimePlayer* EnginePlayers::by_index(std::int32_t i) {
    return view(engine::invoke<void*>(bindings_.entity_list, 3, i));
}
RuntimePlayer* EnginePlayers::by_handle(std::int32_t h) {
    return view(engine::invoke<void*>(bindings_.entity_list, 4, h));
}
RuntimeWeapon* EnginePlayers::weapon(std::int32_t handle) {
    auto* entity = engine::invoke<void*>(bindings_.entity_list, 4, handle);
    if (!entity)
        return nullptr;
    auto& result = weapons_[entity];
    result.engine_entity = entity;
    return &result;
}
PlayerRuntimeEngine& EnginePlayers::runtime_engine() {
    return runtime_;
}
bool EnginePlayers::is_player(const RuntimePlayer& p) const {
    return engine::invoke<bool>(p.engine_entity, 158);
}
bool EnginePlayers::is_weapon(const RuntimePlayer& p) const {
    return engine::invoke<bool>(p.engine_entity, 166);
}
std::uint16_t EnginePlayers::item_definition(const RuntimePlayer& p) const {
    return field<std::uint16_t>(p.engine_entity, offset(0x4346908c, 0x43468ba0, 0x5c9f0fbb));
}
std::int32_t EnginePlayers::owner_handle(const RuntimePlayer& p) const {
    return field<std::int32_t>(p.engine_entity, offset(0x434679d8, 0x43467fb4, 0x87907bc3));
}
std::int32_t EnginePlayers::local_index() const {
    return engine::invoke<std::int32_t>(bindings_.engine_client, 12);
}
bool EnginePlayers::dormant(const RuntimePlayer& p) const {
    return engine::invoke<bool>(static_cast<std::byte*>(p.engine_entity) + 8, 9);
}
std::int32_t EnginePlayers::index(const RuntimePlayer& p) const {
    return engine::invoke<std::int32_t>(static_cast<std::byte*>(p.engine_entity) + 8, 10);
}
Vector3 EnginePlayers::origin(const RuntimePlayer& p) const {
    return *engine::invoke<const Vector3*>(p.engine_entity, 10);
}
Vector3 EnginePlayers::minimum(const RuntimePlayer& p) const {
    return *engine::invoke<const Vector3*>(p.engine_entity, 76);
}
Vector3 EnginePlayers::maximum(const RuntimePlayer& p) const {
    return *engine::invoke<const Vector3*>(p.engine_entity, 77);
}
float EnginePlayers::time() const {
    return engine::invoke<float>(bindings_.engine_client, 14);
}
float EnginePlayers::tick_interval() const noexcept {
    return field<float>(bindings_.engine_globals, 0x20);
}
float EnginePlayers::frame_time() const noexcept {
    return field<float>(bindings_.engine_globals, 0x14);
}
float EnginePlayers::absolute_frame_time() const noexcept {
    return field<float>(bindings_.engine_globals, 8);
}
#define PLAYER_FIELD(TYPE, NAME, PTR, KEY, MASK)                                                   \
    TYPE EnginePlayers::NAME(const RuntimePlayer& p) const {                                       \
        return field<TYPE>(p.engine_entity, offset(PTR, KEY, MASK));                               \
    }
PLAYER_FIELD(std::uint8_t, life, 0x43468e78, 0x434684e4, 0x9d920e35)
PLAYER_FIELD(std::uint8_t, movement, 0x43468bd4, 0x4346a678, 0x7501cb91)
PLAYER_FIELD(std::int32_t, observer_mode, 0x4346a6cc, 0x43468ef8, 0x2f4e97c4)
PLAYER_FIELD(std::int32_t, observer_target, 0x43468f84, 0x43467e64, 0x3b704b41)
PLAYER_FIELD(std::int32_t, team, 0x434688b0, 0x4346a518, 0x8b5586f6)
PLAYER_FIELD(std::int32_t, squad, 0x434678ac, 0x43467b7c, 0x430c8993)
PLAYER_FIELD(std::uint8_t, eligibility, 0x43468bf8, 0x43467b8c, 0xd070684c)
PLAYER_FIELD(std::uint8_t, suppressed, 0x4346a5dc, 0x43468430, 0xcde290d2)
PLAYER_FIELD(Vector3, network_origin, 0x43468d30, 0x43468ed4, 0x2a671b42)
PLAYER_FIELD(Vector3, view_offset, 0x43468a34, 0x43468c00, 0xc246315e)
PLAYER_FIELD(std::int32_t, armor, 0x43467908, 0x434682e4, 0x47a6f50c)
PLAYER_FIELD(float, flash, 0x43467434, 0x43467fa8, 0x1bc6d160)
PLAYER_FIELD(float, duck_amount, 0x434679f8, 0x434675f8, 0x3309c0cf)
PLAYER_FIELD(float, duck_speed, 0x43467dd4, 0x4346a52c, 0xfb7e3404)
PLAYER_FIELD(float, simulation_time, 0x43469198, 0x4346a538, 0xd5fd2e2c)
PLAYER_FIELD(std::int32_t, active_weapon, 0x43467a74, 0x434678c0, 0xcd812654)
PLAYER_FIELD(std::int32_t, money, 0x43467a58, 0x4346a7f4, 0x6a00049e)
#undef PLAYER_FIELD
#define PLAYER_BOOL(NAME, PTR, KEY, MASK)                                                          \
    bool EnginePlayers::NAME(const RuntimePlayer& p) const {                                       \
        return field<std::uint8_t>(p.engine_entity, offset(PTR, KEY, MASK)) != 0;                  \
    }
PLAYER_BOOL(helmet, 0x43468c04, 0x43468fe8, 0x7d6280c7)
PLAYER_BOOL(scoped, 0x43468da4, 0x434684ec, 0xcacaeecc)
PLAYER_BOOL(vip, 0x43467c38, 0x43468498, 0xaf6689c2)
PLAYER_BOOL(defusing, 0x4346a7e0, 0x43468c34, 0xa15c3105)
#undef PLAYER_BOOL
float EnginePlayers::pose(const RuntimePlayer& p, unsigned index) const {
    return field<float>(p.engine_entity, offset(0x43467d78, 0x43467b80, 0x1caaf0a8) + index * 4);
}
std::int32_t EnginePlayers::inventory_weapon(const RuntimePlayer& p, unsigned index) const {
    return field<std::int32_t>(p.engine_entity,
                               offset(0x434685d4, 0x43467af4, 0xfc3297f7) + index * 4);
}
std::int32_t EnginePlayers::health(const RuntimePlayer& p) const {
    return engine::invoke<std::int32_t>(p.engine_entity, 122);
}
std::uint16_t EnginePlayers::definition(const RuntimeWeapon& w) const {
    return field<std::uint16_t>(w.engine_entity, offset(0x4346908c, 0x43468ba0, 0x5c9f0fbb));
}
std::int32_t EnginePlayers::ammunition(const RuntimeWeapon& w) const {
    return field<std::int32_t>(w.engine_entity, offset(0x4346a60c, 0x434678a8, 0xca5ec698));
}
bool EnginePlayers::pin_pulled(const RuntimeWeapon& w) const {
    return field<std::uint8_t>(w.engine_entity, offset(0x434670ec, 0x43467b1c, 0xe1df7e9b)) != 0;
}
float EnginePlayers::reload_cycle(const RuntimePlayer& p) const {
    const auto* descriptor =
        static_cast<const std::byte*>(p.engine_entity) + offset(0x4346a660, 0x434679e0, 0xb538bba5);
    if (field<std::int32_t>(descriptor, 0xc) < 1)
        return 1.0f;
    const auto* layer = field<const std::byte*>(descriptor, 0);
    if (field<float>(layer, 0x58) == 0.0f)
        return 1.0f;
    const auto sequence = field<std::int32_t>(layer, 0x50);
    const auto activity =
        bindings_.sequence_activity ? bindings_.sequence_activity(p.engine_entity, sequence) : -1;
    return activity == 0x3c7 ? field<float>(layer, 0x64) : 1.0f;
}
bool EnginePlayers::is_weapon(const RuntimeWeapon& w) const {
    return engine::invoke<bool>(w.engine_entity, 166);
}
WeaponInformation EnginePlayers::information(const RuntimeWeapon& w) const {
    const auto* info = information_pointer(w);
    return {field<float>(info, 0x100), field<float>(info, 0x10c), field<float>(info, 0x110),
            field<std::int32_t>(info, 0xfc)};
}
const void* EnginePlayers::information_pointer(const RuntimeWeapon& w) const {
    return engine::invoke<const void*>(w.engine_entity, 461);
}
float EnginePlayers::armor_ratio(const RuntimeWeapon& w) const {
    return engine::invoke<float>(w.engine_entity, 434);
}
std::int32_t EnginePlayers::damage(const RuntimeWeapon& w) const {
    return engine::invoke<std::int32_t>(w.engine_entity, 431);
}
std::int32_t EnginePlayers::capacity(const RuntimeWeapon& w) const {
    return engine::invoke<std::int32_t>(w.engine_entity, 375);
}
std::int32_t EnginePlayers::kind(const RuntimeWeapon& w) const {
    return engine::invoke<std::int32_t>(w.engine_entity, 455);
}
bool EnginePlayers::player_info(std::int32_t index, std::string_view& name) {
    if (!engine::invoke<bool>(bindings_.engine_client, 8, index, player_info_.data()))
        return false;
    name = {reinterpret_cast<const char*>(player_info_.data() + 0x10), 20};
    return true;
}
bool EnginePlayers::player_info(std::int32_t index, hud::ObserverPlayerInfo& output) {
    std::array<std::byte, 0x158> info{};
    if (!engine::invoke<bool>(bindings_.engine_client, 8, index, info.data()))
        return false;
    std::memcpy(output.name.data(), info.data() + 0x10, output.name.size());
    //4334E62B/639 test buffer+13D (HLTV) and+13C (fake player).
    output.fake_player = info[0x13c] != std::byte{};
    output.hltv = info[0x13d] != std::byte{};
    return true;
}
std::int32_t EnginePlayers::game_type() const {
    return convar(bindings_.game_type);
}
std::int32_t EnginePlayers::game_mode() const {
    return convar(bindings_.game_mode);
}
bool EnginePlayers::teammates_are_enemies() const {
    return convar(bindings_.teammates_are_enemies) != 0;
}
void* EnginePlayers::resource() const {
    return bindings_.player_resource ? *bindings_.player_resource : nullptr;
}
bool EnginePlayers::connected(const void* r, std::int32_t i) const {
    return field<std::uint8_t>(r, offset(0x43467cb8, 0x4346a814, 0x9571d5f5) + i) != 0;
}
bool EnginePlayers::alive(const void* r, std::int32_t i) const {
    return field<std::uint8_t>(r, offset(0x4346a724, 0x4346857c, 0xd266b537) + i) != 0;
}
std::int32_t EnginePlayers::resource_team(const void* r, std::int32_t i) const {
    return field<std::int32_t>(r, offset(0x434684f0, 0x4346a558, 0xc1c49a34) + i * 4);
}
std::int32_t EnginePlayers::resource_health(const void* r, std::int32_t i) const {
    return field<std::int32_t>(r, offset(0x4346716c, 0x434675b4, 0x9bb80163) + i * 4);
}
bool EnginePlayers::retain_dormant_alpha() const {
    return bindings_.game_rules && *bindings_.game_rules &&
           field<std::uint8_t>(*bindings_.game_rules, offset(0x43467910, 0x43467c08, 0xd720aea6)) !=
               0;
}
float EnginePlayers::rule_time() const {
    if (!bindings_.game_rules || !*bindings_.game_rules)
        return 0.0f;
    return field<float>(*bindings_.game_rules, offset(0x43468d6c, 0x43468428, 0x9f8e7f71));
}
std::int32_t EnginePlayers::bomb_carrier() const {
    if (!bindings_.game_rules || !*bindings_.game_rules)
        return -1;
    const auto* rules = *bindings_.game_rules;
    if (field<std::uint8_t>(rules, offset(0x4346746c, 0x43467d50, 0xd000a932)) ||
        field<std::uint8_t>(rules, offset(0x4346921c, 0x43468f24, 0x82f5ad08)))
        return -1;
    auto* resource = EnginePlayers::resource();
    if (!resource)
        return -1;
    return field<std::int32_t>(resource, offset(0x4346a370, 0x43468d70, 0x23b532ef));
}
bool EnginePlayers::spotted(const RuntimePlayer& p) const {
    return field<std::uint8_t>(p.engine_entity, offset(0x43467d48, 0x4346722c, 0xfb1e8dab)) != 0;
}
void EnginePlayers::set_spotted(const RuntimePlayer& p, SpottedProxy proxy) const {
    if (!proxy)
        return;
    SpottedProxyData packet;
    packet.words[2] = 1;
    packet.words[5] = 1;
    proxy(&packet, p.engine_entity,
          static_cast<std::byte*>(p.engine_entity) + offset(0x43467d48, 0x4346722c, 0xfb1e8dab));
}
void EnginePlayers::set_spotted(const RuntimePlayer& p) const {
    const auto* slot = bindings_.offsets.property_proxy_slots[2];
    const auto proxy = slot ? reinterpret_cast<SpottedProxy>(*slot) : nullptr;
    set_spotted(p, proxy);
}
} //namespace skeet::esp

// ============================================================================
// Subcomponent: player_engine_draw.cpp
// ============================================================================

namespace skeet::esp {
using namespace detail;
namespace {
/* [removed duplicate field] */
RecordHistory history(const client::PlayerState& entry) {
    RecordHistory out;
    out.count = entry.history.count;
    out.first = entry.history.first;
    if (out.count)
        out.ticks[out.first & 31] = entry.history.records[out.first & 31]->header.simulation_tick;
    if (out.count >= 2)
        out.ticks[(out.first + 1) & 31] =
            entry.history.records[(out.first + 1) & 31]->header.simulation_tick;
    const auto* bytes = reinterpret_cast<const std::byte*>(&entry.native);
    out.correction_index = field<std::uint32_t>(bytes, 0x234);
    out.correction_count = field<std::uint32_t>(bytes, 0x238);
    for (unsigned i = 0; i < 6; ++i) {
        out.corrections[i].counter = field<std::int32_t>(bytes, 0x23c + i * 32);
        out.corrections[i].limit = field<std::int32_t>(bytes, 0x254 + i * 32);
    }
    return out;
}
/* [removed duplicate needs_box] */
PlayerWeaponMetadata metadata(const RuntimeWeapon& weapon, EnginePlayers& source) {
    const auto* info = source.information_pointer(weapon);
    PlayerWeaponMetadata result;
    result.definition = source.definition(weapon);
    result.penetration = field<float>(info, 0x100);
    result.range = field<float>(info, 0x10c);
    result.range_modifier = field<float>(info, 0x110);
    result.armor_ratio = source.armor_ratio(weapon);
    result.damage = float(source.damage(weapon));
    result.kind = field<std::int32_t>(info, 0xfc);
    return result;
}
PlayerWeaponMetadata metadata(const RuntimePlayer& player, EnginePlayers& source) {
    auto handle = source.active_weapon(player);
    if (handle != -1)
        if (auto* weapon = source.weapon(handle))
            if (ammunition_weapon(source.definition(*weapon)))
                return metadata(*weapon, source);
    if (source.eligibility(player))
        for (unsigned index = 0; index < 2; ++index) {
            handle = source.inventory_weapon(player, index);
            if (handle == -1)
                continue;
            auto* weapon = source.weapon(handle);
            if (weapon && source.is_weapon(*weapon) &&
                ammunition_weapon(source.definition(*weapon)))
                return metadata(*weapon, source);
        }
    return {}; //the seven literal words at4346F1B8
}
std::int32_t health(const RuntimePlayer& player, EnginePlayers& source) {
    if (!source.dormant(player))
        return source.health(player);
    auto* resource = source.resource();
    if (!resource)
        return 0;
    const auto index = source.index(player);
    return source.connected(resource, index) ? source.resource_health(resource, index) : 0;
}
std::int32_t threshold(const RuntimePlayer& target, const RuntimePlayer& view,
                       EnginePlayers& source, client::PlayerStateCache& cache) {
    const auto weapon = metadata(view, source);
    if (weapon.definition != 64)
        return 92;
    const auto index = source.index(target);
    const auto& shared = *cache.get(std::uint32_t(index));
    const auto target_origin = shared.geometry.origin;
    const auto target_height = shared.cached_target_height;
    const auto origin = source.origin(view);
    const auto eye = source.view_offset(view);
    const auto x = (origin.x + eye.x) - target_origin.x, y = (origin.y + eye.y) - target_origin.y;
    const auto z = (origin.z + eye.z) - (target_height * 0.5f + target_origin.z);
    const auto distance = std::sqrt((x * x + y * y) + z * z);
    const auto damage = float(std::pow(double(weapon.range_modifier), double(distance * 0.002f)) *
                              double(weapon.damage));
    // Hitgroup3 belongs to mask3D, so433FC78E never reads the helmet byte here.
    const auto multiplier = source.armor(target) > 0 ? (weapon.armor_ratio * 0.5f) * 1.25f : 1.25f;
    return truncate(multiplier * damage);
}
}
PlayerWeaponMetadata player_weapon_metadata(const RuntimePlayer& player, EnginePlayers& source) {
    return metadata(player, source);
}
std::uint32_t player_overlay_flags(const RuntimePlayer& player, const PlayerDrawContext& c,
                                   EnginePlayers& source) {
    const auto index = source.index(player);
    const auto& cached = *c.cache.get(std::uint32_t(index));
    std::uint32_t flags = 0;
    if (source.game_mode() != 2) {
        flags = source.helmet(player) ? 1u : 0u;
        if (source.armor(player) > 0)
            flags |= 2;
        if (flags == 3)
            flags = 4;
    }
    if (!source.dormant(player)) {
        if (source.scoped(player))
            flags |= 8;
        if (source.flash(player) > 20.0f)
            flags |= 0x10;
        if (source.vip(player))
            flags |= 0x80;
        if (source.team(player) == 3 && source.defusing(player))
            flags |= 0x100;
        const auto fake_duck_index = source.index(player);
        const auto records = history(*c.cache.get(std::uint32_t(fake_duck_index)));
        if (records.count >= 2 &&
            sub(records.ticks[records.first & 31], records.ticks[(records.first + 1) & 31]) > 2) {
            const auto amount = source.duck_amount(player);
            if (amount > 0.0f && amount < 1.0f && source.duck_speed(player) == 8.0f)
                flags |= 0x200;
        }
        const auto handle = source.active_weapon(player);
        if (handle != -1)
            if (const auto* weapon = source.weapon(handle)) {
                if (source.ammunition(*weapon) != -1 && source.reload_cycle(player) != 1.0f)
                    flags |= 0x20;
                if (source.kind(*weapon) == 9 && source.pin_pulled(*weapon))
                    flags |= 0x400;
            }
    }
    const auto carrier = source.bomb_carrier();
    if (source.index(player) == carrier)
        flags |= 0x40;
    const auto time = source.time();
    const auto now = truncate(float(
        static_cast<long double>(time) / static_cast<long double>(source.tick_interval()) + 0.5L));
    const auto last = field<std::int32_t>(&cached.native, 0x100);
    const auto difference = sub(last, now),
               magnitude = difference < 0 ? sub(0, difference) : difference;
    if (magnitude <= 16)
        flags |= 0x800;
    if (source.rage_enabled()) {
        const auto handle = source.on_shot_target();
        auto* target = handle == -1 ? nullptr : source.by_handle(handle);
        if (target == &player) {
            auto* local = source.local();
            if (local && source.life(*local) == 0 && source.on_shot_active())
                flags |= 0x1000;
        }
        const auto records = history(cached);
        if (records.count) {
            const auto tick =
                truncate(source.simulation_time(player) / source.simulation_tick_interval() + 0.5f);
            if (tick < records.ticks[records.first & 31])
                flags |= 0x20000;
            else if (correction_pending(records))
                flags |= 0x2000;
        }
    }
    return flags;
}
void EnginePlayers::skeleton(ui::RenderQueue& queue, const Projection& projection,
                             const RuntimePlayer& player, std::uint32_t color) const {
    //433FD30D retains the wrapper; setup may replace its header and bone cache.
    const auto* wrapper =
        field<const std::byte*>(player.engine_entity, offset(0x43467e8c, 0x43467900, 0x32e4daf1));
    if (!wrapper || !field<const void*>(wrapper, 0))
        return;
    if (!bindings_.bone_counter ||
        field<std::int32_t>(player.engine_entity, offset(0x4346a808, 0x43468be8, 0x2b554ad0)) !=
            *bindings_.bone_counter)
        if (!engine::invoke<bool>(static_cast<std::byte*>(player.engine_entity) + 4, 13,
                                  static_cast<Matrix3x4*>(nullptr), -1, 0x300,
                                  *bindings_.current_time))
            return;
    const auto* cache = static_cast<const std::byte*>(player.engine_entity) +
                        offset(0x43469064, 0x4346a484, 0x17978090);
    const auto count = std::min(field<std::int32_t>(field<const void*>(wrapper, 0), 0x9c),
                                field<std::int32_t>(cache, 0xc));
    if (count > 256)
        throw std::length_error("Skeleton exceeds native bone workspace");
    std::array<Point, 256> points;
    std::array<std::uint32_t, 8> projected{};
    for (std::int32_t i = 0; i < count; ++i) {
        const auto* header = field<const std::byte*>(wrapper, 0);
        const auto* bone = header + field<std::int32_t>(header, 0xa0) + std::ptrdiff_t(i) * 0xd8;
        if (!(field<std::uint32_t>(bone, 0xa0) & 0x100))
            continue;
        const auto* matrix = field<const Matrix3x4*>(cache, 0) + i;
        if (!project_point(projection, {matrix->rows[0][3], matrix->rows[1][3], matrix->rows[2][3]},
                           points[std::size_t(i)]))
            continue;
        projected[std::uint32_t(i) >> 5] |= std::uint32_t(1) << (i & 31);
        const auto parent = field<const std::uint32_t*>(wrapper, 0x44)[i];
        if (parent == UINT32_MAX)
            continue;
        if (parent >= 256)
            throw std::out_of_range("Skeleton parent exceeds native bone workspace");
        if (!(projected[parent >> 5] & (std::uint32_t(1) << (parent & 31))))
            continue;
        const auto a = points[parent], b = points[std::size_t(i)];
        queue.line({a.x, a.y, b.x, b.y, color});
    }
}
void draw_live_prepared_player(ui::RenderQueue& queue, const PlayerEspConfig& config,
                               const WorldEspConfig& world, const PreparedPlayer& prepared,
                               const PlayerDrawContext& c, EnginePlayers& source) {
    if (prepared.stage != PreparedPlayerStage::overlay)
        return;
    auto& player = *prepared.player;
    auto& geometry = *prepared.geometry;
    if (!prepared.dormant && !prepared.teammate && config.out_of_fov.value &&
        source.is_player(*prepared.view_player)) {
        const auto screen = queue.screen();
        draw_selected_offscreen_indicator(queue, config,
                                          {screen, source.view_origin(), geometry.origin,
                                           source.view_pitch(), source.view_yaw(),
                                           source.animation_time(), c.camera_fov});
    }
    if (needs_box(config)) {
        RuntimeWeapon* weapon = nullptr;
        const auto handle = source.active_weapon(player);
        if (handle != -1)
            weapon = source.weapon(handle);
        Rectangle box;
        if (player_box(c.projection, geometry.upper, geometry.lower, box)) {
            if (config.health_bar.value) {
                const auto value = health(player, source);
                const auto number = threshold(player, *prepared.view_player, source, c.cache);
                draw_health(queue, box, value, number, config.health_color.value);
            }
#ifdef small
#undef small
#endif
            const auto small = ui::font_height(*queue.fonts, 4),
                       normal = ui::font_height(*queue.fonts, 0);
            if (config.name.value) {
                std::string_view name;
                if (source.player_info(prepared.entity_index, name)) {
                    const auto wide = player_name(name);
                    if (!wide.empty())
                        draw_name(queue, box, wide, config.name_color.value, normal);
                }
            }
            Point side{add(add(box.x, box.width), 2), box.y};
            auto baseline = add(box.y, box.height);
            if (c.money && !prepared.dormant) {
                const auto value = std::u16string(u"$") + decimal(source.money(player));
                queue.text(side.x, side.y, 0xd215c27bu, 4, 0, value);
                side.y = add(side.y, small);
            }
            float reload = 1.0f;
            if (!prepared.dormant && weapon && source.ammunition(*weapon) != -1)
                reload = source.reload_cycle(player);
            if (config.flags.value) {
                draw_flags(queue, side, small, player_overlay_flags(player, c, source), c.flags);
                if (c.scripts && c.script_flags)
                    script::draw_flags(*c.scripts, *c.script_flags, queue, prepared.entity_index,
                                       side, small);
            }
            if (weapon && config.ammo.value && !prepared.dormant) {
                const auto ammunition = source.ammunition(*weapon);
                if (ammunition != -1) {
                    const auto capacity = source.capacity(*weapon);
                    if (capacity > 0)
                        baseline = draw_ammo(queue, box, baseline, ammunition, capacity, reload,
                                             config.ammo_color.value);
                }
            }
            if (config.distance.value) {
                const auto& view = source.view_origin();
                draw_distance(queue, add(box.x, box.width / 2), baseline,
                              view.x - geometry.origin.x, view.y - geometry.origin.y,
                              view.z - geometry.origin.z);
                baseline = add(baseline, small);
            }
            auto icon_baseline = baseline;
            if (config.weapon_text.value) {
                draw_weapon_text(queue, c.names, prepared.cache_state->cached_definition,
                                 {add(box.x, box.width / 2), add(baseline, 6)}, 0xc8ffffffu);
                icon_baseline = add(baseline, small);
            }
            if (config.weapon_icon.value && !source.suspended() && box.width > 1)
                draw_weapon_icon(queue, c.icons, prepared.cache_state->cached_definition,
                                 {add(box.x, box.width / 2), add(icon_baseline, 5)},
                                 config.weapon_icon_color.value);
            if (config.bounding_box.value)
                draw_box(queue, box, config.box_color.value);
        }
    }
    if (!prepared.dormant) {
        if (config.skeleton.value)
            source.skeleton(queue, c.projection, player, config.skeleton_color.value);
        if (world.radar.value && !source.suspended() && !source.spotted(player))
            source.set_spotted(player);
    }
    finish_player_overlay(queue);
}
} //namespace skeet::esp

// ============================================================================
// Subcomponent: player_engine_runtime.cpp
// ============================================================================
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

namespace skeet::esp {
using namespace detail;
namespace {
bool enemy_teams(std::int32_t a, std::int32_t b, EnginePlayers& source) {
    if (a == 2)
        return b == 3 || source.teammates_are_enemies();
    if (a == 3)
        return b == 2 || source.teammates_are_enemies();
    return false;
}
}
void update_player_geometry(PlayerGeometry& out, const RuntimePlayer& player, EnginePlayers& source,
                            Vector3 origin) {
    out.origin = origin;
    out.center = origin;
    const auto minimum = source.minimum(player);
    const auto maximum = source.maximum(player);
    out.center.z += (minimum.z + maximum.z) * 0.5f;
    out.eye_height = source.pose(player, 1) * 18.0f + 58.0f;
    out.pose_adjustment = source.pose(player, 12) * 6.0f;
    const auto second_maximum = source.maximum(player);
    out.lower = {out.center.x, out.center.y, out.center.z - second_maximum.z * 0.6f};
    out.upper = {out.center.x, out.center.y, (out.origin.z + out.eye_height) - out.pose_adjustment};
}
RuntimePlayer* effective_view_player(EnginePlayers& source) {
    auto* local = source.local();
    if (!local || source.life(*local) == 0 || source.movement(*local) != 10 ||
        (source.observer_mode(*local) != 4 && source.observer_mode(*local) != 5))
        return local;
    const auto handle = source.observer_target(*local);
    if (handle == -1)
        return local;
    auto* target = source.by_handle(handle);
    return target && source.is_player(*target) ? target : local;
}
bool live_enemy(const RuntimePlayer* first, const RuntimePlayer* second, EnginePlayers& source) {
    if (!first || !second || first == second)
        return false;
    if (source.game_type() == 6) {
        const auto squad = source.squad(*first);
        return squad == -1 || squad != source.squad(*second);
    }
    const auto a = source.team(*first), b = source.team(*second);
    return enemy_teams(a, b, source);
}
bool dormant_enemy(const RuntimePlayer* first, const RuntimePlayer* second, EnginePlayers& source) {
    if (!first || !second || first == second)
        return false;
    if (source.game_type() == 6) {
        const auto squad = source.squad(*first);
        return squad == -1 || squad != source.squad(*second);
    }
    auto* resource = source.resource();
    if (!resource)
        return false;
    const auto first_index = source.index(*first), second_index = source.index(*second);
    if (!source.connected(resource, first_index) || !source.connected(resource, second_index))
        return false;
    const auto b = source.resource_team(resource, second_index),
               a = source.resource_team(resource, first_index);
    return enemy_teams(a, b, source);
}
bool dormant_alive(const RuntimePlayer* player, EnginePlayers& source) {
    if (!player)
        return false;
    auto* resource = source.resource();
    if (!resource)
        return false;
    const auto index = source.index(*player);
    return source.connected(resource, index) && source.alive(resource, index);
}
PreparedPlayer prepare_live_player(ui::RenderQueue& queue, PlayerRuntimeState& state,
                                   const PlayerEspConfig& config, const WorldEspConfig& world,
                                   EnginePlayers& source, RuntimePlayer* initial_view,
                                   std::int32_t slot, bool activation, bool blocked) {
    PreparedPlayer output;
    auto reject_slot = [&] {
        state.geometry(std::uint32_t(slot)).alpha = 0.0f;
        return output;
    };
    if (!activation || blocked)
        return reject_slot();
    auto* player = source.by_index(slot);
    if (!player || player == initial_view || !source.is_player(*player) ||
        source.eligibility(*player))
        return reject_slot();
    const auto index = source.index(*player);
    auto& shared = *state.cache.get(std::uint32_t(index));
    auto& g = shared.geometry;
    auto reject = [&] {
        g.alpha = 0.0f;
        return output;
    };
    if (source.suppressed(*player))
        return reject();
    const auto dormant = source.dormant(*player);
    bool teammate, alive;
    if (dormant) {
        if (!source.resource())
            return reject();
        teammate = !dormant_enemy(initial_view, player, source);
        alive = dormant_alive(player, source);
    } else {
        teammate = !live_enemy(initial_view, player, source);
        alive = source.life(*player) == 0;
        if (alive) {
            const auto origin = source.origin(*player);
            update_player_geometry(g, *player, source, origin);
            const auto time = source.time();
            g.last_live_tick = truncate(float(static_cast<long double>(time) /
                                                  static_cast<long double>(source.tick_interval()) +
                                              0.5L));
        } else
            g.alpha = 0.0f;
        if (source.movement(*player) == 10 &&
            (source.observer_mode(*player) == 4 || source.observer_mode(*player) == 5)) {
            g.alpha = 0.0f;
            if (world.spectators.value)
                output = {PreparedPlayerStage::observer_panel,
                          player,
                          nullptr,
                          &g,
                          index,
                          false,
                          teammate,
                          &shared};
            return output;
        }
    }
    if (!alive || (!config.teammates.value && teammate))
        return reject();
    float alpha;
    if (dormant) {
        if (g.alpha == 0.0f)
            return output;
        if (!source.retain_dormant_alpha()) {
            g.alpha = sse_min(
                sse_max(g.alpha - source.frame_time() * (config.dormant.value ? 0.25f : 4.0f),
                        0.0f),
                1.0f);
            if (g.alpha == 0.0f)
                return output;
        }
        queue.set_desaturation(true);
        alpha = sse_min(0.8f, g.alpha);
    } else {
        alpha = sse_min(
            sse_max(g.alpha + source.frame_time() * std::bit_cast<float>(0x41555555u), 0.1f), 1.0f);
        g.alpha = alpha;
    }
    queue.set_alpha(alpha);
    auto* view = effective_view_player(source);
    if (!view || view == player || !source.is_player(*view)) {
        g.alpha = 0.0f;
        finish_player_overlay(queue);
        return output;
    }
    return {PreparedPlayerStage::overlay, player, view, &g, index, dormant, teammate, &shared};
}
void draw_live_observer(ui::RenderQueue& queue, PlayerFrameState& state, EnginePlayers& source,
                        RuntimePlayer* initial_view, const PreparedPlayer& prepared) {
    if (prepared.stage != PreparedPlayerStage::observer_panel || !initial_view || !prepared.player)
        return;
    const auto handle = source.observer_target(*prepared.player);
    if (handle == -1)
        return;
    auto* target = source.by_handle(handle);
    if (!target)
        return;
    auto* view = effective_view_player(source);
    if (!view)
        return;
    const auto view_index = source.index(*view), target_index = source.index(*target);
    if (view_index != target_index)
        return;
    hud::ObserverPlayerInfo info;
    if (!source.player_info(prepared.entity_index, info) || info.hltv || info.fake_player)
        return;
    info.name[20] = 0;
    std::array<wchar_t, 256> wide;
    const auto count =
        MultiByteToWideChar(CP_UTF8, 0, info.name.data(), -1, wide.data(), int(wide.size()));
    if (count <= 1)
        return;
    const auto row = std::uint32_t(state.transient);
    state.transient = std::bit_cast<std::int32_t>(row + 1);
    const auto width = queue.screen().x;
    queue.text(std::bit_cast<std::int32_t>(std::uint32_t(width) - 10),
               std::bit_cast<std::int32_t>(row * 20 + 5), 0xc8ffffffu, 0x88, 0,
               {reinterpret_cast<const char16_t*>(wide.data()), std::size_t(count - 1)});
}
} //namespace skeet::esp

// ============================================================================
// Subcomponent: player_overlay.cpp
// ============================================================================
#ifndef NOMINMAX
#define NOMINMAX
#endif

namespace skeet::esp {
using namespace detail;

std::u16string player_name(std::string_view utf8) {
    char input[21]{};
    std::copy_n(utf8.data(), std::min<std::size_t>(utf8.size(), 20), input);
    wchar_t output[256];
    const auto count = MultiByteToWideChar(CP_UTF8, 0, input, -1, output, 256);
    if (count <= 1)
        return {};
    static_assert(sizeof(wchar_t) == sizeof(char16_t));
    return {reinterpret_cast<const char16_t*>(output), std::size_t(count - 1)};
}

void draw_box(ui::RenderQueue& q, Rectangle b, std::uint32_t color) { // 0x4340AB68..0x4340AC1C
    q.rectangle_outline({sub(b.x, 1), sub(b.y, 1), add(b.width, 2), add(b.height, 2), 0x8C000000u},
                        1);
    q.rectangle_outline({b.x, b.y, b.width, b.height, color}, 1);
    q.rectangle_outline({add(b.x, 1), add(b.y, 1), sub(b.width, 2), sub(b.height, 2), 0x8C000000u},
                        1);
}

void draw_health(ui::RenderQueue& q, Rectangle b, i32 health,
                 i32 threshold) { // 0x4340A44D..0x4340A5C4
    const auto value = std::clamp(health, 0, 100);
    const auto fraction = (float(std::clamp(value, 25, 75)) - 25.0f) / 50.0f;
    const float green = fraction * 175.0f + 50.0f + 8388608.0f;
    const float red = 255.0f - fraction * 135.0f + 8388608.0f;
    const auto color = 0xFF500000u | ((std::bit_cast<std::uint32_t>(green) & 255) << 8) |
                       (std::bit_cast<std::uint32_t>(red) & 255);
    draw_health(q, b, health, threshold, color);
}

void draw_health(ui::RenderQueue& q, Rectangle b, i32 health, i32 threshold, std::uint32_t color) {
    const auto value = std::clamp(health, 0, 100);
    const auto opacity = color >> 24;
    const auto background = ((opacity * 160u) / 255u) << 24;
    const auto number_color = (((opacity * 240u) / 255u) << 24) | 0x00FFFFFFu;
    const auto filled = mul(b.height, value) / 100;
    const auto empty = sub(b.height, filled);
    q.rectangle_outline({sub(b.x, 6), sub(b.y, 1), 4, add(b.height, 2), background}, 1);
    q.rectangle({sub(b.x, 6), sub(b.y, 1), 4, empty, background});
    q.rectangle({sub(b.x, 5), add(b.y, empty), 2, filled, color});
    if (value <= threshold || health > 100)
        q.text(sub(b.x, 7), add(b.y, add(empty, 1)), number_color, 0x14, 0, decimal(health));
}

void draw_name(ui::RenderQueue& q, Rectangle b, std::u16string_view text, std::uint32_t color,
               i32 height) { // 0x4340A674..0x4340A6A6
    q.text(add(b.x, b.width / 2), sub(b.y, add(height, 2) / 2), color, 0x18, 0, text);
}

i32 draw_ammo(ui::RenderQueue& q, Rectangle b, i32 baseline, i32 count, i32 capacity, float reload,
              std::uint32_t color) { // 0x4340A8AE..0x4340A9FD
    const auto progress = reload != 1.0f ? reload : float(count) / float(capacity);
    const auto fraction = sse_min(sse_max(progress, 0.0f), 1.0f);
    const auto filled = truncate(float(b.width) * fraction);
    const auto background = (((color >> 24) * 160) / 255) << 24;
    q.rectangle({sub(b.x, 1), add(baseline, 2), add(b.width, 2), 4, background});
    q.rectangle({b.x, add(baseline, 3), filled, 2, color});
    if (fraction < 0.25f && reload == 1.0f)
        q.text(add(b.x, filled), add(baseline, 5), 0xC8FFFFFFu, 0x14, 0, decimal(count));
    return add(baseline, 5);
}

i32 distance_in_feet(float dx, float dy, float dz) noexcept { // 0x4340AA17..0x4340AAC6
    const float distance = std::sqrt((dx * dx + dy * dy) + dz * dz);
    return mul(sub(2, truncate(distance / -12.0f)) / 5, 5);
}

void draw_distance(ui::RenderQueue& q, i32 x, i32 baseline, float dx, float dy, float dz) {
    auto text = decimal(distance_in_feet(dx, dy, dz));
    text += u" FT";
    q.text(x, add(baseline, 6), 0xC8FFFFFFu, 0x14, 0, text);
}

OverlayLayout draw_player_overlay(ui::RenderQueue& q, const OverlayConfig& c, const OverlayInput& p,
                                  std::span<const FlagEntry, 18> table,
                                  OverlayCallbacks callbacks) {
    const auto b = p.box;
    OverlayLayout result{{add(add(b.x, b.width), 2), b.y}, add(b.y, b.height)};
    if (!(c.bounding_box.value || c.health_bar.value || c.name.value || c.flags.value ||
          c.ammo.value || c.distance.value))
        return result;
    if (c.health_bar.value)
        draw_health(q, b, p.health, p.health_number_threshold, c.health_color.value);
    if (!q.fonts)
        throw std::logic_error("Player ESP requires the renderer font backend");
    const auto small_height = ui::font_height(*q.fonts, 4);
    const auto normal_height = ui::font_height(*q.fonts, 0);
    if (c.name.value && p.player_info_available) {
        const auto name = player_name(p.name_utf8);
        if (!name.empty())
            draw_name(q, b, name, c.name_color.value, normal_height);
    }
    if (p.money_enabled && !p.dormant) {
        auto amount = std::u16string(u"$") + decimal(p.money);
        q.text(result.side.x, result.side.y, 0xD215C27Bu, 4, 0, amount);
        result.side.y = add(result.side.y, small_height);
    }
    if (c.flags.value) {
        draw_flags(q, result.side, small_height, p.flags, table);
        if (callbacks.extra_flags)
            callbacks.extra_flags(callbacks.context, q, p.entity_index, result.side, small_height);
    }
    if (p.weapon_valid && c.ammo.value && !p.dormant && p.ammunition != -1 && p.capacity > 0)
        result.baseline = draw_ammo(q, b, result.baseline, p.ammunition, p.capacity,
                                    p.reload_progress, c.ammo_color.value);
    if (c.distance.value) {
        draw_distance(q, add(b.x, b.width / 2), result.baseline, p.view_delta_x, p.view_delta_y,
                      p.view_delta_z);
        result.baseline = add(result.baseline, small_height);
    }
    if (c.bounding_box.value)
        draw_box(q, b, c.box_color.value);
    return result;
}

} // namespace skeet::esp

// ============================================================================
// Subcomponent: player_runtime.cpp
// ============================================================================

// The owner honors MXCSR rounding. In particular, multiplying by +0.6 then
// subtracting must not become multiplication by -0.6 followed by addition.
#pragma STDC FENV_ACCESS ON

namespace skeet::esp {
using namespace detail;
namespace {
bool team_enemy(i32 first, i32 second, bool teammates_are_enemies) noexcept {
    if (first == 2)
        return second == 3 || teammates_are_enemies;
    if (first == 3)
        return second == 2 || teammates_are_enemies;
    return false;
}
bool squad_enemy(const RuntimePlayer& first, const RuntimePlayer& second) noexcept {
    return first.squad == -1 || first.squad != second.squad;
}
PlayerGeometry& record(PlayerRuntimeState& state, i32 index) noexcept {
    return state.geometry(std::uint32_t(index));
}
}

RuntimePlayer* effective_view_player(const PlayerRuntimeEngine& engine) { // 43359AFA
    auto* local = engine.local_player;
    if (!local || local->life_state == 0 || local->move_type != 10 ||
        (local->observer_mode != 4 && local->observer_mode != 5) || local->observer_handle == -1)
        return local;
    auto* target = engine.by_handle(engine.context, local->observer_handle);
    return target && engine.is_player(engine.context, *target) ? target : local;
}
bool live_enemy(const RuntimePlayer* first, const RuntimePlayer* second,
                const PlayerRuntimeRules& rules) noexcept { // 4339EF8C
    if (!first || !second || first == second)
        return false;
    if (rules.game_mode == 6)
        return squad_enemy(*first, *second);
    return team_enemy(first->team, second->team, rules.teammates_are_enemies);
}
bool dormant_enemy(const RuntimePlayer* first, const RuntimePlayer* second,
                   const PlayerRuntimeEngine& engine, const PlayerRuntimeRules& rules) { // 433E4E4E
    if (!first || !second || first == second)
        return false;
    if (rules.game_mode == 6)
        return squad_enemy(*first, *second);
    if (!rules.present)
        return false;
    const auto first_index = engine.index(engine.context, *first);
    const auto second_index = engine.index(engine.context, *second);
    const auto& a = rules.players[std::uint32_t(first_index)];
    const auto& b = rules.players[std::uint32_t(second_index)];
    return a.connected && b.connected && team_enemy(a.team, b.team, rules.teammates_are_enemies);
}
bool dormant_alive(const RuntimePlayer* player, const PlayerRuntimeEngine& engine,
                   const PlayerRuntimeRules& rules) { // 433D7563
    if (!player || !rules.present)
        return false;
    const auto index = engine.index(engine.context, *player);
    const auto& entry = rules.players[std::uint32_t(index)];
    return entry.connected && entry.alive;
}
void update_player_geometry(PlayerGeometry& g, const RuntimePlayer& player,
                            const PlayerRuntimeEngine& engine, Vector3 origin) { // 4341AF4A
    g.origin = origin;
    g.center = origin;
    const auto minimum = engine.minimum_bounds(engine.context, player);
    const auto maximum = engine.maximum_bounds(engine.context, player);
    g.center.z += (minimum.z + maximum.z) * 0.5f;
    g.eye_height = player.pose_parameter_1 * 18.0f + 58.0f;
    g.pose_adjustment = player.pose_parameter_12 * 6.0f;
    const auto second_maximum = engine.maximum_bounds(engine.context, player);
    g.lower = {g.center.x, g.center.y, g.center.z - second_maximum.z * 0.6f};
    g.upper = {g.center.x, g.center.y, (g.origin.z + g.eye_height) - g.pose_adjustment};
}

void finish_player_overlay(ui::RenderQueue& queue) {
    queue.set_alpha(1.0f);
    queue.set_desaturation(false);
}
PreparedPlayer prepare_player(ui::RenderQueue& queue, PlayerRuntimeState& state,
                              const PlayerEspConfig& config, const WorldEspConfig& world,
                              const PlayerRuntimeEngine& engine, const PlayerRuntimeRules& rules,
                              PlayerRuntimeTiming timing, RuntimePlayer* initial_view, i32 slot,
                              bool activation, bool blocked) {
    PreparedPlayer output;
    auto reject_slot = [&] {
        record(state, slot).alpha = 0.0f;
        return output;
    };
    if (!activation || blocked)
        return reject_slot();
    auto* player = engine.by_index(engine.context, slot);
    if (!player || player == initial_view || !engine.is_player(engine.context, *player) ||
        player->eligibility_state != 0)
        return reject_slot();
    const auto index = engine.index(engine.context, *player);
    auto& g = record(state, index);
    auto reject = [&] {
        g.alpha = 0.0f;
        return output;
    };
    if (player->suppressed != 0)
        return reject();
    const bool dormant = engine.dormant(engine.context, *player);
    bool teammate, alive;
    if (dormant) {
        if (!rules.present)
            return reject();
        teammate = !dormant_enemy(initial_view, player, engine, rules);
        alive = dormant_alive(player, engine, rules);
    } else {
        teammate = !live_enemy(initial_view, player, rules);
        alive = player->life_state == 0;
        if (alive) {
            const auto origin = engine.absolute_origin(engine.context, *player);
            update_player_geometry(g, *player, engine, origin);
            const auto time = engine.time(engine.context);
            const auto ticks = float(static_cast<long double>(time) /
                                         static_cast<long double>(timing.tick_interval) +
                                     0.5L);
            g.last_live_tick = truncate(ticks);
        } else
            g.alpha = 0.0f;
        if (player->move_type == 10 && (player->observer_mode == 4 || player->observer_mode == 5)) {
            g.alpha = 0.0f;
            if (world.spectators.value) { // constructor owner +0x27C
                output.stage = PreparedPlayerStage::observer_panel;
                output.player = player;
                output.geometry = &g;
                output.entity_index = index;
                output.cache_state = state.cache.find(std::uint32_t(index));
            }
            return output;
        }
    }
    if (!alive || (!config.teammates.value && teammate))
        return reject();
    float alpha;
    if (dormant) {
        if (g.alpha == 0.0f)
            return output;
        if (!rules.retain_dormant_alpha) {
            const auto speed = config.dormant.value ? 0.25f : 4.0f;
            g.alpha = sse_min(sse_max(g.alpha - timing.frame_time * speed, 0.0f), 1.0f);
            if (g.alpha == 0.0f)
                return output;
        }
        queue.set_desaturation(true);
        alpha = sse_min(0.8f, g.alpha);
    } else {
        alpha = sse_min(
            sse_max(g.alpha + timing.frame_time * std::bit_cast<float>(0x41555555u), 0.1f), 1.0f);
        g.alpha = alpha;
    }
    queue.set_alpha(alpha);
    auto* view = effective_view_player(engine);
    if (!view || view == player || !engine.is_player(engine.context, *view)) {
        g.alpha = 0.0f;
        finish_player_overlay(queue);
        return output;
    }
    output = {PreparedPlayerStage::overlay,          player, view, &g, index, dormant, teammate,
              state.cache.find(std::uint32_t(index))};
    return output;
}

void begin_player_frame(PlayerFrameState& state, const HotkeyBinding& binding, InputBitmap input,
                        float absolute_frame_time) {
    state.transient = 0;
    state.activation = hotkey_active(binding, input);
    state.timer = maximum_number(state.timer - absolute_frame_time, 0.0f);
}

char runtime_bomb_site(const PlayerRuntimeEngine& engine, i32 entity_index,
                       const RuntimeBombSites* sites) { // 433AC842
    if (std::uint32_t(entity_index) & 0xFFFF0000u)
        return 'A';
    auto* entity = engine.by_index(engine.context, entity_index);
    if (!entity || !sites)
        return 'A';
    const auto maximum = engine.maximum_bounds(engine.context, *entity);
    const auto minimum = engine.minimum_bounds(engine.context, *entity);
    const Vector3 center{(maximum.x - minimum.x) * 0.5f + minimum.x,
                         (maximum.y - minimum.y) * 0.5f + minimum.y,
                         (maximum.z - minimum.z) * 0.5f + minimum.z};
    auto distance = [&](Vector3 site) {
        const auto x = center.x - site.x, y = center.y - site.y, z = center.z - site.z;
        return (y * y + x * x) + z * z;
    };
    return distance((*sites)[0]) > distance((*sites)[1]) ? 'B' : 'A';
}
void draw_player_frame_notice(ui::RenderQueue& queue, const PlayerFrameState& state,
                              const WorldEspConfig& world, const PlayerRuntimeEngine& engine,
                              i32 entity_index, const RuntimeBombSites* sites, i32& cursor,
                              const IndicatorLayout& layout, i32 texture) {
    if (!world.bomb.enabled.value || !(state.timer > 0.0f) || entity_index == -1)
        return;
    const auto site = runtime_bomb_site(engine, entity_index, sites);
    const auto fraction = sse_min(sse_max((3.0f - state.timer) / 3.0f, 0.0f), 1.0f);
    draw_indicator(queue, cursor, {&site, 1}, 0xFF69F3FCu, layout, texture, fraction);
}

} // namespace skeet::esp

// ============================================================================
// Subcomponent: projection.cpp
// ============================================================================

namespace skeet::esp {
using namespace detail;

bool project_point(const Projection& p, Vector3 v, Point& output) noexcept { // 0x433F44F5
    const auto& m = p.matrix;
    const float w = ((v.x * m[12] + v.y * m[13]) + v.z * m[14]) + m[15];
    if (0.001f > w)
        return false;
    const float reciprocal = 1.0f / w;
    const float horizontal = ((v.x * m[0] + v.y * m[1]) + v.z * m[2]) + m[3];
    const float vertical = ((v.x * m[4] + v.y * m[5]) + v.z * m[6]) + m[7];
    output.x = _mm_cvtss_si32(_mm_set_ss((reciprocal * horizontal - -1.0f) * p.horizontal_scale));
    output.y = _mm_cvtss_si32(_mm_set_ss((1.0f - vertical * reciprocal) * p.vertical_scale));
    return true;
}

bool player_box(const Projection& p, Vector3 first, Vector3 second,
                Rectangle& output) noexcept { // 0x433404E2
    Point a, b;
    if (!project_point(p, first, a) || !project_point(p, second, b))
        return false;
    const auto delta = sub(b.y, a.y);
    const auto height = delta < 0 ? sub(0, delta) : delta;
    const auto width = height / 2;
    output = {sub(add(b.x, a.x) / 2, width / 2), a.y, width, height};
    return true;
}

float convert_field_of_view(float degrees, float aspect) noexcept { // 0x433ED56D
    const float radians = degrees * std::bit_cast<float>(0x3C8EFA35u);
    const float half = std::tan(radians * 0.5f) * aspect;
    const float converted = std::atan(half) * std::bit_cast<float>(0x42652EE0u);
    return converted + converted;
}

float angle_to_target(Vector3 origin, Vector3 target, Vector3 axis) noexcept { // 0x433B4B55
    const float x = target.x - origin.x, y = target.y - origin.y, z = target.z - origin.z;
    const float length = std::sqrt((x * x + y * y) + z * z);
    if (length == 0.0f)
        return 0.0f;
    const float dot = (y * axis.y + x * axis.x) + z * axis.z;
    return std::acos(dot / length) * std::bit_cast<float>(0x42652EE0u);
}

} // namespace skeet::esp

// ============================================================================
// Subcomponent: runtime_bindings.cpp
// ============================================================================

namespace skeet::esp {
namespace {
void require(bool condition, const char* message) {
    if (!condition)
        throw std::invalid_argument(message);
}
void require_interface(client::ClientRuntimeStorage& storage,
                       const client::EngineRuntimeBindings& engine, std::uint32_t cell) {
    const auto manifest = engine::build_interface_manifest();
    for (std::size_t index = 0; index < manifest.size(); ++index) {
        if (manifest[index].destination_cell != cell)
            continue;
        auto* object = storage.data().native_cell<void*>(cell);
        require(object && object == engine.interfaces.objects[index],
                "ESP requires the genuine interface from its canonical engine bundle");
        return;
    }
    throw std::logic_error("ESP interface is absent from the build acquisition manifest");
}
void require_storage(client::ClientRuntimeStorage& storage,
                     const client::EngineRuntimeBindings& engine) {
    auto& data = storage.data();
    require(&engine.variables.data() == &data && storage.offsets().words &&
                !storage.offsets().globals.pointers.empty(),
            "ESP requires initialized canonical engine variables and entity offsets");
    const auto* globals = data.native_cell<const std::byte*>(0x43467B04);
    require(globals && reinterpret_cast<std::uintptr_t>(globals) % 4 == 0 &&
                reinterpret_cast<const std::byte*>(data.native_cell<const float*>(0x4346A334)) ==
                    globals &&
                reinterpret_cast<const std::byte*>(data.native_cell<const float*>(0x43467EB8)) ==
                    globals + 0x10 &&
                reinterpret_cast<const std::byte*>(data.native_cell<const float*>(0x4346A338)) ==
                    globals + 0x20 &&
                reinterpret_cast<const std::byte*>(
                    data.native_cell<const std::int32_t*>(0x43469300)) == globals + 0x18,
            "ESP requires genuine live engine clock and maximum-player aliases");
}
}
PlayerEngineBindings native_player_engine_bindings(client::ClientRuntimeStorage& storage,
                                                   const client::EngineRuntimeBindings& engine) {
    require_storage(storage, engine);
    require_interface(storage, engine, 0x4346A890);
    require_interface(storage, engine, 0x4346A514);
    auto& data = storage.data();
    require(data.native_cell<void*>(0x43468FF0) && data.native_cell<void*>(0x4346A3A4) &&
                data.native_cell<const std::int32_t*>(0x43469060) &&
                data.native_cell<PlayerSequenceActivity>(0x4346A364),
            "ESP requires the recovered resource, game-rules, bone-counter and sequence producers");
    for (const auto cell : {0x43467A8Cu, 0x4346A694u, 0x4346A568u})
        require(engine.variables.at(cell).get() != nullptr,
                "ESP requires its acquired game-mode ConVars");
    return {data.native_cell<void*>(0x4346A890),
            data.native_cell<void*>(0x4346A514),
            data.native_cell<void*>(0x43475808),
            data.native_cell<void**>(0x43468FF0),
            data.native_cell<void**>(0x4346A3A4),
            engine.variables.at(0x43467A8C),
            engine.variables.at(0x4346A694),
            engine.variables.at(0x4346A568),
            data.native_cell<const std::int32_t*>(0x43469060),
            data.native_cell<const float*>(0x43467EB8),
            data.native_cell<const void*>(0x43467B04),
            data.native_cell<const float*>(0x4346A338),
            data.native_cell<Vector3>(0x43479CCC),
            data.native_cell<float>(0x43479CD8),
            data.native_cell<float>(0x43479CDC),
            data.native_cell<const float*>(0x4346A334),
            data.native_cell<std::uint8_t>(0x43468E58),
            data.native_cell<std::uint8_t>(0x43467224),
            data.native_cell<std::int32_t>(0x4346DB1C),
            data.native_cell<std::uint8_t>(0x4347CA30),
            data.native_cell<PlayerSequenceActivity>(0x4346A364),
            data.native_cell<const std::int32_t*>(0x43469300),
            storage.offsets()};
}
SoundOverlayBindings native_sound_overlay_bindings(client::ClientRuntimeStorage& storage,
                                                   const client::EngineRuntimeBindings& engine,
                                                   engine::SceneQueryRuntimeOwner& scene) {
    require_storage(storage, engine);
    require(&scene.storage() == &storage, "ESP sound must share the canonical scene query owner");
    // These acquisitions are mandatory for this supported build. Their native
    // call sites still preserve their nullable-interface guards on later reads.
    require_interface(storage, engine, 0x43468260);
    auto& data = storage.data();
    require(data.native_cell<void*>(0x434679EC) != nullptr,
            "ESP sound requires the acquired genuine C_TempEntsSystem inline object");
    return {scene.queries().movement(), data.native_cell<std::uint8_t*>(0x434683D0),
            data.native_cell<void*>(0x43468260), data.native_cell<void*>(0x434679EC)};
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: runtime_owner.cpp
// ============================================================================

namespace skeet::esp {
namespace {
template <class T> T& pointed(engine::NativeDataImage& data, std::uint32_t identity) {
    auto* pointer = data.native_cell<T*>(identity);
    if (!pointer)
        throw std::invalid_argument("ESP requires the actual initialized engine alias");
    return *pointer;
}
std::ptrdiff_t offset(client::ClientRuntimeStorage& storage, std::uint32_t identity) {
    const auto& globals = storage.offsets().globals;
    for (const auto& row : engine::build_receive_offset_recipes())
        if (row.pointer_identity == identity)
            return engine::DecodedOffset(globals, identity, row.xor_key_identity,
                                         row.initial_xor_key, 0);
    for (const auto& row : engine::build_data_map_offset_recipes())
        if (row.pointer_identity == identity)
            return engine::DecodedOffset(globals, identity, row.xor_key_identity,
                                         row.initial_xor_key, 0);
    throw std::invalid_argument("ESP field has no proven receive/datamap offset recipe");
}
engine::OwnerRegistryBinding registry(client::ClientRuntimeStorage& storage,
                                      std::uint32_t identity) {
    return {storage.owner_registries().at(identity), storage.memory()};
}
client::ClientRuntimeStorage&
checked(client::ClientRuntimeStorage& storage, const client::EngineRuntimeBindings& engine,
        client::ConfigurationRuntimeOwner& configuration, client::PlayerRuntimeOwner& players,
        engine::SceneQueryRuntimeOwner& scene, client::VisualRuntimeOwner& visual,
        client::CrtRuntimeOwner& crt, script::SourceScriptRuntimeOwner& scripts,
        ui::RendererAssets& assets, engine::BuildEspBindings native) {
    auto& data = storage.data();
    if (&engine.variables.data() != &data || &native.data != &data ||
        &configuration.storage() != &storage || &players.storage() != &storage ||
        &scene.storage() != &storage || &scene.players() != &players ||
        &visual.storage() != &storage || &crt.storage() != &storage ||
        &scripts.storage() != &storage || &assets.storage() != &storage ||
        &assets.startup_renderer() != &visual.startup_renderer())
        throw std::invalid_argument(
            "ESP must share the canonical engine, player, scene, visual, CRT and script owners");
    if (data.native_cell<EspOwnerRuntime*>(0x434758DC))
        throw std::logic_error(
            "ESP bindings must be composed before the canonical feature factory");
    for (const auto cell :
         {0x4346A31Cu, 0x43468B9Cu, 0x43467198u, 0x43467470u, 0x4346A898u, 0x43467B54u, 0x434758B4u,
          0x43475B90u, 0x43468D4Cu, 0x434678F8u, 0x43475A60u})
        if (!data.native_cell<void*>(cell))
            throw std::invalid_argument(
                "ESP requires its genuine interface, icon, clock and packet producers");
    for (const auto cell : {0x43467C3Cu, 0x43467A8Cu, 0x4346A568u, 0x4346A850u, 0x43468520u})
        if (!engine.variables.at(cell).get())
            throw std::invalid_argument("ESP requires the actual acquired encoded ConVar");
    return storage;
}
std::int32_t dropped_capacity(void*, const WorldEntity& entity) {
    return engine::invoke<std::int32_t>(reinterpret_cast<void*>(entity.identity), 375);
}
PacketTracerOffsets packet_fields(client::ClientRuntimeStorage& storage) {
    return {{offset(storage, 0x43467EC0), offset(storage, 0x43468910)},
            offset(storage, 0x4346A45C),
            offset(storage, 0x434693FC),
            offset(storage, 0x43467034),
            offset(storage, 0x43468B28),
            offset(storage, 0x43467480),
            offset(storage, 0x43468E90),
            offset(storage, 0x43468FAC)};
}
TabletUpgradeOffsets tablet_fields(client::ClientRuntimeStorage& storage) {
    return {offset(storage, 0x43467A74), offset(storage, 0x4346908C), offset(storage, 0x43467E74),
            offset(storage, 0x43467000), offset(storage, 0x434693B0), offset(storage, 0x4346A770)};
}
}
struct EspRuntimeOwner::Data {
    client::ClientRuntimeStorage& storage;
    EspFrameOwner state;
    PlayerEngineBindings players;
    WeaponTextBindings words;
    WeaponIconProvider icons;
    WeaponTextProvider names;
    PlayerDrawContext draw;
    SoundOverlayBindings sound;
    effects::GlowEngine glow_engine;
    effects::GlowEntityEngine glow_entities;
    effects::GlowRuntime glow;
    TabletUpgradeBindings tablet;
    LineOfSightBindings sight;
    anti_aim::RawTargetTraceEntities trace_entities;
    rage::ShotgunPattern pattern;
    PacketTracerBindings packet;
    events::DeathBindings death;
    EspOwnerBindings bindings;

    Data(client::ClientRuntimeStorage& source, const client::EngineRuntimeBindings& engine,
         client::ConfigurationRuntimeOwner& configuration, client::PlayerRuntimeOwner& player_state,
         engine::SceneQueryRuntimeOwner& scene, client::VisualRuntimeOwner& visual,
         client::CrtRuntimeOwner& crt, script::SourceScriptRuntimeOwner& scripts,
         ui::RendererAssets& assets, engine::BuildEspBindings native)
        : storage(checked(source, engine, configuration, player_state, scene, visual, crt, scripts,
                          assets, native)),
          state(player_state.cache(), source.data().native_cell<std::uint8_t>(0x4346A65C)),
          players(native_player_engine_bindings(source, engine)),
          words{source.data().native_cell<void*>(0x43476424),
                source.data().native_cell<WeaponTextBindings::ItemSystemFactory>(0x43468B9C),
                source.data().native_cell<void*>(0x4346A31C)},
          icons(assets.icon_cache().weapon_provider()), names(weapon_text_provider(words)),
          draw{visual.projection(),
               player_state.cache(),
               source.data().native_cell<float>(0x4346D548),
               configuration.player_esp_settings().money.value,
               crt.esp_flags(),
               icons,
               names,
               &scripts.bridge(),
               &scripts.flags()},
          sound(native_sound_overlay_bindings(source, engine, scene)),
          glow_engine(native.glow_engine()),
          glow_entities({source.data().native_cell<void*>(0x4346A890),
                         source.data().native_cell<void*>(0x43475808),
                         source.data().native_cell<std::int32_t*>(0x43469300),
                         source.data().native_cell<void*>(0x434688C0),
                         source.data().native_cell<std::uint8_t>(0x43468E58),
                         source.data().native_cell<std::uint8_t>(0x4346A65C),
                         engine.variables.at(0x43467A8C), engine.variables.at(0x4346A568),
                         player_state.adjustments(), source.offsets()}),
          glow(configuration.player_esp_settings().player, configuration.other_esp_settings().world,
               glow_engine, glow_entities),
          tablet{configuration.other_esp_settings().world,
                 source.data().native_cell<void*>(0x43475808),
                 source.data().native_cell<void*>(0x4346A890),
                 source.data().native_cell<std::int32_t*>(0x434678F8),
                 source.data().native_cell<float*>(0x43467EB8),
                 tablet_fields(source)},
          sight{source.data().native_cell<void*>(0x4346A514),
                source.data().native_cell<void*>(0x43468260),
                engine.variables.at(0x4346A850),
                source.data().native_cell<std::int32_t*>(0x43469300),
                source.data().native_cell<std::uint8_t*>(0x434683D0),
                scene.queries().movement(),
                crt.trigonometry(),
                offset(source, 0x434679F8),
                offset(source, 0x43468EA0)},
          trace_entities(source.data().native_cell<void*>(0x4346A890),
                         static_cast<std::uint32_t>(offset(source, 0x434683C8)),
                         static_cast<std::uint32_t>(offset(source, 0x43469344) - 0x18)),
          pattern(native.shotgun_pattern()),
          packet{packet_fields(source),
                 crt.spread().tables(),
                 crt.random(),
                 pattern,
                 crt.trigonometry(),
                 engine.variables.at(0x43468520),
                 source.data().native_cell<void*>(0x43467B54),
                 scene.queries().ballistics(),
                 source.data().native_cell<void* const*>(0x43475B90),
                 trace_entities,
                 source.data().native_cell<const float*>(0x43467EB8)},
          death{source.data().native_cell<void*>(0x4346A514),
                source.data().native_cell<void*>(0x4346A890),
                source.data().native_cell<void*>(0x43475808)},
          bindings{{registry(source, 0x43475B78), registry(source, 0x43475A9C),
                    source.entity_observers(), registry(source, 0x43475988),
                    registry(source, 0x4347584C), registry(source, 0x43475AD8),
                    registry(source, 0x43475770), source.event_registry(), source.configuration()},
                   state,
                   configuration.player_esp_settings().player,
                   configuration.other_esp_settings().world,
                   configuration.effects_settings(),
                   {configuration.player_esp_settings().player,
                    configuration.other_esp_settings().world,
                    configuration.other_esp_settings().grenades,
                    draw,
                    {icons, names, nullptr, dropped_capacity},
                    &glow_engine,
                    visual.indicators(),
                    source.input_runtime().bitmap(),
                    player_state.adjustments()},
                   players,
                   {scene.queries(), source.data().native_cell<void**>(0x4346A3A4),
                    source.data().native_cell<void*>(0x434688C0), engine.variables.at(0x43467C3C),
                    engine.variables.at(0x43467A8C),
                    pointed<std::int32_t>(source.data(), 0x43469300),
                    pointed<float>(source.data(), 0x43467EB8),
                    source.data().native_cell<std::int32_t>(0x43467DA8),
                    source.data().native_cell<std::int32_t*>(0x43475A60)[1],
                    pointed<float>(source.data(), 0x4346A338),
                    source.data().native_cell<Vector3>(0x43479CCC),
                    source.data().native_cell<std::uint8_t>(0x43468E58),
                    WorldEngineOffsets{source.offsets().globals}},
                   {configuration.player_esp_settings().player, sound, player_state.adjustments(),
                    state.frame},
                   tablet,
                   glow,
                   {configuration.player_esp_settings().player, player_state.adjustments(),
                    state.frame, source.data().native_cell<std::uint8_t>(0x43468E58), sight},
                   packet,
                   death,
                   source.data().native_cell<const void*>(0x43467B04),
                   source.data().native_cell<const float*>(0x4346A338),
                   source.data().native_cell<std::int32_t*>(0x43468D4C),
                   source.data().native_cell<void*>(0x43467198),
                   source.data().native_cell<void*>(0x43467470),
                   source.data().native_cell<void*>(0x4346A898)} {}
};
EspRuntimeOwner::EspRuntimeOwner(client::ClientRuntimeStorage& storage,
                                 const client::EngineRuntimeBindings& engine,
                                 client::ConfigurationRuntimeOwner& configuration,
                                 client::PlayerRuntimeOwner& players,
                                 engine::SceneQueryRuntimeOwner& scene,
                                 client::VisualRuntimeOwner& visual, client::CrtRuntimeOwner& crt,
                                 script::SourceScriptRuntimeOwner& scripts,
                                 ui::RendererAssets& assets, engine::BuildEspBindings native)
    : data_(std::make_unique<Data>(storage, engine, configuration, players, scene, visual, crt,
                                   scripts, assets, native)) {}
EspRuntimeOwner::~EspRuntimeOwner() = default;
client::ClientRuntimeStorage& EspRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
EspOwnerBindings& EspRuntimeOwner::factory_bindings() noexcept {
    return data_->bindings;
}
EspOwnerRuntime*& EspRuntimeOwner::active() const noexcept {
    return data_->storage.data().native_cell<EspOwnerRuntime*>(0x434758DC);
}
EspFrameOwner& EspRuntimeOwner::state() noexcept {
    return data_->state;
}
effects::GlowRuntime& EspRuntimeOwner::glow() noexcept {
    return data_->glow;
}
const rage::ShotgunPattern& EspRuntimeOwner::shotgun_pattern() const noexcept {
    return data_->pattern;
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: skeleton.cpp
// ============================================================================

namespace skeet::esp {

void draw_skeleton(ui::RenderQueue& queue, const Projection& projection,
                   std::span<const SkeletonBone> bones, std::span<const Matrix3x4> transforms,
                   std::uint32_t color) {
    const auto count = std::min(bones.size(), transforms.size());
    if (count > 256)
        throw std::length_error("Skeleton exceeds native bone workspace");
    std::array<Point, 256> points;
    std::array<std::uint32_t, 8> projected{};
    for (std::size_t index = 0; index < count; ++index) {
        const auto& bone = bones[index];
        if (!(bone.flags & 0x100))
            continue;
        const auto& matrix = transforms[index];
        if (!project_point(projection, {matrix.rows[0][3], matrix.rows[1][3], matrix.rows[2][3]},
                           points[index]))
            continue;
        projected[index >> 5] |= std::uint32_t(1) << (index & 31);
        if (bone.parent == 0xFFFFFFFFu)
            continue;
        if (bone.parent >= points.size())
            throw std::out_of_range("Skeleton parent exceeds native bone workspace");
        if (!(projected[bone.parent >> 5] & (std::uint32_t(1) << (bone.parent & 31))))
            continue;
        const auto a = points[bone.parent], b = points[index];
        queue.line({a.x, a.y, b.x, b.y, color});
    }
}

} // namespace skeet::esp

// ============================================================================
// Subcomponent: sound_overlay.cpp
// ============================================================================

namespace skeet::esp {
using engine::invoke;
namespace {
/* [removed duplicate field] */
/* [removed duplicate field] */
Vector3 vector(MovementVector value) {
    return {value.x, value.y, value.z};
}
std::int32_t clock_tick(const SoundOverlayContext& c) {
    const auto time = c.players.time();
    return detail::truncate(
        float(static_cast<long double>(time) / static_cast<long double>(c.players.tick_interval()) +
              0.5L));
}

// The four recipient queries are the concrete 4346F198 filter: reliable,
// not initialization, one recipient, actual engine local-player index.
#if defined(_M_IX86) || defined(__i386__)
#define SOUND_MEMBER __fastcall
#define SOUND_EDX , void*
#else
#define SOUND_MEMBER
#define SOUND_EDX
#endif
struct Recipient {
    const void* const* table;
    EnginePlayers* players;
    static void* SOUND_MEMBER destroy(void* self SOUND_EDX, std::uint32_t) {
        return self;
    }
    static bool SOUND_MEMBER reliable(void* SOUND_EDX) {
        return true;
    }
    static bool SOUND_MEMBER initialization(void* SOUND_EDX) {
        return false;
    }
    static std::int32_t SOUND_MEMBER count(void* SOUND_EDX) {
        return 1;
    }
    static std::int32_t SOUND_MEMBER index(void* self SOUND_EDX, std::int32_t) {
        return static_cast<Recipient*>(self)->players->local_index();
    }
};
const void* recipient_table[] = {
    reinterpret_cast<const void*>(&Recipient::destroy),
    reinterpret_cast<const void*>(&Recipient::reliable),
    reinterpret_cast<const void*>(&Recipient::initialization),
    reinterpret_cast<const void*>(&Recipient::count),
    reinterpret_cast<const void*>(&Recipient::index),
};
#undef SOUND_MEMBER
#undef SOUND_EDX

void debug_circle(Vector3 center, std::uint32_t color, float duration, void* overlay) {
    // Complete fixed-angle 433EBD74 -> 433F0D0C path used by SoundESP.
    // Pitch=-90, yaw=roll=0 gives the same native up and negative-right axes.
    constexpr float radians = 0.01745329238474369f;
    const Vector3 up{std::sin(-90.0f * radians), 0.0f, std::cos(-90.0f * radians)};
    const Vector3 negative_right{0.0f, 1.0f, 0.0f};
    auto point = center;
    point.x += up.x * 15.0f;
    point.y += up.y * 15.0f;
    point.z += up.z * 15.0f;
    const auto first = point;
    const auto red = std::int32_t(color & 255u);
    const auto green = std::int32_t((color >> 8) & 255u);
    const auto blue = std::int32_t((color >> 16) & 255u);
    const auto alpha = std::int32_t(color >> 24);
    for (std::int32_t step = 1; step <= 32; ++step) {
        const auto previous = point;
        const auto angle = float(step) * 0.19634954631328583f;
        const auto cosine = std::cos(angle), sine = std::sin(angle);
        point = {(up.x * cosine) * 15.0f + center.x + (negative_right.x * sine) * 15.0f,
                 (up.y * cosine) * 15.0f + center.y + (negative_right.y * sine) * 15.0f,
                 (up.z * cosine) * 15.0f + center.z + (negative_right.z * sine) * 15.0f};
        // The native line branch explicitly replaces configured alpha by255.
        if (overlay)
            invoke<void>(overlay, 20, &previous, &point, red, green, blue, std::int32_t(255), true,
                         duration);
        if (alpha != 0 && step > 1)
            invoke<void>(overlay, 3, &first, &previous, &point, red, green, blue, alpha, true,
                         duration);
    }
}

std::int32_t first_sound(const ActiveSoundTree& tree) {
    auto index = tree.root;
    while (index != -1 && tree.nodes[index].left != -1)
        index = tree.nodes[index].left;
    return index;
}
std::int32_t next_sound(const ActiveSoundTree& tree, std::int32_t index) {
    // Complete 433BFEC6, including its allocated/free-node admission checks.
    if (std::uint32_t(index) >= tree.allocation_count || index > tree.last_allocated ||
        tree.nodes[index].left == index)
        return -1;
    if (tree.nodes[index].right != -1) {
        index = tree.nodes[index].right;
        while (tree.nodes[index].left != -1)
            index = tree.nodes[index].left;
        return index;
    }
    auto parent = tree.nodes[index].parent;
    while (parent != -1 && tree.nodes[parent].right == index) {
        index = parent;
        parent = tree.nodes[index].parent;
    }
    return parent;
}
}

void SoundOverlay::initialize_model(void* tables) {
    void* models = nullptr;
    const auto count = invoke<std::int32_t>(tables, 5);
    for (std::int32_t index = 0; index < count; ++index) {
        auto* table = invoke<void*>(tables, 4, index);
        if (!table)
            continue;
        const auto* name = invoke<const char*>(table, 1);
        if (name && events::event_hash(name) == 0x9fe6b136u) {
            models = table;
            break;
        }
    }
    if (!models)
        return;
    initialize_model_table(models);
}
void SoundOverlay::initialize_model_table(void* models) {
    for (const auto wanted : {0x69a49bb6u, 0xf458a8fau, 0x855fee75u}) {
        std::int32_t index = 0;
        auto strings = invoke<std::int32_t>(models, 3);
        for (; index < strings; strings = invoke<std::int32_t>(models, 3)) {
            const auto* name = invoke<const char*>(models, 9, index);
            if (name && events::event_hash(name) == wanted) {
                ring_model = index;
                return;
            }
            ++index;
        }
        ring_model = -1;
    }
}

void SoundOverlay::synchronize_listeners(bool refresh, const PlayerEspConfig& player,
                                         const WorldEspConfig& world,
                                         std::array<events::EventListener, 3> listeners,
                                         events::EventRegistry& registry) {
    if (!refresh)
        return;
    const std::array<std::uint8_t, 3> requested{std::uint8_t(player.dormant.value),
                                                std::uint8_t(world.bomb.enabled.value),
                                                std::uint8_t(world.bomb.enabled.value)};
    for (std::size_t index = 0; index < listeners.size(); ++index) {
        if (requested[index] == listener_enabled[index])
            continue;
        listener_enabled[index] = requested[index];
        if (requested[index])
            registry.add(listeners[index]);
        else
            registry.remove(listeners[index]);
    }
}

void SoundOverlay::consume(RuntimePlayer* view, const ActiveSound& sound,
                           const SoundOverlayContext& c) {
    auto source = sound.source_index;
    if (source <= 0)
        return;
    RuntimePlayer* player = nullptr;
    if (source <= 64)
        player = c.players.by_index(source);
    else {
        RuntimePlayer* entity = nullptr;
        if ((std::uint32_t(source) & 0xffff0000u) == 0)
            entity = c.players.by_index(source);
        if (entity && !c.players.dormant(*entity)) {
            if (!c.players.is_weapon(*entity))
                return;
            const auto definition = c.players.item_definition(*entity);
            const auto owner = c.players.owner_handle(*entity);
            if (owner == -1 || (definition >= 43 && definition <= 48))
                return;
            player = c.players.by_handle(owner);
        } else {
            for (std::uint32_t index = 0; index < c.pickups.count; ++index) {
                const auto& pickup = c.pickups.entries[index];
                if (std::int32_t(pickup.item_index) != source)
                    continue;
                player = c.players.by_index(pickup.player_index);
                break;
            }
        }
    }
    if (!player || !c.players.is_player(*player) || c.players.eligibility(*player))
        return;
    const auto index = c.players.index(*player);
    auto* cached = c.state.cache.get(std::uint32_t(index));
    if (c.players.resource() && c.players.dormant(*player)) {
        if (!c.config.dormant.value || !dormant_alive(player, c.players))
            return;
    } else if (!c.config.visualize_sounds.value || c.players.life(*player) != 0)
        return;
    if (!dormant_enemy(view, player, c.players) ||
        c.player_adjustments.records[std::uint32_t(index)].adjustments.disable_visuals)
        return;

    MovementTraceFlagGuard guard(c.bindings.trace_flag);
    // Actual native ground probe, including its start offset and thin hull.
    const auto raised_z = sound.origin.z + 10.0f;
    const MovementVector delta{sound.origin.x - sound.origin.x, sound.origin.y - sound.origin.y,
                               (sound.origin.z - 72.0f) - raised_z};
    const auto swept = (delta.y * delta.y + delta.x * delta.x) + delta.z * delta.z != 0.0f;
    const MovementCollisionRay ray{
        {sound.origin.x - 16.0f, sound.origin.y - 16.0f, raised_z + 1.0f},
        delta,
        {16.0f, 16.0f, -1.0f},
        {0.0f, 0.0f, 1.0f},
        false,
        swept};
    MovementCollisionTrace hit{};
    c.bindings.collision.trace_ray(
        ray, 1u,
        {MovementCollisionFilterKind::skip_player,
         std::uint32_t(reinterpret_cast<std::uintptr_t>(player->engine_entity)), 0, 0},
        hit);
    if (hit.all_solid || hit.start_solid || !(hit.fraction < 1.0f))
        return;
    auto position = vector(hit.end);
    if (c.config.dormant.value && c.players.dormant(*player)) {
        if (cached->geometry.last_live_tick < clock_tick(c)) {
            update_player_geometry(cached->geometry, *player, c.players, position);
            cached->geometry.alpha = 1.0f;
        }
    }
    if (!c.config.visualize_sounds.value || !c.frame.activation || c.players.suspended())
        return;
    position.z += 6.0f;
    const auto color = c.config.sound_color.value;
    if (ring_model <= 0) {
        const auto interval = c.players.simulation_tick_interval();
        debug_circle(position, color, interval + interval, c.bindings.debug_overlay);
        return;
    }
    const auto elapsed = c.players.animation_time() - field<float>(&cached->native, 0x158);
    // COMISS/JAE suppresses ordered <=0.5; unordered elapsed values pass.
    if (0.5f >= elapsed)
        return;
    auto count = std::int32_t((color >> 24) / 80u) + 1;
    float delay = 0.0f;
    for (; count != 0; --count) {
        Recipient recipient{recipient_table, &c.players};
        if (c.bindings.temp_entities)
            invoke<void>(c.bindings.temp_entities, 9, &recipient, delay, &position, 12.0f, 800.0f,
                         ring_model, std::int32_t(0), std::int32_t(0), std::int32_t(2), 0.75f, 8.0f,
                         std::int32_t(50), 0.0f, std::int32_t(color & 255u),
                         std::int32_t((color >> 8) & 255u), std::int32_t((color >> 16) & 255u),
                         std::int32_t(color >> 24), std::int32_t(0), std::int32_t(8));
        delay += 0.05f;
    }
    field(&cached->native, 0x1b0, position);
    field(&cached->native, 0x158, c.players.animation_time());
}

void process_sound_overlay(RuntimePlayer* view, SoundOverlay& owner,
                           const ActiveSoundTree* const& source, const SoundOverlayContext& c) {
    if (!view || !source || c.players.game_type() == 6 ||
        (!c.config.visualize_sounds.value && !c.config.dormant.value))
        return;
    auto index = first_sound(*source);
    while (index != -1) {
        owner.consume(view, source->nodes[index].value, c);
        index = next_sound(*source, index);
    }
}

void expire_pickup_observations(PickupObservations& observations, std::int32_t frame,
                                float interval) noexcept {
    const auto age = detail::truncate(1.0f / interval + 0.5f);
    const auto threshold = std::bit_cast<std::int32_t>(std::uint32_t(frame) - std::uint32_t(age));
    while (observations.count != 0) {
        if (threshold < observations.entries[observations.count - 1].frame)
            return;
        --observations.count;
    }
}

SoundHistoryReceiver::SoundHistoryReceiver(client::EntityObserverRegistry& registry,
                                           PickupObservations& observations,
                                           const void* const& globals, const float* const& interval)
    : observations_(observations), globals_(globals), interval_(interval) {
    registry.append(*this); //4348265A /4333BF32; table remains unpublished
}
void SoundHistoryReceiver::process_incoming() {
    const auto interval = *interval_;
    const auto frame = field<std::int32_t>(globals_, 0x1c);
    expire_pickup_observations(observations_, frame, interval);
}

void update_dormant_radar(const void* packet, DormantRadarOffsets offsets,
                          const SoundOverlayContext& c) {
    if (!c.config.dormant.value || !packet)
        return;
    const auto raw_index = field<std::uint32_t>(packet, offsets.zero_based_index);
    const auto index = std::bit_cast<std::int32_t>(raw_index + 1u);
    if (index > 64)
        return;
    auto* player = c.players.by_index(index);
    if (!player || !c.players.is_player(*player) || !c.players.dormant(*player))
        return;
    auto* cached = c.state.cache.get(std::uint32_t(index));
    const auto origin = field<Vector3>(packet, offsets.origin);
    MovementTraceFlagGuard guard(c.bindings.trace_flag);
    const MovementVector delta{origin.x - origin.x, origin.y - origin.y,
                               (origin.z - 72.0f) - origin.z};
    const auto swept = (delta.y * delta.y + delta.x * delta.x) + delta.z * delta.z != 0.0f;
    const MovementCollisionRay ray{{origin.x - 16.0f, origin.y - 16.0f, origin.z + 24.0f},
                                   delta,
                                   {16.0f, 16.0f, -24.0f},
                                   {0.0f, 0.0f, 24.0f},
                                   false,
                                   swept};
    MovementCollisionTrace hit{};
    trace_movement_world(c.bindings.collision, ray, 0x1400bu, hit, c.bindings.trace_flag);
    if (hit.start_solid || hit.all_solid)
        return;
    const auto position = vector(hit.end);
    const auto dx = position.x - cached->geometry.origin.x;
    const auto dy = position.y - cached->geometry.origin.y;
    const auto dz = position.z - cached->geometry.origin.z;
    if (!((dy * dy + dx * dx) + dz * dz > 16.0f))
        return;
    update_player_geometry(cached->geometry, *player, c.players, position);
    cached->geometry.alpha = 1.0f;
    cached->geometry.last_live_tick = clock_tick(c);
}

} // namespace skeet::esp

// ============================================================================
// Subcomponent: tablet_upgrade.cpp
// ============================================================================

#pragma STDC FENV_ACCESS ON

namespace skeet::esp {
namespace {
/* [removed duplicate field] */
/* [removed duplicate field] */
void* tablet(const TabletUpgradeBindings& b) {
    auto* local = b.local_player;
    if (!local)
        return nullptr;
    const auto handle = field<std::int32_t>(local, b.offsets.active_weapon);
    if (handle == -1)
        return nullptr;
    auto* weapon = engine::invoke<void*>(b.entity_list, 4, handle);
    return weapon && field<std::uint16_t>(weapon, b.offsets.definition) == 72 ? weapon : nullptr;
}
}
void prepare_tablet_upgrade(TabletUpgradeState& state, const TabletUpgradeBindings& b) {
    if (!b.settings.upgrade_tablet.value)
        return;
    auto* weapon = tablet(b);
    if (!weapon)
        return;
    state.status = field<std::uint8_t>(weapon, b.offsets.status);
    state.value_bits = field<decltype(state.value_bits)>(weapon, b.offsets.values);
    state.identifiers = field<decltype(state.identifiers)>(weapon, b.offsets.identifiers);
    state.time_bits = field<decltype(state.time_bits)>(weapon, b.offsets.times);
    const auto frame = *b.frame_count;
    const auto* time = b.current_time;
    state.frame = frame;
    field(weapon, b.offsets.status, std::uint8_t{0});
    for (std::int32_t desired = 1; desired <= 3; ++desired) {
        field(weapon, b.offsets.values + (desired - 1) * 4, std::uint32_t{0xBF800000u});
        std::int32_t slot = 0;
        for (; slot < 8; ++slot)
            if (field<std::int32_t>(weapon, b.offsets.identifiers + slot * 4) == desired)
                break;
        if (slot == 8) {
            for (slot = 0; slot < 8; ++slot)
                if (field<std::int32_t>(weapon, b.offsets.identifiers + slot * 4) == 0)
                    break;
        }
        if (slot == 8)
            continue;
        field(weapon, b.offsets.identifiers + slot * 4, desired);
        field(weapon, b.offsets.times + slot * 4, *time - 1.100000023841858f);
    }
}
void restore_tablet_upgrade(TabletUpgradeState& state, const TabletUpgradeBindings& b) {
    if (!b.settings.upgrade_tablet.value || state.frame != *b.frame_count)
        return;
    state.frame = -1;
    auto* weapon = tablet(b);
    if (!weapon)
        return;
    field(weapon, b.offsets.status, state.status);
    field(weapon, b.offsets.values, state.value_bits);
    field(weapon, b.offsets.identifiers, state.identifiers);
    field(weapon, b.offsets.times, state.time_bits);
}

PlayerEspUpdateHandler::PlayerEspUpdateHandler(engine::OwnerRegistryBinding registry,
                                               TabletUpgradeState& state,
                                               const TabletUpgradeBindings& bindings,
                                               effects::GlowRuntime& glow)
    : state_(state), bindings_(bindings), glow_(glow) {
    network::register_update_observer(registry, *this);
    vtable = &entries_;
}
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_ESP_MEMBER __thiscall
#define SKEET_ESP_FAST __fastcall
#else
#define SKEET_ESP_MEMBER
#define SKEET_ESP_FAST
#endif
void SKEET_ESP_MEMBER PlayerEspUpdateHandler::prepare(void* object) {
    auto& self =
        *static_cast<PlayerEspUpdateHandler*>(static_cast<network::UpdateObserver*>(object));
    prepare_tablet_upgrade(self.state_, self.bindings_);
}
void SKEET_ESP_MEMBER PlayerEspUpdateHandler::restore(void* object) {
    auto& self =
        *static_cast<PlayerEspUpdateHandler*>(static_cast<network::UpdateObserver*>(object));
    restore_tablet_upgrade(self.state_, self.bindings_);
}
void SKEET_ESP_MEMBER PlayerEspUpdateHandler::glow(void* object) {
    static_cast<PlayerEspUpdateHandler*>(static_cast<network::UpdateObserver*>(object))
        ->glow_.update();
}
void SKEET_ESP_MEMBER PlayerEspUpdateHandler::unchanged(void*) {} //433CCE8E
void SKEET_ESP_FAST PlayerEspUpdateHandler::unchanged_player(void*, void*, std::uintptr_t,
                                                             std::uintptr_t, std::uintptr_t) {
} //4337D653
void SKEET_ESP_FAST PlayerEspUpdateHandler::unchanged_view(void*, float*, float*, float*) {
} //4334A269
void SKEET_ESP_FAST PlayerEspUpdateHandler::unchanged_post(void*, void*) {} //433CCE8E
#undef SKEET_ESP_MEMBER
#undef SKEET_ESP_FAST
} // namespace skeet::esp

// ============================================================================
// Subcomponent: user_message_handlers.cpp
// ============================================================================

#pragma STDC FENV_ACCESS ON

namespace skeet::esp {
namespace {
/* [removed duplicate field] */
const void* pointer32(const void* object, std::ptrdiff_t offset) {
    return reinterpret_cast<const void*>(std::uintptr_t(field<std::uint32_t>(object, offset)));
}
std::int32_t clock_result(float time, float interval) {
    return detail::truncate(
        float(static_cast<long double>(time) / static_cast<long double>(interval) + 0.5L));
}
float integer_float(std::int32_t value) {
    return _mm_cvtss_f32(_mm_cvtepi32_ps(_mm_cvtsi32_si128(value)));
}
UserMessageHandlers* handlers;
}

UserMessageHandlers::UserMessageHandlers(engine::PreservedInitialization& initialization,
                                         UserMessageBindings bindings)
    : UserMessageHandlers(initialization.image_owner(), bindings) {}
UserMessageHandlers::UserMessageHandlers(std::shared_ptr<engine::NativeDataImage> image,
                                         UserMessageBindings bindings)
    : image_(image), bindings_(bindings) {
    (void)engine::require_native_data_image(image_);
    if (&bindings_.active_owner != &image_->native_cell<EspOwnerRuntime*>(0x434758dc))
        throw std::invalid_argument("User messages require the canonical active ESP owner cell");
    if (&bindings_.recording_enabled != &image_->native_cell<std::uint8_t>(0x4346754c))
        throw std::invalid_argument("User messages require the canonical recording flag cell");
    const auto* correction =
        bindings_.adjustments.settings_base() + offsetof(players::Adjustments, correction_active);
    if (correction != &image_->native_cell<std::uint8_t>(0x4347be0e))
        throw std::invalid_argument("User messages require the canonical player correction bytes");
}

void UserMessageHandlers::weapon_sound(const void* packet) {
    auto* owner = bindings_.active_owner;
    if (!owner || !owner->player_config().dormant.value)
        return;
    if (!(field<std::uint8_t>(packet, 0x24) & 1))
        return;
    const auto present = field<std::uint32_t>(packet, 0x24);
    if (!(present & 2) || !(present & 4) || !(present & 8))
        return;
    const auto index = field<std::uint32_t>(packet, 8);
    if (index - 1u >= 64u)
        return;
    auto& players = owner->player_engine();
    auto* player = players.by_index(std::bit_cast<std::int32_t>(index));
    if (!player || !players.is_player(*player) || !players.dormant(*player))
        return;
    // These three floats are captured before allocation and the first clock
    // call. Geometry's actual SDK bounds/pose reads follow that clock gate.
    const Vector3 origin{field<float>(packet, 0xc), field<float>(packet, 0x10),
                         field<float>(packet, 0x14)};
    auto& cached = *owner->state().players.cache.get(index);
    const auto time = players.time();
    const auto now = clock_result(time, players.tick_interval());
    if (now <= cached.geometry.last_live_tick)
        return;
    update_player_geometry(cached.geometry, *player, players, origin);
    cached.geometry.alpha = 1.0f;
    const auto refreshed_time = players.time();
    cached.geometry.last_live_tick = clock_result(refreshed_time, players.tick_interval());
}

void UserMessageHandlers::spotted_entry(EspOwnerRuntime& owner, const void* entry) {
    const auto index = field<std::uint32_t>(entry, 8);
    if (index - 1u >= 64u)
        return;
    auto& players = owner.player_engine();
    auto* player = players.by_index(std::bit_cast<std::int32_t>(index));
    if (!player || !players.is_player(*player))
        return;
    if (bindings_.recording_enabled && (field<std::uint32_t>(entry, 0x28) & 0x20) &&
        bindings_.adjustments.records[index].adjustments.correction_active) {
        auto& cached = *owner.state().players.cache.get(index);
        cached.native.shared_rule_time = players.rule_time();
        cached.native.shared_yaw = integer_float(field<std::int32_t>(entry, 0x1c));
        const auto time = players.time();
        const auto interval = players.tick_interval();
        const auto quotient = static_cast<long double>(time) / static_cast<long double>(interval);
        cached.native.flags &= 0xffffff3fu;
        cached.native.shared_yaw_tick = detail::truncate(float(quotient + 0.5L));
    }
    if (!owner.player_config().dormant.value || !players.dormant(*player))
        return;
    // A second native cache lookup follows the shared-yaw branch; callbacks
    // may have replaced the selected low-six-bit record in the meantime.
    auto& cached = *owner.state().players.cache.get(index);
    const auto time = players.time();
    const auto interval = players.tick_interval();
    const auto simulation_interval = players.simulation_tick_interval();
    const auto quotient = static_cast<long double>(time) / static_cast<long double>(interval);
    const auto reciprocal = 1.0f / simulation_interval;
    const auto rounded = float(quotient + 0.5L);
    const auto rounded_age = reciprocal + 0.5f;
    const auto now = detail::truncate(rounded);
    const auto maximum_age = detail::truncate(rounded_age);
    if (detail::sub(now, cached.geometry.last_live_tick) <= maximum_age)
        return;
    const Vector3 origin{integer_float(field<std::int32_t>(entry, 0x10)) * 4.0f,
                         integer_float(field<std::int32_t>(entry, 0x14)) * 4.0f,
                         integer_float(field<std::int32_t>(entry, 0x18)) * 4.0f};
    update_player_geometry(cached.geometry, *player, players, origin);
    cached.geometry.alpha = std::bit_cast<float>(0x3f4ccccdu);
    // Unlike WeaponSound, this path retains its pre-geometry clock sample.
    cached.geometry.last_live_tick = now;
}

void UserMessageHandlers::spotted(const void* packet) {
    auto* owner = bindings_.active_owner;
    if (!owner || (!owner->player_config().dormant.value && !bindings_.recording_enabled))
        return;
    for (std::int32_t index = 0; index < field<std::int32_t>(packet, 0xc); ++index) {
        const auto* entries = pointer32(packet, 8);
        const auto* entry = pointer32(entries, std::ptrdiff_t(index) * 4);
        spotted_entry(*owner, entry);
    }
}

void bind_user_message_handlers(UserMessageHandlers& runtime) noexcept {
    handlers = &runtime;
}
void unbind_user_message_handlers(UserMessageHandlers& runtime) noexcept {
    if (handlers == &runtime)
        handlers = nullptr;
}
extern "C" void __cdecl weapon_sound_user_message_hook(const void* packet) {
    handlers->weapon_sound(packet);
}
extern "C" void __cdecl spotted_user_message_hook(const void* packet) {
    handlers->spotted(packet);
}
} //namespace skeet::esp

// ============================================================================
// Subcomponent: weapon_icon.cpp
// ============================================================================

namespace skeet::esp {
using namespace detail;

void draw_weapon_icon(ui::RenderQueue& q, WeaponIconProvider provider, std::uint16_t definition,
                      Point position, std::uint32_t color, i32* output) {
    std::uint16_t normalized;
    if (std::uint16_t(definition - 500) <= 25)
        normalized = 42;
    else {
        if (std::uint16_t(definition - 1) > 63)
            return;
        normalized = definition;
    }
    WeaponIcon icon;
    if (!provider.load(provider.context, q, normalized, definition, 25, icon) || icon.texture == -1)
        return;
    const auto scale = 12.0f / float(icon.height);
    const auto height = truncate(float(icon.height) * scale);
    const auto width = truncate(float(icon.width) * scale);
    if (output)
        *output = width;
    const auto x = sub(position.x, width / 2);
    const auto alpha = std::min<std::uint32_t>(color >> 24, 180) << 24;
    q.texture({x, position.y, std::uint32_t(icon.texture), alpha, 0, 0, width, height, 1, 1.0f});
    q.texture({sub(x, 1), sub(position.y, 1), std::uint32_t(icon.texture), color, 0, 0, width,
               height, 1, 1.0f});
}

} // namespace skeet::esp

// ============================================================================
// Subcomponent: weapon_text.cpp
// ============================================================================
#ifndef NOMINMAX
#define NOMINMAX
#endif

namespace skeet::esp {
using namespace detail;

WeaponTextProvider weapon_text_provider(WeaponTextBindings& bindings) noexcept {
    return {&bindings,
            [](void* context, std::int32_t definition) -> const char* {
                auto& engine = *static_cast<WeaponTextBindings*>(context);
                if (!engine.item_system && engine.item_system_factory)
                    engine.item_system = engine.item_system_factory();
                if (!engine.item_system)
                    return nullptr;
                const auto schema = skeet::engine::invoke<void*>(engine.item_system, 0);
                if (!schema)
                    return nullptr;
                const auto item = skeet::engine::invoke<void*>(schema, 4, definition);
                if (!item)
                    return nullptr;
                return skeet::engine::invoke<const char*>(item, 2);
            },
            [](void* context, const char* token) -> const char16_t* {
                auto& engine = *static_cast<WeaponTextBindings*>(context);
                return skeet::engine::invoke<const char16_t*>(engine.localization, 11, token);
            }};
}

void draw_weapon_text(ui::RenderQueue& queue, WeaponTextProvider provider, std::int32_t definition,
                      Point position, std::uint32_t color, Point* measured_size) {
    if (definition == 0)
        return;
    const auto* token = provider.schema_name(provider.context, definition);
    if (!token)
        return;
    const auto* localized = provider.localize(provider.context, token);
    std::array<wchar_t, 64> wide{};
    static_assert(sizeof(wchar_t) == sizeof(char16_t));
    if (localized) {
        const auto length = std::char_traits<char16_t>::length(localized);
        if (length >= wide.size())
            throw std::length_error("Weapon localization exceeds native label storage");
        std::copy_n(localized, length, wide.begin());
        _wcsupr_s(wide.data(), wide.size());
        queue.text(sub(position.x, 1), position.y, color, 0x14, 0,
                   {reinterpret_cast<const char16_t*>(wide.data()), std::wcslen(wide.data())});
        if (measured_size)
            *measured_size = ui::measure_text(
                queue, {reinterpret_cast<const char16_t*>(wide.data()), std::wcslen(wide.data())},
                4);
    } else {
        const auto length = std::strlen(token);
        if (length < 13 || length - 13 >= 64)
            throw std::length_error("Weapon schema token violates native label storage contract");
        std::array<char, 64> narrow{};
        std::copy_n(token + 13, length - 13, narrow.begin());
        _strupr_s(narrow.data(), narrow.size());
        const auto count = _snwprintf(wide.data(), wide.size(), L"%hs", narrow.data());
        if (count > 0)
            queue.text(sub(position.x, 1), position.y, color, 0x14, 0,
                       {reinterpret_cast<const char16_t*>(wide.data()), std::size_t(count)});
        if (measured_size) { // 0x433B578E -> UTF-8 conversion 0x43396CB3
            wchar_t measured[1024]{};
            const auto converted =
                MultiByteToWideChar(CP_UTF8, 0, narrow.data(), -1, measured, 1024);
            *measured_size = ui::measure_text(queue,
                                              {reinterpret_cast<const char16_t*>(measured),
                                               converted > 0 ? std::size_t(converted - 1) : 0},
                                              4);
        }
    }
}

} // namespace skeet::esp

// ============================================================================
// Subcomponent: world_engine.cpp
// ============================================================================

#pragma STDC FENV_ACCESS ON

namespace skeet::esp {
using namespace world_detail;
void* WorldEntityEngine::by_index(std::int32_t index) const {
    return bindings_.queries.grenades().by_index(index);
}
bool WorldEntityEngine::dormant(void* entity) const {
    return engine::invoke<bool>(static_cast<std::byte*>(entity) + 8, 9);
}
std::uint32_t WorldEntityEngine::client_class_index(void* entity) const {
    const auto* type = engine::invoke<const void*>(static_cast<std::byte*>(entity) + 8, 2);
    return field<std::uint32_t>(type, 0x14);
}
bool WorldEntityEngine::is_weapon(void* entity) const {
    return engine::invoke<bool>(entity, 166);
}
std::int32_t WorldEntityEngine::highest_entity_index() const {
    return field<std::int32_t>(bindings_.queries.grenades().bindings().entity_list, 0x24);
}
std::int32_t WorldEntityEngine::game_type() const {
    return convar_integer(bindings_.game_type);
}
std::int32_t WorldEntityEngine::ammunition(const WorldEntity& entity) const {
    return field<std::int32_t>(reinterpret_cast<const void*>(entity.identity),
                               bindings_.offsets.ammunition);
}
std::int32_t WorldEntityEngine::ammunition_capacity(const WorldEntity& entity) const {
    return engine::invoke<std::int32_t>(reinterpret_cast<void*>(entity.identity), 375);
}
char WorldEntityEngine::bomb_site(std::int32_t index) const {
    if (std::uint32_t(index) & 0xFFFF0000u)
        return 'A';
    auto* entity = by_index(index);
    if (!entity)
        return 'A';
    auto* slot = bindings_.queries.grenades().bindings().player_resource_slot;
    if (!slot)
        return 'A';
    auto* resource = field<void*>(slot, 0);
    if (!resource)
        return 'A';
    // The two SDK calls precede every coordinate load, including the first
    // returned bounds vector. Their providers can mutate the same storage.
    const auto* maximum = engine::invoke<const Vector3*>(entity, 77);
    const auto* minimum = engine::invoke<const Vector3*>(entity, 76);
    const Vector3 center{(maximum->x - minimum->x) * 0.5f + minimum->x,
                         (maximum->y - minimum->y) * 0.5f + minimum->y,
                         (maximum->z - minimum->z) * 0.5f + minimum->z};
    const auto first = field<Vector3>(resource, bindings_.offsets.bomb_site_first);
    const auto second = field<Vector3>(resource, bindings_.offsets.bomb_site_second);
    return squared(center, first) > squared(center, second) ? 'B' : 'A';
}
void WorldEntityEngine::dispatch(ui::RenderQueue& queue, EspFrameOwner& owner, std::int32_t& cursor,
                                 const EspFrameContext& frame, RuntimePlayer& view, void* entity,
                                 EntityOverlayHandler handler) {
    switch (handler) {
    case EntityOverlayHandler::hostage:
        hostage(queue, frame, view, entity);
        break;
    case EntityOverlayHandler::dropped_bomb:
        dropped_bomb(queue, owner, frame, entity);
        break;
    case EntityOverlayHandler::planted_bomb:
        planted_bomb(queue, owner, cursor, frame, view, entity);
        break;
    case EntityOverlayHandler::grenade_projectile:
        grenade(queue, owner, frame, entity);
        break;
    case EntityOverlayHandler::inferno:
        inferno(queue, owner, frame, view, entity);
        break;
    case EntityOverlayHandler::danger_zone_item:
        danger_zone(owner, view, entity);
        break;
    case EntityOverlayHandler::defuse_kit:
        defuse_kit(queue, owner, frame, view, entity);
        break;
    case EntityOverlayHandler::none:
        break;
    }
}
void WorldEntityEngine::collect_dropped(WorldEntityScan& scan,
                                        std::array<WorldEntity, 65>& frame_entities, void* entity,
                                        std::int32_t index, std::uint32_t options,
                                        const EspFrameOwner& owner, const EspFrameContext& frame) {
    if (!options)
        return;
    const auto& offsets = bindings_.offsets;
    std::uint16_t definition;
    if (is_weapon(entity)) {
        if (field<std::int32_t>(entity, offsets.owner_handle) != -1)
            return;
        definition = field<std::uint16_t>(entity, offsets.definition);
        if (ammunition_weapon(definition)) {
            const auto clip = field<std::int32_t>(entity, offsets.ammunition);
            if (clip < 0 ||
                detail::add(field<std::int32_t>(entity, offsets.reserve_ammunition), clip) <= 0)
                return;
        }
    } else {
        if (owner.defuse_kit_model == -1 ||
            field<std::int16_t>(entity, offsets.model_index) != owner.defuse_kit_model ||
            field<std::int32_t>(entity, offsets.owner_handle) != -1)
            return;
        definition = 55;
    }
    Point screen;
    if (!project_point(frame.draw.projection, field<Vector3>(entity, offsets.origin), screen) ||
        definition >= 65)
        return;
    const auto distance = squared(bindings_.view_origin, field<Vector3>(entity, offsets.origin));
    auto& selected = scan.dropped[definition];
    if (selected.entity && !(distance < selected.distance_squared))
        return;
    auto& value = frame_entities[definition];
    value.identity = reinterpret_cast<std::uintptr_t>(entity);
    value.index = static_cast<std::uint32_t>(index);
    value.definition = definition;
    selected = {&value, distance, screen};
}
void WorldEntityEngine::danger_zone(EspFrameOwner& owner, RuntimePlayer& view, void* entity) {
    const auto model = field<std::int16_t>(entity, bindings_.offsets.model_index);
    if (model < 0)
        return;
    const auto* item = owner.entity_models.models.find(static_cast<std::uint16_t>(model));
    if (!item || !item->setting->enabled.value)
        return;
    WorldEntity value;
    value.model_index = model;
    value.origin = field<Vector3>(entity, bindings_.offsets.origin);
    collect_danger_zone_model(owner.entity_models, value,
                              field<Vector3>(view.engine_entity, bindings_.offsets.origin));
}
void WorldEntityEngine::dropped_bomb(ui::RenderQueue& queue, EspFrameOwner& owner,
                                     const EspFrameContext& frame, void* entity) {
    if (!frame.world_config.dropped_weapons.value)
        return;
    if (bindings_.game_rules && *bindings_.game_rules &&
        !field<std::uint8_t>(*bindings_.game_rules, bindings_.offsets.rules_allow_bomb))
        return;
    if (field<std::int32_t>(entity, bindings_.offsets.dropped_bomb_state))
        return;
    Point screen;
    if (!project_point(frame.draw.projection, field<Vector3>(entity, bindings_.offsets.origin),
                       screen))
        return;
    queue.text(screen.x, screen.y, frame.world_config.bomb.color.value, 0x14, 0,
               owner.dropped_bomb_label);
    if (frame.world_config.dropped_weapons.value & 16)
        distance(queue, {screen.x, detail::add(screen.y, 10)},
                 squared(bindings_.view_origin, field<Vector3>(entity, bindings_.offsets.origin)),
                 frame.world_config.bomb.color.value);
}
void WorldEntityEngine::defuse_kit(ui::RenderQueue& queue, const EspFrameOwner& owner,
                                   const EspFrameContext& frame, RuntimePlayer& view,
                                   void* entity) {
    if (!frame.world_config.dropped_weapons.value ||
        field<std::int32_t>(view.engine_entity, bindings_.offsets.team) == 2)
        return;
    if (field<std::int16_t>(entity, bindings_.offsets.model_index) != owner.defuse_kit_model)
        return;
    Point screen;
    if (!project_point(frame.draw.projection, field<Vector3>(entity, bindings_.offsets.origin),
                       screen))
        return;
    queue.text(screen.x, screen.y, frame.world_config.dropped_weapon_color.value, 0x14, 0, u"KIT");
    if (frame.world_config.dropped_weapons.value & 16)
        distance(queue, {screen.x, detail::add(screen.y, 10)},
                 squared(bindings_.view_origin, field<Vector3>(entity, bindings_.offsets.origin)),
                 frame.world_config.dropped_weapon_color.value);
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: world_engine_bomb.cpp
// ============================================================================

#pragma STDC FENV_ACCESS ON

namespace skeet::esp {
using namespace world_detail;

// 433CD3A7. The registry entity is independent of the world entity whose
// countdown is being drawn. Its trace precedes every target-property read.
float WorldEntityEngine::bomb_damage(EspFrameOwner& owner, void* target) {
    const auto peak = owner.bomb_peak_damage;
    const auto* registry = bindings_.planted_bombs;
    if (!registry || field<std::int32_t>(registry, 0xC) == 0)
        return 0.0f;
    const auto* entries = field<void* const*>(registry, 0);
    auto* bomb = *entries;
    if (!bomb || !field<std::uint8_t>(bomb, bindings_.offsets.bomb_ticking))
        return 0.0f;

    const auto origin = *engine::invoke<const Vector3*>(bomb, 10);
    const MovementVector start{origin.x, origin.y, origin.z + 8.0f};
    const MovementVector delta{start.x - start.x, start.y - start.y, (start.z - 40.0f) - start.z};
    const bool swept = (delta.y * delta.y + delta.x * delta.x) + delta.z * delta.z != 0.0f;
    const MovementCollisionRay ray{start, delta, {0, 0, 0}, {0, 0, 0}, true, swept};
    MovementCollisionTrace trace;
    trace.fraction = 1.0f;
    trace.all_solid = 0;
    trace.start_solid = 0;
    trace.hitgroup = -1;
    trace.entity = 0;
    trace.hitbox = -1;
    bindings_.queries.movement().trace_ray(
        ray, 0x200400Bu,
        {MovementCollisionFilterKind::skip_player,
         static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(bomb)), 0, 0},
        trace);
    auto explosion = origin;
    if (trace.fraction != 1.0f) {
        explosion.x = trace.plane_normal.x * 0.6f + trace.end.x;
        explosion.y = trace.plane_normal.y * 0.6f + trace.end.y;
        explosion.z = trace.plane_normal.z * 0.6f + trace.end.z;
    }
    const auto radius = (peak * 3.5f) * std::bit_cast<float>(0x3EAAAAABu);
    const auto denominator = (radius + radius) * radius;
    const auto* target_origin = engine::invoke<const Vector3*>(target, 10);
    const auto explosion_z = explosion.z + 1.0f;
    const auto origin_x = target_origin->x, origin_y = target_origin->y,
               origin_z = target_origin->z;
    const auto offset = field<Vector3>(target, bindings_.offsets.view_offset);
    const auto y = (origin_y + offset.y) - explosion.y;
    const auto x = (origin_x + offset.x) - explosion.x;
    const auto z = (origin_z + offset.z) - explosion_z;
    const auto distance_ratio = ((y * y + x * x) + z * z) / denominator;
    const auto damage = std::exp(-distance_ratio) * peak;
    return field<std::int32_t>(target, bindings_.offsets.armor) > 0 ? damage * 0.5f : damage;
}

// Complete 43345EF2. Raw fields remain behind their original gates; in
// particular the indicator's Lua call precedes life-state, health and damage.
void WorldEntityEngine::planted_bomb(ui::RenderQueue& queue, EspFrameOwner& owner,
                                     std::int32_t& cursor, const EspFrameContext& frame,
                                     RuntimePlayer& view, void* entity) {
    auto* const target = view.engine_entity;
    std::memset(&owner.frame.timer, 0, sizeof owner.frame.timer);
    if (!frame.world_config.bomb.enabled.value)
        return;
    const auto& offsets = bindings_.offsets;
    if (!field<std::uint8_t>(entity, offsets.bomb_ticking))
        return;
    const auto now = bindings_.current_time;
    const auto detonation = field<float>(entity, offsets.bomb_detonation_time);
    const auto remaining = detonation - now;
    auto indicator = [&](std::string_view text, std::uint32_t color, std::int32_t texture) {
        if (frame.draw.scripts)
            draw_indicator(*frame.draw.scripts, queue, cursor, text, color, frame.indicator_layout,
                           texture, -1.0f);
        else
            draw_indicator(queue, cursor, text, color, frame.indicator_layout, texture, -1.0f);
    };
    if (remaining > 0.0f) {
        const auto defuse_end = field<float>(entity, offsets.bomb_defuse_end);
        if (defuse_end > 0.0f && field<std::int32_t>(entity, offsets.bomb_defuser_state) == 1) {
            const bool in_time = !(defuse_end > detonation);
            const auto defuse_remaining = defuse_end - now;
            const auto height = detail::truncate(
                (float(detail::sub(queue.screen().y, 2)) * defuse_remaining) / 10.0f);
            const auto foreground_height = queue.screen().y;
            const auto background_height = queue.screen().y;
            queue.rectangle({0, 0, 20, background_height, 0x73000000u});
            const auto color = (in_time ? 0x7350FF00u : 0x73503200u) | (in_time ? 0x32u : 0xFFu);
            queue.rectangle(
                {1, detail::sub(detail::sub(foreground_height, height), 2), 18, height, color});
        }
        char site = 'A';
        auto* resource_slot = bindings_.queries.grenades().bindings().player_resource_slot;
        if (resource_slot) {
            auto* resource = field<void*>(resource_slot, 0);
            if (resource) {
                const auto origin = field<Vector3>(entity, offsets.bomb_origin);
                const auto first = field<Vector3>(resource, offsets.bomb_site_first);
                const auto second = field<Vector3>(resource, offsets.bomb_site_second);
                if (squared(origin, first) > squared(origin, second))
                    site = 'B';
            }
        }
        char label[48]{};
        const auto length =
            std::snprintf(label, sizeof label, "%c - %.1fs", site, double(remaining));
        if (length > 0)
            indicator({label, std::size_t(length)}, 0xC8FFFFFFu, bindings_.bomb_label_texture);
    }
    if (!(remaining > -1.0f))
        return;
    if (field<std::uint8_t>(target, offsets.life_state))
        return;
    if (engine::invoke<std::int32_t>(target, 122) <= 0)
        return;
    const auto damage = detail::truncate(bomb_damage(owner, target));
    if (!damage)
        return;
    if (damage >= engine::invoke<std::int32_t>(target, 122)) {
        indicator("FATAL", 0xFF3200FFu, -1);
    } else {
        char label[32]{};
        const auto length = std::snprintf(label, sizeof label, "-%d HP", damage);
        if (length > 0)
            indicator({label, std::size_t(length)}, 0xFF69F3FCu, -1);
    }
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: world_engine_grenade.cpp
// ============================================================================

#pragma STDC FENV_ACCESS ON

namespace skeet::esp {
using namespace world_detail;
void WorldEntityEngine::inferno(ui::RenderQueue& queue, EspFrameOwner& owner,
                                const EspFrameContext& frame, RuntimePlayer& view, void* entity) {
    auto* const viewer = view.engine_entity;
    //43393ECC starts with this unused network index query, even when the
    // following friendly-fire/owner-team gate suppresses the label entirely.
    engine::invoke<std::int32_t>(static_cast<std::byte*>(entity) + 8, 10);
    if (convar_integer(bindings_.friendly_fire) == 0) {
        // Complete433B1A30: resolve the same8D1BD8D0 owner handle used by
        // dropped entities, only after the ConVar's parent chain reads zero.
        const auto handle = field<std::uint32_t>(entity, bindings_.offsets.owner_handle);
        if (handle != UINT32_MAX) {
            auto* thrower = bindings_.queries.grenades().by_handle(handle);
            if (thrower && field<std::int32_t>(thrower, bindings_.offsets.team) ==
                               field<std::int32_t>(viewer, bindings_.offsets.team))
                return;
        }
    }
    grenade(queue, owner, frame, entity);
}
void WorldEntityEngine::grenade(ui::RenderQueue& queue, EspFrameOwner& owner,
                                const EspFrameContext& frame, void* entity) {
    const auto& config = frame.grenade_config;
    if (!config.entities.value)
        return;
    const auto type = static_cast<std::int32_t>(client_class_index(entity));
    const auto& classes = owner.grenade_classes;
    const auto& offsets = bindings_.offsets;
    std::u16string_view label;
    if (type == classes.base_projectile) {
        const auto damage = field<float>(entity, offsets.grenade_damage);
        if (damage == 350.0f && field<float>(entity, offsets.grenade_radius) == 99.0f) {
            if (field<std::int32_t>(entity, offsets.explosion_effect_tick) != 0)
                return;
            label = owner.grenade_labels.explosive;
        } else {
            if (damage != 100.0f || field<float>(entity, offsets.grenade_radius) != 100.0f)
                return;
            label = owner.grenade_labels.flash;
        }
    } else if (type == classes.smoke_projectile)
        label = owner.grenade_labels.smoke;
    else if (type == classes.decoy_projectile)
        label = owner.grenade_labels.decoy;
    else if (type == classes.molotov_projectile || type == classes.inferno)
        label = owner.grenade_labels.fire;
    else
        label = owner.grenade_labels.other;
    Point screen;
    if (!project_point(frame.draw.projection, field<Vector3>(entity, offsets.origin), screen))
        return;
    std::uint32_t color = 0xC8FFFFFFu;
    if (!bindings_.suspended) {
        if (type != classes.inferno && frame.glow && frame.glow->registry) {
            const auto index = field<std::int32_t>(entity, offsets.glow_index);
            if (index != -1) {
                const auto packed = config.entity_color.value;
                constexpr float scale = 0.0039215688593685626983642578125f;
                effects::update_glow(*frame.glow, entity, static_cast<std::uint32_t>(index),
                                     {float(packed & 255) * scale,
                                      float((packed >> 8) & 255) * scale,
                                      float((packed >> 16) & 255) * scale, 1.0f});
            }
        }
        float duration = 0.0f;
        std::int32_t elapsed = 0;
        if (type == classes.smoke_projectile && field<std::uint8_t>(entity, offsets.smoke_active)) {
            duration = 18.0f;
            elapsed = detail::sub(bindings_.current_tick,
                                  field<std::int32_t>(entity, offsets.smoke_start_tick));
        } else if (type == classes.inferno) {
            duration = 7.0f;
            elapsed = detail::sub(bindings_.current_tick,
                                  field<std::int32_t>(entity, offsets.inferno_start_tick));
        }
        if (duration != 0.0f) {
            const float fraction = (duration - float(elapsed) * bindings_.tick_interval) / duration;
            const auto bounded = detail::sse_min(detail::sse_max(fraction, 0.1f), 1.0f);
            const auto alpha =
                std::bit_cast<std::uint32_t>(std::sqrt(bounded) * 255.0f + 8388608.0f) & 255u;
            color = (alpha << 24) | 0xFFFFFFu;
        }
    }
    queue.text(screen.x, detail::add(screen.y, 12), color, 0x14, 0, label);
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: world_engine_hostage.cpp
// ============================================================================

#pragma STDC FENV_ACCESS ON

namespace skeet::esp {
using namespace world_detail;
namespace {
/* [removed duplicate transform] */
}
void WorldEntityEngine::hostage(ui::RenderQueue& queue, const EspFrameContext& frame,
                                RuntimePlayer& view, void* entity) {
    auto* const viewer = view.engine_entity;
    if (!frame.world_config.hostages.enabled.value)
        return;
    if (dormant(entity))
        return;
    const auto& offsets = bindings_.offsets;
    if ((field<std::uint8_t>(entity, offsets.hostage_effects) & 2) ||
        field<std::int32_t>(entity, offsets.hostage_life_state) >= 2)
        return;
    std::array<Matrix3x4, 256> matrices;
    // Genuine IClientRenderable::SetupBones, not the player's client bone
    // builder. It may replace the studio wrapper and selected hitbox set.
    if (!engine::invoke<bool>(static_cast<std::byte*>(entity) + 4, 13, matrices.data(),
                              std::int32_t(256), std::int32_t(0x300), bindings_.current_time))
        return;
    const auto* wrapper = field<const void*>(entity, offsets.studio_wrapper);
    if (!wrapper)
        return;
    const auto* header = field<const std::byte*>(wrapper, 0);
    if (!header)
        return;
    const auto set_index = field<std::int32_t>(entity, offsets.hitbox_set);
    const auto* set = header + field<std::int32_t>(header, 0xB0) + set_index * 12;
    const auto count = field<std::int32_t>(set, 4);
    if (count == 0)
        return;
    const auto* hitbox = set + field<std::int32_t>(set, 8);
    std::int32_t left = INT32_MAX, top = INT32_MAX, right = INT32_MIN, bottom = INT32_MIN;
    for (std::int32_t n = 0; n < count; ++n, hitbox += 68) {
        // Native copies all68 bytes before transforming this hitbox.
        std::array<std::byte, 68> copy;
        std::memcpy(copy.data(), hitbox, copy.size());
        const auto bone = field<std::uint32_t>(copy.data(), 0);
        if (bone >= matrices.size())
            throw std::out_of_range("Hostage hitbox exceeds the native256-matrix workspace");
        const auto first = transform(matrices[bone], field<Vector3>(copy.data(), 8), false);
        const auto second = transform(matrices[bone], field<Vector3>(copy.data(), 20), true);
        for (const auto point : std::array<Vector3, 8>{{{first.x, first.y, first.z},
                                                        {first.x, first.y, second.z},
                                                        {first.x, second.y, first.z},
                                                        {first.x, second.y, second.z},
                                                        {second.x, first.y, first.z},
                                                        {second.x, first.y, second.z},
                                                        {second.x, second.y, first.z},
                                                        {second.x, second.y, second.z}}}) {
            Point screen;
            if (!project_point(frame.draw.projection, point, screen))
                return;
            left = std::min(left, screen.x);
            right = std::max(right, screen.x);
            top = std::min(top, screen.y);
            bottom = std::max(bottom, screen.y);
        }
    }
    left = std::max(left, 0);
    right = std::max(right, 0);
    top = std::max(top, 0);
    bottom = std::max(bottom, 0);
    if (left >= right || top >= bottom)
        return;
    const auto width = detail::sub(right, left), height = detail::sub(bottom, top);
    const auto center = detail::add(left, width / 2);
    queue.text(center, detail::sub(top, 5), frame.world_config.hostages.color.value, 0x14, 0,
               u"HOSTAGE");
    auto baseline = detail::add(top, 3);
    if (frame.player_config.bounding_box.value) {
        queue.rectangle_outline({detail::sub(left, 1), detail::sub(top, 1), detail::add(width, 2),
                                 detail::add(height, 2), 0x8C000000u},
                                1);
        queue.rectangle_outline({left, top, width, height, frame.world_config.hostages.color.value},
                                1);
        queue.rectangle_outline({detail::add(left, 1), detail::add(top, 1), detail::sub(width, 2),
                                 detail::sub(height, 2), 0x8C000000u},
                                1);
        baseline = detail::add(bottom, 6);
    }
    if (frame.player_config.distance.value)
        distance(
            queue, {center, baseline},
            squared(field<Vector3>(viewer, offsets.origin), field<Vector3>(entity, offsets.origin)),
            0xC8FFFFFFu);
}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: world_engine_offsets.cpp
// ============================================================================

namespace skeet::esp {
WorldEngineOffsets::WorldEngineOffsets(const engine::NativeOffsetGlobals& g)
    : origin{g, 0x43468D30, 0x43468ED4, 0x2A671B42, 0},
      model_index{g, 0x43468978, 0x43468F7C, 0x47877DEB, 0},
      owner_handle{g, 0x43467FCC, 0x43467938, 0x8D1BD8D0, 0},
      definition{g, 0x4346908C, 0x43468BA0, 0x5C9F0FBB, 0},
      ammunition{g, 0x4346A60C, 0x434678A8, 0xCA5EC698, 0},
      reserve_ammunition{g, 0x4346749C, 0x43468C14, 0x49C770CA, 0},
      team{g, 0x434688B0, 0x4346A518, 0x8B5586F6, 0},
      hostage_effects{g, 0x43467634, 0x43468F2C, 0xBB78DC43, 0},
      hostage_life_state{g, 0x434684FC, 0x43467D44, 0xCB6C59C2, 0},
      studio_wrapper{g, 0x43467E8C, 0x43467900, 0x32E4DAF1, 0},
      hitbox_set{g, 0x43468240, 0x4346A84C, 0xC2B6FB10, 0},
      rules_allow_bomb{g, 0x4346921C, 0x43468F24, 0x82F5AD08, 0},
      dropped_bomb_state{g, 0x43468F40, 0x4346A5B0, 0x71A5AC08, 0},
      grenade_damage{g, 0x434674EC, 0x4346894C, 0x727317C4, 0},
      grenade_radius{g, 0x4346A6A4, 0x43467120, 0x9D63BB7F, 0},
      explosion_effect_tick{g, 0x43467210, 0x43467630, 0x01E80986, 0},
      glow_index{g, 0x43469210, 0x4346A540, 0x442A38AE, 0},
      smoke_active{g, 0x4346A530, 0x43467564, 0x58AE2423, 0},
      smoke_start_tick{g, 0x43467AC0, 0x43468E60, 0xF267E956, 0},
      inferno_start_tick{g, 0x43468DD4, 0x43467B40, 0x0DCBA89C, 0},
      bomb_ticking{g, 0x4346A6C4, 0x43468EF4, 0x0E5BECE5, 0},
      bomb_detonation_time{g, 0x4346834C, 0x4346A880, 0xD280D4E1, 0},
      bomb_defuse_end{g, 0x4346A750, 0x4346A728, 0x800BB0B1, 0},
      bomb_defuser_state{g, 0x4346A5F8, 0x43467B88, 0x950F7B20, 0},
      bomb_origin{g, 0x434670CC, 0x43468BCC, 0xA9EE6062, 0},
      bomb_site_first{g, 0x4346A71C, 0x43467DE0, 0xFCCAF99B, 0},
      bomb_site_second{g, 0x43467C00, 0x43467C68, 0xE92192CA, 0},
      view_offset{g, 0x43468A34, 0x43468C00, 0xC246315E, 0},
      armor{g, 0x43467908, 0x434682E4, 0x47A6F50C, 0},
      life_state{g, 0x43468E78, 0x434684E4, 0x9D920E35, 0} {}
} // namespace skeet::esp

// ============================================================================
// Subcomponent: composite_handlers.cpp
// ============================================================================

namespace skeet::hud {
void FreestandingHudBinding::draw(ui::RenderQueue& queue, void*, std::int32_t& cursor) const {
    draw_freestanding_indicator(queue, scripts, global, config, state, indicators, local, input,
                                cursor, layout);
}
void PingHudBinding::draw(ui::RenderQueue& queue, void*, std::int32_t& cursor) const {
    draw_ping_indicator(queue, scripts, indicators, local, ping, cursor, layout);
}
void RageHudBinding::draw(ui::RenderQueue& queue, void*, std::int32_t& cursor) const {
    draw_rage_indicators(queue, scripts, global, config, indicators, local, input, cursor, layout,
                         visualization, highlight, highlight_context);
}
void LiveRageHudBinding::draw(ui::RenderQueue& queue, void* player, std::int32_t& cursor) const {
    if (!highlight_context.life_state(player)) {
        //43468B94 is reloaded after each indicator/script callback. A callback
        //can change the next bit, hotkey, or damage-override setting this frame.
        if ((std::uint32_t(indicators.value) & 1u) &&
            hotkey_active(global.force_safe_point.value.binding, input))
            esp::draw_indicator(scripts, queue, cursor, "SAFE", 0xc8ffffffu, layout, -1, -1.0f);
        if ((std::uint32_t(indicators.value) & 2u) &&
            hotkey_active(global.force_body_aim.value.binding, input))
            esp::draw_indicator(scripts, queue, cursor, "BODY", 0xc8ffffffu, layout, -1, -1.0f);
        if ((std::uint32_t(indicators.value) & 256u) && config.damage_override.value &&
            hotkey_active(config.damage_override_key.value.binding, input))
            esp::draw_indicator(scripts, queue, cursor, "MD", 0xc8ffffffu, layout, -1, -1.0f);
    }
    esp::draw_live_hitbox_highlight(queue, visualization, highlight, player, highlight_context);
}
void ExploitHudBinding::draw(ui::RenderQueue& queue, void* player, std::int32_t& cursor) const {
    draw_exploit_indicators(queue, scripts, state, shared_timer, indicators, player, interfaces,
                            frame_time, cursor, layout);
}
void MovementHudBinding::draw(ui::RenderQueue& queue, void*, std::int32_t& cursor) const {
    draw_movement_indicators(queue, scripts, indicators, duck_enabled, rage, command_flags,
                             quick_peek, state, local, input, cursor, layout, projection);
}
void CrosshairHudBinding::draw(ui::RenderQueue& queue, void* player, std::int32_t&) const {
    draw_crosshair(queue, config, interfaces, player, suspended, screen_size);
}
void PenetrationHudBinding::draw(ui::RenderQueue& queue, void* player, std::int32_t&) const {
    draw_penetration_reticle(queue, config, state, player, interfaces, screen_size);
}
void ScreenOverlayHudBinding::draw(ui::RenderQueue& queue, void* player, std::int32_t&) const {
    draw_screen_overlay(queue, config, state, player, life_state_offset, screen_size, projection,
                        view_origin, trigonometry);
}
void ScopeHudBinding::draw(ui::RenderQueue& queue, void* player, std::int32_t&) const {
    state.draw_scope(queue, screen_size, player);
}
void WorldHudBinding::draw(ui::RenderQueue&, void*, std::int32_t&) const {
    state.update();
}
void HitMarkerHudBinding::draw(ui::RenderQueue& queue, void* player, std::int32_t&) const {
    auto frame = view;
    frame.player = player;
    state.draw(queue, frame);
}
void PerformanceHudBinding::draw(ui::RenderQueue& queue, void* player, std::int32_t& cursor) const {
    std::uint8_t life;
    std::memcpy(&life, static_cast<const std::byte*>(player) + life_state_offset, sizeof(life));
    if (life || !config.warning.value || !low_fps || !*rage_enabled)
        return;
    if (!hotkey_active(rage.activation.value.binding, input))
        return;
    esp::draw_indicator(scripts, queue, cursor, "FPS", 0xc828ffffu, layout, -1, -1.0f);
}
void BlockbotHudBinding::draw(ui::RenderQueue& queue, void* player, std::int32_t&) const {
    std::uint8_t life;
    std::memcpy(&life, static_cast<const std::byte*>(player) + life_state_offset, sizeof(life));
    if (life || !state.activated || !(state.flags & 1u))
        return;
    const auto* origin = engine::invoke<const esp::Vector3*>(player, 10);
    const auto right = state.locked_right;
    const auto x = right.x * 64.0f, y = right.y * 64.0f, z = right.z * 64.0f;
    const esp::Vector3 first{origin->x + x, origin->y + y, origin->z + z};
    const esp::Vector3 second{origin->x - x, origin->y - y, origin->z - z};
    Point a, b;
    if (!esp::project_point(projection, first, a) || !esp::project_point(projection, second, b))
        return;
    queue.line({a.x, a.y, b.x, b.y, 0xc8000000u});
}
} // namespace skeet::hud

// ============================================================================
// Subcomponent: composite_runtime.cpp
// ============================================================================

namespace skeet::hud {
HudFrameCallback* register_hud_frame(engine::OwnerRegistryBinding registry,
                                     HudFrameCallback* callback) {
    registry.append(callback);
    return callback;
}
void dispatch_hud_frame(const engine::OwnerRegistry& callbacks, ui::RenderQueue& queue,
                        void* player, std::int32_t& cursor) {
    const auto entries = callbacks.entries();
    for (auto* object : entries) {
        auto* callback = static_cast<HudFrameCallback*>(object);
        callback->vtable[0](callback, &queue, player, &cursor);
    }
}
bool draw_composite_frame(const CompositeFrameContext& context) {
    if (context.blocked)
        return false;
    auto* renderer = context.renderer;
    if (!renderer)
        return false;
    auto* queue = renderer->begin();
    if (!queue)
        return false;
    //433CC0D6 is CVTSS2SI, including the caller's current MXCSR rounding mode.
    const auto scaled = _mm_mul_ss(_mm_set_ss(context.scale), _mm_set_ss(350.0f));
    auto cursor = _mm_cvtss_si32(scaled);
    queue->set_alpha(1.0f);
    if (engine::invoke<bool>(context.engine, 26)) {
        const auto index = engine::invoke<std::int32_t>(context.engine, 12);
        auto* player = engine::invoke<void*>(context.entity_list, 3, index);
        if (player)
            dispatch_hud_frame(context.callbacks, *queue, player, cursor);
    }
    queue->set_alpha(1.0f);
    script::dispatch_paint_ui(context.scripts, *queue, cursor);
    for (auto* window = context.windows; window; window = window->next)
        if ((window->input.mouse_flags & 1u) || window->transition_state)
            window->draw(*queue, context.accent);
    renderer->publish();
    return true;
}
} // namespace skeet::hud

// ============================================================================
// Subcomponent: crosshair.cpp
// ============================================================================

namespace skeet::hud {
using esp::Vertex;
using esp::integer_float;
namespace {
template <class T> T read(const void* object, std::uint32_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof(value));
    return value;
}
template <class T> void write(void* object, std::uint32_t offset, T value) {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(value));
}
std::int32_t variable_integer(const void* object) {
    for (;;) {
        const auto* parent = read<const void*>(object, 0x1C);
        if (parent == object)
            break;
        object = parent;
    }
    return std::bit_cast<std::int32_t>(read<std::uint32_t>(object, 0x30) ^
                                       std::uint32_t(reinterpret_cast<std::uintptr_t>(object)));
}
std::int32_t sub(std::int32_t first, std::uint32_t second) {
    return std::bit_cast<std::int32_t>(std::uint32_t(first) - second);
}
std::uint32_t return_hash(const std::byte* address) {
    auto hash = 0x811C9DC5u;
    if (address)
        for (unsigned i = 0; i != 6; ++i)
            hash = (hash ^ std::to_integer<std::uint8_t>(address[i])) * 0x01000193u;
    return hash ^ 0x92CA5DF1u;
}
struct BoundCrosshairAccess {
    const CrosshairBindings& bindings;
    const Point& dimensions;
    std::uint32_t life() const {
        return bindings.offsets.life_state;
    }
    std::uint32_t scoped() const {
        return bindings.offsets.scoped;
    }
    std::uint32_t weapon() const {
        return bindings.offsets.active_weapon;
    }
    std::uint32_t flag() const {
        return bindings.offsets.weapon_draw_flag;
    }
    void* entities() const {
        return bindings.entity_list;
    }
    const void* style() const {
        return bindings.style_variable;
    }
    const void* mode() const {
        return bindings.weapon_mode_variable;
    }
    Point screen(const ui::RenderQueue&) const {
        return dimensions;
    }
};
struct NativeCrosshairAccess {
    const NativeCrosshairBindings& bindings;
    std::uint32_t offset(std::uint32_t pointer, std::uint32_t key, std::uint32_t mask) const {
        const auto& globals = bindings.offsets.globals;
        return *globals.pointers.at(pointer) ^ globals.keys.at(key) ^ mask;
    }
    std::uint32_t life() const {
        return offset(0x43468e78, 0x434684e4, 0x9d920e35);
    }
    std::uint32_t scoped() const {
        return offset(0x43468da4, 0x434684ec, 0xcacaeecc);
    }
    std::uint32_t weapon() const {
        return offset(0x43467a74, 0x434678c0, 0xcd812654);
    }
    std::uint32_t flag() const {
        return offset(0x43469230, 0x434679a8, 0xa1af8ffc);
    }
    void* entities() const {
        return bindings.entity_list;
    }
    const void* style() const {
        return bindings.style_variable;
    }
    const void* mode() const {
        return bindings.weapon_mode_variable;
    }
    Point screen(const ui::RenderQueue& queue) const {
        return queue.screen();
    }
};
template <class Access, class Suspended>
void draw_crosshair_body(ui::RenderQueue& queue, const esp::ScreenOverlayConfig& config,
                         const Access& access, void* local, const Suspended& suspended) {
    if (!config.crosshair.value || !local)
        return;
    if (read<std::uint8_t>(local, access.life()) || read<std::uint8_t>(local, access.scoped()))
        return;
    if (variable_integer(access.style()) < 2)
        return;
    const auto handle = read<std::int32_t>(local, access.weapon());
    if (handle == -1)
        return;
    auto* weapon = engine::invoke<void*>(access.entities(), 4, handle);
    if (!weapon)
        return;
    auto* data = engine::invoke<void*>(weapon, 461);
    if (!data)
        return;
    const auto type = read<std::int32_t>(data, 0xC8);
    if (type == 0) {
        const auto* mode = access.mode();
        if (mode && variable_integer(mode) != 0)
            return;
    } else if (type != 5)
        return;
    if (suspended) {
        const auto screen_size = access.screen(queue);
        const auto x = screen_size.x / 2;
        const auto y = screen_size.y / 2;
        queue.rectangle({sub(x, 2), sub(y, 2), 5, 5, 0xF0000000u});
        queue.rectangle({sub(x, 1), sub(y, 1), 3, 3, 0xFF00EEC4u});
    } else {
        const auto flag_offset = access.flag();
        const auto flag = read<std::uint8_t>(weapon, flag_offset);
        write(data, 0xC8, std::int32_t(3));
        write(weapon, access.flag(), std::uint8_t(0));
        engine::invoke<void>(weapon, 404);
        write(data, 0xC8, type);
        write(weapon, flag_offset, flag);
    }
}
} //namespace

void draw_crosshair(ui::RenderQueue& queue, const esp::ScreenOverlayConfig& config,
                    const CrosshairBindings& bindings, void* local, bool suspended,
                    Point screen_size) {
    draw_crosshair_body(queue, config, BoundCrosshairAccess{bindings, screen_size}, local,
                        suspended);
}
void draw_live_crosshair(ui::RenderQueue& queue, const esp::ScreenOverlayConfig& config,
                         const NativeCrosshairBindings& bindings, void* local,
                         const std::uint8_t& suspended) {
    draw_crosshair_body(queue, config, NativeCrosshairAccess{bindings}, local, suspended);
}
void prepare_crosshair_view(const esp::ScreenOverlayConfig& config,
                            const CrosshairBindings& bindings, void* local, void* view_setup,
                            bool suspended, const std::byte* return_address,
                            void** original_vtable) {
    prepare_crosshair_view(config, CrosshairViewBindings{bindings.entity_list, bindings.offsets},
                           local, view_setup, suspended, return_address, original_vtable);
}
void prepare_crosshair_view(const esp::ScreenOverlayConfig& config,
                            const CrosshairViewBindings& bindings, void* local, void* view_setup,
                            bool suspended, const std::byte* return_address,
                            void** original_vtable) {
    if (config.crosshair.value && !suspended && view_setup &&
        return_hash(return_address) == 0xA3C52284u) {
        const auto& offsets = bindings.offsets;
        if (!read<std::uint8_t>(local, offsets.life_state) &&
            !read<std::uint8_t>(local, offsets.scoped)) {
            const auto handle = read<std::int32_t>(local, offsets.active_weapon);
            if (handle != -1) {
                auto* weapon = engine::invoke<void*>(bindings.entity_list, 4, handle);
                if (weapon) {
                    auto* data = engine::invoke<void*>(weapon, 461);
                    if (data) {
                        const auto type = read<std::int32_t>(data, 0xC8);
                        if (type == 0 || type == 5)
                            write(view_setup, 0xE0, std::uint16_t(0x0101));
                    }
                }
            }
        }
    }
    reinterpret_cast<engine::MemberFunction<void>>(original_vtable[294])(local);
}
} // namespace skeet::hud

// ============================================================================
// Subcomponent: feature_indicators.cpp
// ============================================================================

namespace skeet::hud {
void draw_freestanding_indicator(ui::RenderQueue& queue, script::Bridge& scripts,
                                 const anti_aim::GlobalConfig& global,
                                 const anti_aim::Config& config, anti_aim::State& state,
                                 const IntegerConfig& feature_indicators,
                                 const esp::RuntimePlayer& local, InputBitmap input,
                                 std::int32_t& cursor, const esp::IndicatorLayout& layout) {
    if (!global.enabled.value)
        return;
    if (local.life_state != 0) {
        state.value_1b4 = -1;
        state.value_1b0 = 0;
        return;
    }
    if (!(std::uint32_t(feature_indicators.value) & 32) || !config.freestanding.value)
        return;
    if (!hotkey_active(config.freestanding_hotkey.value.binding, input))
        return;
    esp::draw_indicator(scripts, queue, cursor, "FS", 0xC8FFFFFFu, layout, -1, -1.0f);
}
void draw_ping_indicator(ui::RenderQueue& queue, script::Bridge& scripts,
                         const IntegerConfig& feature_indicators, const esp::RuntimePlayer& local,
                         const misc::PingSpikeRuntime& ping, std::int32_t& cursor,
                         const esp::IndicatorLayout& layout) {
    if (!(std::uint32_t(feature_indicators.value) & 4) || local.life_state || !ping.active())
        return;
    draw_ping_latency_indicator(queue, scripts, ping, cursor, layout);
}
void draw_ping_latency_indicator(ui::RenderQueue& queue, script::Bridge& scripts,
                                 const misc::PingSpikeRuntime& ping, std::int32_t& cursor,
                                 const esp::IndicatorLayout& layout) {
    auto* channel = engine::invoke<void*>(ping.engine, 78);
    if (!channel)
        return;
    const auto latency = reinterpret_cast<engine::MemberFunction<float, std::int32_t>>(
        (*static_cast<void***>(channel))[9]);
    const auto first = latency(channel, 0);
    const auto second = latency(channel, 1);
    const auto sum = float(static_cast<long double>(second) + static_cast<long double>(first));
    const auto requested =
        _mm_mul_ss(_mm_cvtepi32_ps(_mm_cvtsi32_si128(*ping.milliseconds)), _mm_set_ss(-0.001f));
    auto denominator =
        _mm_sub_sd(_mm_set_sd(*ping.latency_4347a508), _mm_cvtss_sd(_mm_setzero_pd(), requested));
    denominator = _mm_add_sd(denominator, _mm_set_sd(*ping.latency_4347b120));
    const auto divisor =
        _mm_min_ss(_mm_max_ss(_mm_cvtsd_ss(_mm_setzero_ps(), denominator), _mm_set_ss(0.001f)),
                   _mm_set_ss(0.2f));
    const auto ratio = _mm_cvtss_f32(_mm_min_ss(
        _mm_max_ss(_mm_div_ss(_mm_set_ss(sum), divisor), _mm_setzero_ps()), _mm_set_ss(1)));
    std::uint32_t color;
    if (ratio <= 0)
        color = 0xC8FFFFFFu;
    else if (ratio >= 1)
        color = 0xFF15C28Fu;
    else {
        const auto from = ui::rgb_to_hsv(0xC8FFFFFFu), to = ui::rgb_to_hsv(0xFF15C28Fu);
        color = ui::hsv_to_rgb({(to.hue - from.hue) * ratio + from.hue,
                                (to.saturation - from.saturation) * ratio + from.saturation,
                                (to.value - from.value) * ratio + from.value});
    }
    esp::draw_indicator(scripts, queue, cursor, "PING", color, layout, -1, -1.0f);
}
void draw_rage_indicators(ui::RenderQueue& queue, script::Bridge& scripts,
                          const rage::GlobalSettings& global, const rage::AimbotSettings& config,
                          const IntegerConfig& feature_indicators, const esp::RuntimePlayer& local,
                          InputBitmap input, std::int32_t& cursor,
                          const esp::IndicatorLayout& layout,
                          const esp::AimbotVisualizationConfig& visualization,
                          esp::HitboxHighlightState& highlight_state,
                          const esp::HitboxHighlightContext& highlight_context) {
    if (!local.life_state) {
        const auto mask = std::uint32_t(feature_indicators.value);
        if ((mask & 1) && hotkey_active(global.force_safe_point.value.binding, input))
            esp::draw_indicator(scripts, queue, cursor, "SAFE", 0xC8FFFFFFu, layout, -1, -1.0f);
        if ((mask & 2) && hotkey_active(global.force_body_aim.value.binding, input))
            esp::draw_indicator(scripts, queue, cursor, "BODY", 0xC8FFFFFFu, layout, -1, -1.0f);
        if ((mask & 256) && config.damage_override.value &&
            hotkey_active(config.damage_override_key.value.binding, input))
            esp::draw_indicator(scripts, queue, cursor, "MD", 0xC8FFFFFFu, layout, -1, -1.0f);
    }
    esp::draw_hitbox_highlight(queue, visualization, highlight_state, local, highlight_context);
}
#pragma STDC FENV_ACCESS ON
void draw_exploit_indicators(ui::RenderQueue& queue, script::Bridge& scripts,
                             network::ExploitState& state, float& shared_timer,
                             const IntegerConfig& feature_indicators, void* local,
                             const ExploitIndicatorBindings& bindings, float frame_time,
                             std::int32_t& cursor, const esp::IndicatorLayout& layout) {
    state.timer_124 = maximum_number(0.0f, state.timer_124 - frame_time);
    shared_timer = maximum_number(0.0f, shared_timer - frame_time);
    if (!state.mode)
        return;
    std::uint8_t life;
    std::memcpy(&life, static_cast<std::byte*>(local) + bindings.life_state, sizeof(life));
    if (life) {
        shared_timer = 0.0f;
        return;
    }
    const auto mask = std::uint32_t(feature_indicators.value);
    if (state.mode == 1) {
        std::int32_t handle;
        std::memcpy(&handle, static_cast<std::byte*>(local) + bindings.active_weapon,
                    sizeof(handle));
        if (handle == -1)
            return;
        auto* weapon = engine::invoke<void*>(bindings.entity_list, 4, handle);
        if (!weapon || !(mask & 8))
            return;
        bool unavailable = state.available < state.limit;
        if (!unavailable) {
            std::uint16_t definition;
            std::memcpy(&definition, static_cast<std::byte*>(weapon) + bindings.weapon_definition,
                        sizeof(definition));
            unavailable = definition == 31;
        }
        esp::draw_indicator(scripts, queue, cursor, "DT", unavailable ? 0xFF3200FFu : 0xC8FFFFFFu,
                            layout, -1, -1.0f);
    } else if (state.mode == 2 && (mask & 64))
        esp::draw_indicator(scripts, queue, cursor, "OSAA", 0xC8FFFFFFu, layout, -1, -1.0f);
}
#pragma STDC FENV_ACCESS OFF
} // namespace skeet::hud

// ============================================================================
// Subcomponent: movement_indicators.cpp
// ============================================================================

namespace skeet::hud {
void draw_movement_indicators(ui::RenderQueue& queue, script::Bridge& scripts,
                              const IntegerConfig& feature_indicators, std::uint8_t duck_enabled,
                              const rage::GlobalSettings& rage, std::uint32_t flags,
                              const QuickPeekConfig& quick_peek, MovementIndicatorState state,
                              const esp::RuntimePlayer& local, InputBitmap input,
                              std::int32_t& cursor, const esp::IndicatorLayout& layout,
                              const esp::Projection& projection) {
    if (local.life_state) {
        reset_quick_peek_state(state.quick_peek);
        state.duck_target_52c = 0.0f;
        state.byte_51c = 0;
        state.crouch_active_544 = 0;
        state.value_540 = 27;
        return;
    }
    if ((std::uint32_t(feature_indicators.value) & 16) && duck_enabled) {
        bool active = (flags & 0x40) != 0;
        if (!active)
            active =
                hotkey_active(rage.duck_peek_assist.value.binding, input) || (flags & 0x100) != 0;
        if (active)
            esp::draw_indicator(scripts, queue, cursor, "DUCK", 0xC8FFFFFFu, layout, -1, -1.0f);
    }
    draw_quick_peek(quick_peek, input, state.quick_peek, projection, queue);
}
} // namespace skeet::hud

// ============================================================================
// Subcomponent: observer_panel.cpp
// ============================================================================
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

namespace skeet::hud {
void draw_observer_panel(ui::RenderQueue& queue, esp::PlayerFrameState& state,
                         const ObserverEngine& engine, const esp::RuntimePlayer* initial_view,
                         const esp::RuntimePlayer* observer, std::int32_t entity_index,
                         std::int32_t screen_width) {
    if (!initial_view || !observer || observer->observer_handle == -1)
        return;
    auto* target = engine.players.by_handle(engine.players.context, observer->observer_handle);
    if (!target)
        return;
    auto* view = esp::effective_view_player(engine.players);
    if (!view)
        return;
    const auto view_index = engine.players.index(engine.players.context, *view);
    const auto target_index = engine.players.index(engine.players.context, *target);
    if (target_index != view_index)
        return;
    ObserverPlayerInfo info;
    if (!engine.player_info(engine.context, entity_index, info) || info.hltv || info.fake_player)
        return;
    info.name[20] = '\0';
    std::array<wchar_t, 256> wide;
    const auto count = MultiByteToWideChar(CP_UTF8, 0, info.name.data(), -1, wide.data(),
                                           static_cast<int>(wide.size()));
    if (count <= 1)
        return;
    const auto row = std::uint32_t(state.transient);
    state.transient = std::bit_cast<std::int32_t>(row + 1);
    const auto x = std::bit_cast<std::int32_t>(std::uint32_t(screen_width) - 10);
    const auto y = std::bit_cast<std::int32_t>(row * 20 + 5);
    static_assert(sizeof(wchar_t) == sizeof(char16_t));
    queue.text(x, y, 0xC8FFFFFFu, 0x88, 0,
               {reinterpret_cast<const char16_t*>(wide.data()), std::size_t(count - 1)});
}
void draw_prepared_observer(ui::RenderQueue& queue, esp::PlayerFrameState& state,
                            const ObserverEngine& engine, const esp::RuntimePlayer* initial_view,
                            const esp::PreparedPlayer& prepared, std::int32_t screen_width) {
    if (prepared.stage == esp::PreparedPlayerStage::observer_panel)
        draw_observer_panel(queue, state, engine, initial_view, prepared.player,
                            prepared.entity_index, screen_width);
}
} // namespace skeet::hud

// ============================================================================
// Subcomponent: penetration_reticle.cpp
// ============================================================================

namespace skeet::hud {
void draw_penetration_reticle(ui::RenderQueue& queue, const esp::ScreenOverlayConfig& config,
                              const PenetrationReticleState& state, void* local,
                              const PenetrationReticleBindings& bindings, Point screen_size) {
    if (!local)
        return;
    std::uint8_t life;
    std::memcpy(&life, static_cast<std::byte*>(local) + bindings.life_state, sizeof(life));
    if (life || !config.penetration_reticle.value || !state.ready)
        return;
    std::int32_t handle;
    std::memcpy(&handle, static_cast<std::byte*>(local) + bindings.active_weapon, sizeof(handle));
    if (handle == -1)
        return;
    auto* weapon = engine::invoke<void*>(bindings.entity_list, 4, handle);
    if (!weapon)
        return;
    std::uint16_t definition;
    std::memcpy(&definition, static_cast<std::byte*>(weapon) + bindings.weapon_definition,
                sizeof(definition));
    if (!lagcomp::history_weapon_is_firearm(definition))
        return;
    const auto x = screen_size.x / 2, y = screen_size.y / 2;
    queue.rectangle({x - 1, y - 1, 3, 3, 0x96000000u});
    queue.rectangle({x, y - 1, 1, 3, state.color});
    queue.rectangle({x - 1, y, 3, 1, state.color});
}
} // namespace skeet::hud

// ============================================================================
// Subcomponent: penetration_reticle_owner.cpp
// ============================================================================

namespace skeet::hud {
using esp::Vertex;
using esp::integer_float;
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define RETICLE_MEMBER __thiscall
#define RETICLE_FAST __fastcall
#else
#define RETICLE_MEMBER
#define RETICLE_FAST
#endif
using OwnerEntry = void(RETICLE_MEMBER*)(void*);
struct PenetrationReticleInstance;
struct RenderEndpoint : HudFrameCallback {
    PenetrationReticleInstance* owner;
};
void RETICLE_MEMBER destruction(void*);
void RETICLE_MEMBER reticle_unchanged(void*) {} //433CCE8E
void RETICLE_MEMBER reset(void*);
void RETICLE_MEMBER refresh(void*);
void RETICLE_FAST draw(void*, ui::RenderQueue*, void*, std::int32_t*);
const std::array<OwnerEntry, 9> primary_table{
    destruction, reticle_unchanged, reticle_unchanged, reticle_unchanged, reticle_unchanged, reset, reticle_unchanged, refresh, reticle_unchanged};
const std::array<HudFrameEntry, 1> render_table{draw};

struct PenetrationReticleInstance {
    const OwnerEntry* vtable = nullptr;
    client::NativeCommandPhaseReceiver command_receiver;
    RenderEndpoint render{{nullptr}, this};
    client::PenetrationReticleCommandOwner& command;
    const PenetrationHudBinding& rendering;
    const engine::JobMemory& memory;

    PenetrationReticleInstance(const PenetrationReticleOwnerRegistries& registries,
                               ConfigRegistries& config, esp::ScreenOverlayConfig& settings,
                               client::PenetrationReticleCommandOwner& producer,
                               PenetrationReticleEnvironment& environment,
                               void* const& engine_trace, const PenetrationHudBinding& visual)
        : command(producer), rendering(visual), memory(registries.lifecycle.memory) {
        //434824D1..43482511: all three appends precede table publication and
        // descriptor construction. The command registry invokes the existing
        // concrete4334B1DA producer, sharing this same state/environment.
        registries.lifecycle.append(this);
        registries.commands.append(command_receiver, &command);
        register_hud_frame(registries.hud, &render);
        vtable = primary_table.data();
        registries.commands.publish(command_receiver,
                                    client::CommandOwnerId::penetration_reticle); //434824F4
        render.vtable = render_table.data();
        initialize_boolean_config(config.descriptors, settings.penetration_reticle, 0x1E7432A5u);
        command.state.color = 0;
        //4348251A is the recorded restored instruction A1 04 83 46 43:
        //MOV EAX,[43468304], followed by MOV[EBX+30],EAX. The constructor
        //does not write ready+2C; the reset method owns that separate write.
        environment.point_contents = engine_trace;
    }
};
static_assert(sizeof(PenetrationReticleInstance) <= penetration_reticle_native_owner_size);
void RETICLE_MEMBER destruction(void* storage) {
    destroy_penetration_reticle_owner(storage);
}
void RETICLE_MEMBER reset(void* storage) {
    reset_penetration_reticle_owner(storage);
}
void RETICLE_MEMBER refresh(void* storage) {
    refresh_penetration_reticle_owner(storage);
}
void RETICLE_FAST draw(void* callback, ui::RenderQueue* queue, void* player, std::int32_t* cursor) {
    auto& owner = *static_cast<RenderEndpoint*>(static_cast<HudFrameCallback*>(callback))->owner;
    owner.rendering.draw(*queue, player, *cursor);
}
void* allocate(engine::JobMemory memory) {
    constexpr auto length = penetration_reticle_native_owner_size;
    auto* const raw = memory.allocate(memory.process_heap, 8, length + 0x20);
    if (!raw)
        return nullptr;
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15) & ~std::uintptr_t(15);
    const auto displacement = std::uint32_t(header - address), total = std::uint32_t(length + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &total, 4);
    return reinterpret_cast<void*>(header + 16);
}
#undef RETICLE_MEMBER
#undef RETICLE_FAST
} //namespace
std::size_t penetration_reticle_owner_size() noexcept {
    return penetration_reticle_native_owner_size;
}
void construct_penetration_reticle_owner(
    void* storage, const PenetrationReticleOwnerRegistries& registries, ConfigRegistries& config,
    esp::ScreenOverlayConfig& settings, client::PenetrationReticleCommandOwner& command,
    PenetrationReticleEnvironment& environment, void* const& engine_trace,
    const PenetrationHudBinding& rendering) {
    ::new (storage) PenetrationReticleInstance(registries, config, settings, command, environment,
                                               engine_trace, rendering);
}
void* allocate_penetration_reticle_owner_storage(engine::JobMemory memory) {
    return allocate(memory);
}
void construct_allocated_penetration_reticle_owner(
    void* storage, const PenetrationReticleOwnerRegistries& registries, ConfigRegistries& config,
    esp::ScreenOverlayConfig& settings, client::PenetrationReticleCommandOwner& command,
    PenetrationReticleEnvironment& environment, void* const& engine_trace,
    const PenetrationHudBinding& rendering) {
    settings.penetration_reticle = BooleanConfig{};
    command.state = PenetrationReticleState{};
    construct_penetration_reticle_owner(storage, registries, config, settings, command, environment,
                                        engine_trace, rendering);
}
void destroy_penetration_reticle_owner(void* storage) {
    if (!storage)
        return;
    auto* const owner = static_cast<PenetrationReticleInstance*>(storage);
    const auto memory = owner->memory;
    owner->~PenetrationReticleInstance();
    auto* const bytes = static_cast<std::byte*>(storage);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
void reset_penetration_reticle_owner(void* storage) {
    auto& state = static_cast<PenetrationReticleInstance*>(storage)->command.state;
    state.color = 0xFF3200FFu;
    state.ready = false;
}
void refresh_penetration_reticle_owner(void* storage) {
    static_cast<PenetrationReticleInstance*>(storage)->vtable[5](storage);
}
} //namespace skeet::hud

// ============================================================================
// Subcomponent: penetration_runtime.cpp
// ============================================================================

namespace skeet::hud {
using esp::Vertex;
using esp::integer_float;
namespace {
/* [removed duplicate field] */
void tick_field(void* object, std::ptrdiff_t offset, std::int32_t value) {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(value));
}
std::int32_t absolute_difference(std::int32_t first, std::int32_t second) noexcept {
    const auto difference = std::uint32_t(first) - std::uint32_t(second);
    const auto sign = std::bit_cast<std::int32_t>(difference) < 0 ? UINT32_MAX : 0u;
    return std::bit_cast<std::int32_t>((difference ^ sign) - sign);
}
// The first0x24 bytes are the native4346F784 filter prefix. A real engine
// TraceRay receives this object and invokes the source predicate through its
// actual member-function ABI; the remaining four slots retain engine methods.
struct ReticleTraceFilter {
    void** vtable;
    std::array<std::uint32_t, 5> fields_04{};
    std::uint32_t mask = 0x4600400b, exit_mask = 0x4600400b;
    float minimum_damage = 1.0f;
    const PenetrationReticleEnvironment* context;
    std::array<void*, 5> methods;
    ReticleTraceFilter(const PenetrationReticleEnvironment& bindings, BallisticEntity preferred)
        : vtable(nullptr), context(&bindings) {
        if (!context->trace_filter_vtable)
            throw std::logic_error("Required engine trace filter table is not bound");
        fields_04[4] = preferred;
        const auto base = context->trace_filter_vtable;
        methods = {reinterpret_cast<void*>(should_hit), base[1], base[2], base[3], base[4]};
        vtable = methods.data();
    }
#if defined(_M_IX86) || defined(__i386__)
    static bool __thiscall should_hit(void* receiver, BallisticEntity entity, std::uint32_t mask){
#else
    static bool should_hit(void* receiver, BallisticEntity entity, std::uint32_t mask) {
#endif
        auto& self = *static_cast<ReticleTraceFilter*>(receiver);
    if (entity && entity != self.fields_04[4]) {
        auto* current = engine::invoke<void*>(reinterpret_cast<void*>(std::uintptr_t(entity)), 7);
        while (current) {
            if (field<std::int32_t>(current, self.context->collision_group) == 11 ||
                engine::invoke<bool>(current, 158))
                return false;
            const auto handle = field<std::int32_t>(current, self.context->move_parent_handle);
            if (handle == -1)
                break;
            current = engine::invoke<void*>(self.context->targeting.entity_list, 4, handle);
        }
    }
    const auto delegate =
        reinterpret_cast<engine::MemberFunction<bool, BallisticEntity, std::uint32_t>>(
            self.context->trace_filter_vtable[0]);
    return delegate(receiver, entity, mask);
} BallisticFilter binding() noexcept {
    return {this, fields_04[4], mask, exit_mask, minimum_damage};
}
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(offsetof(ReticleTraceFilter, mask) == 0x18);
static_assert(offsetof(ReticleTraceFilter, minimum_damage) == 0x20);
#endif
// All bullet/exit/penetration decisions stay in the recovered implementations.
// Software entity clipping and engine TraceRay both invoke the same ABI filter.
class ReticleBallistics final : public BallisticEngine {
    BallisticEngine& inner;

public:
    explicit ReticleBallistics(BallisticEngine& engine) : inner(engine) {}
    std::int32_t filter_type(const BallisticFilter& f) override {
        return engine::invoke<std::int32_t>(f.object, 1);
    }
    void bind_source(const BallisticFilter& f, BallisticEntity e) override {
        engine::invoke<void>(f.object, 2, e);
    }
    void bind_last_hit(const BallisticFilter& f, BallisticEntity e) override {
        engine::invoke<void>(f.object, 4, e);
    }
    bool accepts(const BallisticFilter& f, BallisticEntity entity, std::uint32_t mask) override {
        return engine::invoke<bool>(f.object, 0, entity, mask);
    }
    std::uint32_t point_contents(const BallisticVector& p, std::uint32_t m, bool w) override {
        return inner.point_contents(p, m, w);
    }
    void trace_ray(const BallisticTraceRequest& r, BallisticTrace& t) override {
        inner.trace_ray(r, t);
    }
    void clip_ray_to_entity(const BallisticRay& r, std::uint32_t m, BallisticEntity e,
                            BallisticTrace& t) override {
        inner.clip_ray_to_entity(r, m, e, t);
    }
    PenetrationSurface surface(std::int16_t p) override {
        return inner.surface(p);
    }
    BallisticEntity entity(std::int32_t i) override {
        return inner.entity(i);
    }
    std::int32_t maximum_clients() override {
        return inner.maximum_clients();
    }
    BallisticEntityInfo entity_info(BallisticEntity e) override {
        return inner.entity_info(e);
    }
    BallisticEntityInfo clip_entity_info(BallisticEntity e) override {
        return inner.clip_entity_info(e);
    }
    BallisticBreakableInfo breakable_info(BallisticEntity e) override {
        return inner.breakable_info(e);
    }
    BallisticModelInfo model_info(std::int32_t i) override {
        return inner.model_info(i);
    }
    std::uint8_t damage_mode(BallisticEntity e) override {
        return inner.damage_mode(e);
    }
    void set_damage_mode(BallisticEntity e, std::uint8_t m) override {
        inner.set_damage_mode(e, m);
    }
    bool has_engine_breakable_callback() override {
        return inner.has_engine_breakable_callback();
    }
    bool engine_breakable(BallisticEntity e) override {
        return inner.engine_breakable(e);
    }
    BallisticRules rules() override {
        return inner.rules();
    }
};
}
void* closest_penetration_target(const rage::TargetBindings& bindings, std::int32_t maximum_clients,
                                 const RageVector& origin, const RageVector& direction,
                                 float maximum_distance) { // 0x4332AA97
    void* selected = nullptr;
    for (std::int32_t index = 1; index <= maximum_clients; ++index) {
        auto* target = engine::invoke<void*>(bindings.entity_list, 3, index);
        if (!rage::target_is_eligible(bindings, target))
            continue;
        const auto bottom = field<RageVector>(target, bindings.offsets.origin);
        const auto x = bottom[0] - origin[0], y = bottom[1] - origin[1], z = bottom[2] - origin[2];
        const auto magnitude = std::sqrt((x * x + z * z) + y * y);
        const auto cosine =
            ((direction[1] * y + direction[0] * x) + direction[2] * z) / (magnitude + 0x1p-23f);
        if (0.0f > cosine)
            continue;
        auto top = bottom;
        top[2] += (*engine::invoke<const RageVector*>(target, 77))[2];
        const auto closest = closest_rage_ray_segment({origin, direction}, {bottom, top});
        const auto other = 1.0f - closest.second_fraction;
        const auto dx =
            closest.first_point[0] - (bottom[0] * other + top[0] * closest.second_fraction);
        const auto dy =
            closest.first_point[1] - (bottom[1] * other + top[1] * closest.second_fraction);
        const auto dz =
            closest.first_point[2] - (bottom[2] * other + top[2] * closest.second_fraction);
        const auto distance = std::sqrt((dx * dx + dy * dy) + dz * dz);
        if (!(distance >= maximum_distance)) {
            maximum_distance = distance;
            selected = target;
        }
    }
    return selected;
}
void update_penetration_reticle(PenetrationReticleState& state,
                                const esp::ScreenOverlayConfig& config,
                                const PenetrationReticleFrame& frame,
                                const PenetrationReticleEnvironment& context) { // 0x4334B1DA
    state.color = 0xff3200ff;
    state.ready = false;
    if (!config.penetration_reticle.value || !frame.weapon)
        return;
    const auto identifier =
        field<std::uint16_t>(frame.weapon, context.targeting.offsets.weapon_identifier);
    if (!lagcomp::history_weapon_is_firearm(identifier))
        return;
    auto* closest = closest_penetration_target(context.targeting, *context.maximum_clients,
                                               frame.origin, frame.direction, 60.0f);
    void* changed = nullptr;
    std::int32_t saved_tick = 0;
    if (closest && context.predicted_tick) {
        saved_tick = field<std::int32_t>(closest, context.target_tick);
        if (absolute_difference(saved_tick, *context.predicted_tick) < 64) {
            tick_field(closest, context.target_tick, *context.predicted_tick);
            changed = closest;
        }
    }
    auto query = rage::prepare_target_bullet(frame.weapon,
                                             context.targeting.offsets.weapon_identifier, true);
    ReticleTraceFilter native_filter(context,
                                     std::uint32_t(reinterpret_cast<std::uintptr_t>(closest)));
    auto filter = native_filter.binding();
    ReticleBallistics filtered_engine(context.ballistics);
    const auto source = std::uint32_t(reinterpret_cast<std::uintptr_t>(frame.local_player));
    const auto result = rage::trace_target_direction(filtered_engine, source, frame.origin,
                                                     frame.direction, filter, query);
    auto targeting = context.targeting;
    targeting.local_player = frame.local_player;
    auto* hit = reinterpret_cast<void*>(std::uintptr_t(query.trace.entity));
    if (result != 0 && hit && engine::invoke<bool>(hit, 158) &&
        rage::target_is_enemy(targeting, hit)) {
        state.color = 0xffff6012;
        state.ready = true;
    } else {
        state.ready = true;
        if (std::bit_cast<std::int32_t>(query.point_count) > 0) {
            const auto& first = query.points[0];
            const auto& last = query.trace.end;
            const auto x = first[0] - last[0], y = first[1] - last[1], z = first[2] - last[2];
            if ((y * y + x * x) + z * z >= 1000.0f) {
                const RageVector midpoint{(last[0] - first[0]) * 0.5f + first[0],
                                          (last[1] - first[1]) * 0.5f + first[1],
                                          (last[2] - first[2]) * 0.5f + first[2]};
                if (engine::invoke<std::int32_t>(context.point_contents, 0, &midpoint, 0x10001u,
                                                 static_cast<void**>(nullptr)) == 0)
                    state.color = 0xff32ff00;
            }
        }
    }
    if (changed)
        tick_field(changed, context.target_tick, saved_tick);
}
}

// ============================================================================
// Subcomponent: screen_overlay.cpp
// ============================================================================

namespace skeet::hud {
using esp::Vertex;
using esp::integer_float;
namespace {
/* [removed duplicate Vertex] */
// Positive radii enter433EC12E after multiplication by pi. That CRT helper
// truncates modulo2^32, returning zero for non-finite values.
std::uint32_t native_integer(float value) {
    const auto bits = std::bit_cast<std::uint32_t>(value);
    const auto exponent = std::int32_t((bits >> 23) & 255) - 127;
    if (exponent < 0 || exponent >= 55 || (bits & 0x7f800000u) == 0x7f800000u)
        return 0;
    const auto mantissa = (bits & 0x007fffffu) | 0x00800000u;
    const auto integer = exponent >= 23 ? mantissa << std::uint32_t(exponent - 23)
                                        : mantissa >> std::uint32_t(23 - exponent);
    return bits >> 31 ? 0u - integer : integer;
}
/* [removed duplicate integer_float] */
std::int32_t displacement(std::int32_t pixel, float center) {
    const auto difference = _mm_cvttss_si32(_mm_set_ss(integer_float(pixel) - center));
    const auto sign = std::uint32_t(difference >> 31);
    const auto absolute = std::bit_cast<std::int32_t>((std::uint32_t(difference) ^ sign) - sign);
    return absolute >= 2 ? difference : 0;
}
}
void draw_screen_overlay(ui::RenderQueue& queue, const esp::ScreenOverlayConfig& config,
                         const ScreenOverlayState& state, void* local, std::uint32_t life_offset,
                         Point screen_size, const esp::Projection& projection,
                         const RageVector& origin, const RageTrigonometry& trig) {
    if ((!config.inaccuracy.value && !config.recoil.value) || !local)
        return;
    std::uint8_t life;
    std::memcpy(&life, static_cast<std::byte*>(local) + life_offset, sizeof(life));
    if (life)
        return;
    float x = integer_float(screen_size.x >> 1), y = integer_float(screen_size.y >> 1);
    if (config.recoil.value && state.byte_6c && !*state.byte_58) {
        const auto* angle = engine::invoke<const float*>(local, 171);
        const std::array<float, 3> copy{angle[0], angle[1], angle[2]};
        const auto yaw = (state.recoil_yaw_64 + copy[1]) * 0.01745329238474369f;
        const auto sy = trig.sine(yaw), cy = trig.cosine(yaw);
        const auto pitch = (state.recoil_pitch_60 + copy[0]) * 0.01745329238474369f;
        const auto sp = trig.sine(pitch), cp = trig.cosine(pitch);
        const esp::Vector3 point{(cp * cy) * 10000.0f + origin[0], (cp * sy) * 10000.0f + origin[1],
                                 (-sp) * 10000.0f + origin[2]};
        Point projected;
        if (esp::project_point(projection, point, projected)) {
            x += integer_float(displacement(projected.x, x));
            y += integer_float(displacement(projected.y, y));
        }
    }
    float radius;
    std::uint32_t segments;
    if (config.inaccuracy.value) {
        radius = state.radius_5c;
        if (radius < 4.0f)
            return;
        segments = std::min(native_integer(radius * 3.1415927410125732f), 512u);
    } else {
        if (!state.byte_6c)
            return;
        radius = 10.0f;
        segments = 31;
    }
    const auto step = float(6.283185307179586 / double(float(segments)));
    auto sine = trig.sine(0), cosine = trig.cosine(0);
    auto last_x = cosine * radius + x + 0.5f, last_y = sine * radius + y + 0.5f;
    const auto color = config.inaccuracy_color.value;
    std::vector<Vertex> vertices;
    vertices.reserve(std::size_t(segments) * 3);
    for (std::uint32_t i = 0; i < segments; ++i) {
        vertices.push_back({last_x, last_y, color});
        const auto angle = float(i + 1) * step;
        sine = trig.sine(angle);
        cosine = trig.cosine(angle);
        last_x = cosine * radius + x + 0.5f;
        last_y = sine * radius + y + 0.5f;
        vertices.push_back({last_x, last_y, color});
        vertices.push_back({x, y, color & 0x00FFFFFFu});
    }
    const auto count = segments * 3;
    auto* payload = queue.append(7, count * sizeof(Vertex) + 16);
    std::memcpy(payload, &count, sizeof(count));
    if (!vertices.empty())
        std::memcpy(payload + 4, vertices.data(), vertices.size() * sizeof(Vertex));
}
} // namespace skeet::hud

// ============================================================================
// Subcomponent: screen_overlay_owner.cpp
// ============================================================================

namespace skeet::hud {
using esp::Vertex;
using esp::integer_float;
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define OVERLAY_MEMBER __thiscall
#define OVERLAY_FAST __fastcall
#else
#define OVERLAY_MEMBER
#define OVERLAY_FAST
#endif
using OwnerEntry = void(OVERLAY_MEMBER*)(void*);
struct ScreenOverlayInstance;
struct ScreenOverlayEndpoint : HudFrameCallback {
    ScreenOverlayInstance* owner;
};
void OVERLAY_MEMBER overlay_destruction(void*);
void OVERLAY_MEMBER overlay_bind(void*);
void OVERLAY_MEMBER overlay_unchanged(void*) {} //433CCE8E
void OVERLAY_MEMBER overlay_reset_forwarder(void*);
void OVERLAY_MEMBER overlay_reset(void*);
void OVERLAY_FAST overlay_draw(void*, ui::RenderQueue*, void*, std::int32_t*);
const std::array<OwnerEntry, 9> overlay_primary_table{overlay_destruction, overlay_bind,      overlay_unchanged,
                                                      overlay_unchanged,   overlay_unchanged, overlay_reset_forwarder,
                                                      overlay_unchanged,   overlay_reset,     overlay_unchanged};
const std::array<HudFrameEntry, 1> overlay_render_table{overlay_draw};

struct ScreenOverlayInstance {
    const OwnerEntry* vtable = nullptr;
    client::NativeCommandPhaseReceiver command_receiver;
    ScreenOverlayEndpoint render{{nullptr}, this};
    client::ScreenOverlayCommandOwner& command;
    const ScreenOverlayHudBinding& rendering;
    ConfigRegistries& configuration;
    const engine::JobMemory& memory;
    const std::int32_t* override_fov_50;
    const std::int32_t* override_zoom_fov_54;

    ScreenOverlayInstance(const ScreenOverlayOwnerRegistries& registries, ConfigRegistries& config,
                          esp::ScreenOverlayConfig& settings,
                          client::ScreenOverlayCommandOwner& producer,
                          const ScreenOverlayHudBinding& visual)
        : command(producer), rendering(visual), configuration(config),
          memory(registries.lifecycle.memory) {
        //4348DBD9..4348DC04. Every registry append precedes table publication
        // and descriptor construction; the existing command dispatcher owns
        // the actual433361BA phase body and its live history-owner cell.
        registries.lifecycle.append(this);
        registries.commands.append(command_receiver, &command);
        register_hud_frame(registries.hud, &render);
        vtable = overlay_primary_table.data();
        registries.commands.publish(command_receiver,
                                    client::CommandOwnerId::screen_overlay); //4348DBFC
        render.vtable = overlay_render_table.data();
        initialize_boolean_config(config.descriptors, settings.recoil, 0x78A1B150u);
        initialize_boolean_config(config.descriptors, settings.inaccuracy, 0xEDF6C582u);
        initialize_color_config(config.descriptors, settings.inaccuracy_color, 0x2B61D25Au,
                                0x3C1E1E1Eu);
        override_fov_50 = nullptr;
        override_zoom_fov_54 = nullptr;
        command.state.byte_58 = nullptr;
        command.state.radius_5c = 0.0f;
        //4348DC58 clears AL-sized storage only. The other pitch bytes and
        // the entire yaw/roll fields retain the incoming canonical state.
        const std::uint8_t zero = 0;
        std::memcpy(&command.state.recoil_pitch_60, &zero, sizeof zero);
        command.state.byte_6c = false;
    }
};
static_assert(sizeof(ScreenOverlayInstance) <= screen_overlay_native_owner_size);
void OVERLAY_MEMBER overlay_destruction(void* storage) {
    destroy_screen_overlay_owner(storage);
}
void OVERLAY_MEMBER overlay_bind(void* storage) {
    bind_screen_overlay_owner_settings(storage);
}
void OVERLAY_MEMBER overlay_reset_forwarder(void* storage) { //433B06E3 ->current primaryslot7
    static_cast<ScreenOverlayInstance*>(storage)->vtable[7](storage);
}
void OVERLAY_MEMBER overlay_reset(void* storage) {
    reset_screen_overlay_state(static_cast<ScreenOverlayInstance*>(storage)->command.state);
}
void OVERLAY_FAST overlay_draw(void* callback, ui::RenderQueue* queue, void* player, std::int32_t* cursor) {
    auto& owner = *static_cast<ScreenOverlayEndpoint*>(static_cast<HudFrameCallback*>(callback))->owner;
    owner.rendering.draw(*queue, player, *cursor);
}
void* allocate(engine::JobMemory memory, std::size_t length) {
    auto* const raw = memory.allocate(memory.process_heap, 8, length + 0x20);
    if (!raw)
        return nullptr;
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15) & ~std::uintptr_t(15);
    const auto displacement = std::uint32_t(header - address), total = std::uint32_t(length + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &total, 4);
    return reinterpret_cast<void*>(header + 16);
}
#undef OVERLAY_MEMBER
#undef OVERLAY_FAST
} //namespace
std::size_t screen_overlay_owner_size() noexcept {
    return screen_overlay_native_owner_size;
}
void construct_screen_overlay_owner(void* storage, const ScreenOverlayOwnerRegistries& registries,
                                    ConfigRegistries& config, esp::ScreenOverlayConfig& settings,
                                    client::ScreenOverlayCommandOwner& command,
                                    const ScreenOverlayHudBinding& rendering) {
    ::new (storage) ScreenOverlayInstance(registries, config, settings, command, rendering);
}
void* allocate_screen_overlay_owner_storage(engine::JobMemory memory) {
    return allocate(memory, screen_overlay_native_owner_size);
}
void construct_allocated_screen_overlay_owner(void* storage,
                                              const ScreenOverlayOwnerRegistries& registries,
                                              ConfigRegistries& config,
                                              esp::ScreenOverlayConfig& settings,
                                              client::ScreenOverlayCommandOwner& command,
                                              const ScreenOverlayHudBinding& rendering) {
    settings.recoil = BooleanConfig{};
    settings.inaccuracy = BooleanConfig{};
    settings.inaccuracy_color = ColorConfig{};
    command.state = ScreenOverlayState{};
    construct_screen_overlay_owner(storage, registries, config, settings, command, rendering);
}
void destroy_screen_overlay_owner(void* storage) {
    if (!storage)
        return; //433837D0's native null branch
    auto* const owner = static_cast<ScreenOverlayInstance*>(storage);
    const auto memory = owner->memory;
    owner->~ScreenOverlayInstance();
    auto* const bytes = static_cast<std::byte*>(storage);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
void bind_screen_overlay_owner_settings(void* storage) {
    auto& owner = *static_cast<ScreenOverlayInstance*>(storage);
    auto& descriptors = owner.configuration.descriptors;
    owner.override_fov_50 =
        &static_cast<IntegerConfig*>(find_config(descriptors, 0xFC225A4Fu))->value;
    owner.override_zoom_fov_54 =
        &static_cast<IntegerConfig*>(find_config(descriptors, 0xCA80F9EDu))->value;
    owner.command.state.byte_58 =
        &static_cast<BooleanConfig*>(find_config(descriptors, 0x63660060u))->value;
}
} //namespace skeet::hud

// ============================================================================
// Subcomponent: screen_overlay_update.cpp
// ============================================================================

namespace skeet::hud {
using esp::Vertex;
using esp::integer_float;
namespace {
/* [removed duplicate field] */
template <class T> void assign(void* object, std::ptrdiff_t offset, T value) {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof value);
}
const void* parent(const void* variable) {
    for (;;) {
        const auto next = field<const void*>(variable, 0x1C);
        if (variable == next)
            return variable;
        variable = next;
    }
}
float variable_float(const void* variable) {
    variable = parent(variable);
    return std::bit_cast<float>(
        field<std::uint32_t>(variable, 0x2C) ^
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(variable)));
}
/* [removed duplicate variable_integer] */
void* find_variable(void* cvar) { // complete4332A793 for hash3EEAB5A6
    for (auto node = field<void*>(cvar, 0x30); node; node = field<void*>(node, sizeof(void*)))
        if (events::event_hash(engine::invoke<const char*>(node, 6)) == 0x3EEAB5A6)
            return node;
    return nullptr;
}
bool supported_weapon(std::uint16_t definition) { // complete433D24BB
    constexpr std::uint32_t words[] = {0xFF8B6F9E, 0xB00001DF, 1};
    return definition < 65 &&
           (words[definition >> 5] & (std::uint32_t{1} << (definition & 31))) != 0;
}
float negate(float value) {
    return std::bit_cast<float>(std::bit_cast<std::uint32_t>(value) ^ 0x80000000);
}
float absolute(float value) {
    return std::bit_cast<float>(std::bit_cast<std::uint32_t>(value) & 0x7FFFFFFF);
}
std::int32_t decrement(std::int32_t value) {
    return std::bit_cast<std::int32_t>(std::uint32_t(value) - 1);
}
}
void bind_screen_overlay_recoil_config(ScreenOverlayState& state,
                                       const rage::GlobalSettings& settings) noexcept {
    state.byte_58 = &settings.remove_recoil.value;
}
void reset_screen_overlay_state(ScreenOverlayState& state) noexcept {
    state.radius_5c = 0;
    if (state.byte_6c)
        state.byte_6c = false;
}
ScreenOverlayCommandFrame screen_overlay_command_frame(const void* context) {
    ScreenOverlayCommandFrame result{};
    result.flags = field<std::uint32_t>(context, 0);
    result.weapon = field<void*>(context, 0xC);
    result.recoil_view = {field<float>(context, 0x9C), field<float>(context, 0xA0)};
    result.spread.inaccuracy = field<float>(context, 0xC4);
    result.spread.spread = field<float>(context, 0xD0);
    return result;
}
void ScreenOverlayPublisher::publish(ScreenOverlayState& state,
                                     const esp::ScreenOverlayConfig& config,
                                     const ScreenOverlayCommandFrame& frame, void* local,
                                     const client::Command& command,
                                     const ScreenOverlayFields& fields,
                                     const ScreenOverlayUpdateBindings& bindings) {
    if (state.byte_6c)
        state.byte_6c = false;
    state.radius_5c = 0;
    if (config.recoil.value && frame.weapon && !*state.byte_58 &&
        supported_weapon(field<std::uint16_t>(frame.weapon, fields.weapon_definition)) &&
        !(1.0f > field<float>(frame.weapon, fields.weapon_recoil_index))) {
        const auto length = frame.recoil_view[0] * frame.recoil_view[0] +
                            frame.recoil_view[1] * frame.recoil_view[1];
        if (!(0.001f >= length)) {
            auto angle = field<RageVector>(local, fields.recoil_angles);
            auto velocity = field<RageVector>(local, fields.recoil_velocity);
            const auto weapon_time = field<float>(frame.weapon, fields.weapon_time_f71ce331);
            auto current_time = *bindings.current_time;
            const auto time_delta = weapon_time - current_time;
            if (time_delta > negate(*bindings.interval) && time_delta < 0.75f) {
                const auto local_time = field<float>(local, fields.player_time_0176a381);
                const auto age = current_time - local_time;
                if (age >= 0 && age < 2.0f) {
                    while (current_time < weapon_time) {
                        const auto decay =
                            negate(variable_float(bindings.first_decay) * 2.3025851249694824f) *
                            *bindings.receive_interval;
                        // The caller makes this CRT call even though4336B421
                        // replaces XMM0 before consuming it.
                        (void)bindings.exponential(decay);
                        legit::decay_recoil(velocity, angle,
                                            {bindings.linear_decay, bindings.angle_decay,
                                             bindings.velocity_decay, *bindings.interval,
                                             bindings.exponential});
                        current_time += *bindings.interval;
                    }
                    auto previous = field<RageVector>(local, fields.recoil_angles);
                    const auto* identity = engine::invoke<const std::uint32_t*>(frame.weapon, 2);
                    auto sequence = decrement(command.number);
                    for (unsigned scanned = 0; scanned < 150;
                         ++scanned, sequence = decrement(sequence)) {
                        const auto* record =
                            prediction::CommandHistoryView::from_owner(bindings.history_owner)
                                .find(sequence);
                        if (!record)
                            continue;
                        if (record->weapon_identity != *identity || record->timestamp != local_time)
                            break;
                        previous = record->recoil_angles;
                    }
                    const auto difference = absolute(current_time - local_time);
                    if (difference != 0) {
                        auto fraction = (*bindings.current_time - local_time) / difference;
                        fraction = fraction > 0 ? fraction : 0;
                        fraction = fraction < 1 ? fraction : 1;
                        if (angle[0] == previous[0] && angle[1] == previous[1] &&
                            angle[2] == previous[2])
                            angle = previous;
                        else {
                            const auto first =
                                lagcomp::history_angle_quaternion(previous, bindings.rotation);
                            const auto second =
                                lagcomp::history_angle_quaternion(angle, bindings.rotation);
                            angle = lagcomp::history_quaternion_angles(
                                lagcomp::slerp_history_quaternion(
                                    first, lagcomp::align_history_quaternion(first, second),
                                    fraction, bindings.rotation),
                                bindings.rotation);
                        }
                    }
                }
            }
            const auto scale = variable_float(bindings.recoil_scale);
            state.recoil_pitch_60 = angle[0] * scale;
            state.recoil_yaw_64 = angle[1] * scale;
            state.recoil_roll_68 = angle[2] * scale;
            state.byte_6c = true;
        }
    }
    if (!frame.weapon || !config.inaccuracy.value || (frame.flags & 0x4000) != 0 ||
        !supported_weapon(field<std::uint16_t>(frame.weapon, fields.weapon_definition)))
        return;
    auto spread = frame.spread.inaccuracy + frame.spread.spread;
    if (!prediction_variable_initialized) {
        prediction_variable_initialized = true;
        prediction_variable = find_variable(bindings.cvar);
    }
    if (prediction_variable && variable_integer(prediction_variable) == 0) {
        const auto weapon_time = field<float>(frame.weapon, fields.weapon_time_f71ce331);
        const auto next =
            maximum_number(field<float>(local, fields.player_time_de530a8b), weapon_time);
        const auto quotient = (next - *bindings.current_time) / *bindings.interval;
        const auto rounded = _mm_cvtss_si32(_mm_set_ss(quotient));
        const auto rounded_float = _mm_cvtss_f32(_mm_cvtsi32_ss(_mm_setzero_ps(), rounded));
        auto count = std::uint32_t(rounded);
        if (quotient > rounded_float)
            ++count;
        if (count - 1 <= 0x7E) {
            const auto penalty = field<float>(frame.weapon, fields.weapon_penalty_d2563274);
            const auto recoil = field<float>(frame.weapon, fields.weapon_recoil_index);
            for (std::uint32_t tick = 0; tick < count; ++tick)
                engine::invoke<void>(frame.weapon, 484);
            spread = engine::invoke<float>(frame.weapon, 483) + frame.spread.spread;
            assign(frame.weapon, fields.weapon_recoil_index, recoil);
            assign(frame.weapon, fields.weapon_penalty_d2563274, penalty);
        }
    }
    std::int32_t width, height;
    engine::invoke<void>(bindings.engine, 5, &width, &height);
    const auto radians = (*bindings.field_of_view * 0.01745329238474369f) * 0.5f;
    const auto scaled = spread * 320.0f;
    const auto ratio = scaled / bindings.tangent(radians);
    const auto screen = _mm_cvtss_f32(_mm_cvtepi32_ps(_mm_cvtsi32_si128(height))) / 480.0f;
    auto radius = screen * ratio;
    if (engine::invoke<std::int32_t>(bindings.input, 32, -1))
        radius *= 0.9f;
    state.radius_5c = radius;
}
} // namespace skeet::hud

// ============================================================================
// Subcomponent: simple_owners.cpp
// ============================================================================

namespace skeet::hud {
using esp::Vertex;
using esp::integer_float;
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define SIMPLE_MEMBER __thiscall
#define SIMPLE_HUD __fastcall
#else
#define SIMPLE_MEMBER
#define SIMPLE_HUD
#endif
using OwnerEntry = void(SIMPLE_MEMBER*)(void*);
const SimpleHudOwnersContext* process_context = nullptr;
void SIMPLE_MEMBER destroy(void* storage) {
    destroy_simple_hud_owner(storage);
}
void SIMPLE_MEMBER simple_unchanged(void*) {} //433CCE8E is exactly RET
void SIMPLE_MEMBER reset_ping(void* storage) {
    reset_ping_hud_history(storage);
}
void SIMPLE_MEMBER bind_crosshair(void* storage) {
    bind_crosshair_hud_variables(storage);
}
void SIMPLE_HUD draw_ping(void*, ui::RenderQueue*, void*, std::int32_t*);
void SIMPLE_HUD draw_crosshair_owner(void*, ui::RenderQueue*, void*, std::int32_t*);
const std::array<OwnerEntry, 9> ping_primary{
    destroy,          simple_unchanged, simple_unchanged, simple_unchanged, simple_unchanged,
    simple_unchanged, simple_unchanged, reset_ping,       simple_unchanged}; //43472364
const std::array<HudFrameEntry, 1> ping_render{draw_ping}; //434727D0
const std::array<OwnerEntry, 9> crosshair_primary{destroy,          bind_crosshair,   simple_unchanged,
                                                  simple_unchanged, simple_unchanged, simple_unchanged,
                                                  simple_unchanged, simple_unchanged, simple_unchanged}; //43470660
const std::array<HudFrameEntry, 1> crosshair_render{draw_crosshair_owner}; //4346F878

// No member initializer may touch the HUD cell before its native registrar.
// These are the actual first-slot ABI cells, not portable binding wrappers.
struct NativeHudEndpoint {
    const HudFrameEntry* vtable;
};
struct PingOwner {
    const OwnerEntry* vtable;
    std::uint32_t retained_4;
    NativeHudEndpoint render;
};
struct CrosshairOwner {
    const OwnerEntry* vtable;
    std::uint32_t retained_4;
    NativeHudEndpoint render;
    const void* style_c;
    const void* weapon_mode_10;
};
static_assert(sizeof(void*) == 4, "The native simple HUD owners require Windows x86");
static_assert(sizeof(PingOwner) == ping_hud_native_owner_size && offsetof(PingOwner, render) == 8);
static_assert(sizeof(CrosshairOwner) == crosshair_hud_native_owner_size &&
              offsetof(CrosshairOwner, render) == 8 && offsetof(CrosshairOwner, style_c) == 0xc &&
              offsetof(CrosshairOwner, weapon_mode_10) == 0x10);
static_assert(sizeof(misc::PingSpikeNetworkHistory) == 0x1830);

/* [removed duplicate field] */
std::uint32_t offset(std::uint32_t pointer, std::uint32_t key, std::uint32_t mask) {
    const auto& globals = process_context->offsets.globals;
    return *globals.pointers.at(pointer) ^ globals.keys.at(key) ^ mask;
}
void* allocate(std::size_t size) { //complete43387448, exact native size
    const auto memory = process_context->lifecycle.memory;
    auto* raw = memory.allocate(memory.process_heap, 8, size + 0x20);
    if (!raw)
        return nullptr;
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15) & ~std::uintptr_t(15);
    const auto displacement = std::uint32_t(header - address), total = std::uint32_t(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &total, 4);
    return reinterpret_cast<void*>(header + 16);
}
void SIMPLE_HUD draw_ping(void*, ui::RenderQueue* queue, void* local, std::int32_t* cursor) {
    const auto& context = *process_context;
    if (!(std::uint32_t(context.feature_indicators.value) & 4u))
        return;
    if (field<std::uint8_t>(local, offset(0x43468e78, 0x434684e4, 0x9d920e35)))
        return;
    if (!context.ping_channel.feature.active())
        return;
    draw_ping_latency_indicator(*queue, context.scripts, context.ping_channel.feature, *cursor,
                                context.indicator_layout);
}
void SIMPLE_HUD draw_crosshair_owner(void* callback, ui::RenderQueue* queue, void* local,
                                     std::int32_t*) {
    const auto& context = *process_context;
    const auto* owner = reinterpret_cast<const CrosshairOwner*>(
        static_cast<const std::byte*>(callback) - offsetof(CrosshairOwner, render));
    const NativeCrosshairBindings bindings{context.entity_list, owner->style_c,
                                           owner->weapon_mode_10, context.offsets};
    draw_live_crosshair(*queue, context.overlays, bindings, local, context.suspended);
}
#undef SIMPLE_MEMBER
#undef SIMPLE_HUD
} //namespace

void bind_simple_hud_owners_context(const SimpleHudOwnersContext& context) {
    if (process_context && process_context != &context)
        throw std::logic_error("Simple HUD callbacks require one stable process context");
    process_context = &context;
}
void initialize_ping_hud_owner() { //whole4347DD87, including its failed-allocation RET
    if (auto* storage = allocate(ping_hud_native_owner_size))
        construct_ping_hud_owner(storage);
}
void initialize_crosshair_hud_owner() { //whole43486BF7, including its failed-allocation RET
    if (auto* storage = allocate(crosshair_hud_native_owner_size))
        construct_crosshair_hud_owner(storage);
}
void construct_ping_hud_owner(void* storage) { //4347DD96..4347DDBA
    auto* owner = ::new (storage) PingOwner;
    owner->vtable = nullptr;
    process_context->lifecycle.append(owner);
    owner->render.vtable = nullptr;
    register_hud_frame(process_context->hud, reinterpret_cast<HudFrameCallback*>(&owner->render));
    owner->render.vtable = ping_render.data();
    owner->vtable = ping_primary.data();
}
void construct_crosshair_hud_owner(void* storage) { //43486C06..43486C2A
    auto* owner = ::new (storage) CrosshairOwner;
    owner->vtable = nullptr;
    process_context->lifecycle.append(owner);
    register_hud_frame(process_context->hud, reinterpret_cast<HudFrameCallback*>(&owner->render));
    owner->style_c = nullptr;
    owner->vtable = crosshair_primary.data();
    owner->render.vtable = crosshair_render.data();
}
void destroy_simple_hud_owner(void* storage) {
    if (!storage)
        return;
    const auto memory = process_context->lifecycle.memory;
    const auto displacement = field<std::uint32_t>(storage, -8);
    memory.free(memory.process_heap, 0, static_cast<std::byte*>(storage) - 16 - displacement);
}
void reset_ping_hud_history(void*) {
    std::memset(&process_context->ping_channel.bindings.history, 0, 0x1830);
}
void bind_crosshair_hud_variables(void* storage) {
    auto& owner = *static_cast<CrosshairOwner*>(storage);
    owner.style_c = misc::find_reveal_ranks_command(process_context->cvar, 0xe95e1c63u);
    owner.weapon_mode_10 = misc::find_reveal_ranks_command(process_context->cvar, 0xb468ad35u);
}
} //namespace skeet::hud

// ============================================================================
// Subcomponent: simple_runtime_owner.cpp
// ============================================================================

namespace skeet::hud {
using esp::Vertex;
using esp::integer_float;
namespace {
client::ClientRuntimeStorage& checked(client::ClientRuntimeStorage& storage,
                                      client::ConfigurationRuntimeOwner& configuration,
                                      client::VisualRuntimeOwner& visual,
                                      engine::HookRuntimeOwner& hooks,
                                      script::SourceScriptRuntimeOwner& scripts) {
    if (&configuration.storage() != &storage || &visual.storage() != &storage ||
        &hooks.storage() != &storage || &scripts.storage() != &storage ||
        !storage.data().native_cell<void*>(0x4346A890u) ||
        !storage.data().native_cell<void*>(0x4346A87Cu))
        throw std::invalid_argument(
            "Simple HUD must share the real renderer, channel, Lua and engine owners");
    return storage;
}
engine::OwnerRegistryBinding registry(client::ClientRuntimeStorage& storage,
                                      std::uint32_t address) {
    return {storage.owner_registries().at(address), storage.memory()};
}
void ping(void*) {
    initialize_ping_hud_owner();
}
void crosshair(void*) {
    initialize_crosshair_hud_owner();
}
}
SimpleHudRuntimeOwner::SimpleHudRuntimeOwner(client::ClientRuntimeStorage& storage,
                                             client::ConfigurationRuntimeOwner& configuration,
                                             client::VisualRuntimeOwner& visual,
                                             engine::HookRuntimeOwner& hooks,
                                             script::SourceScriptRuntimeOwner& scripts)
    : storage_(checked(storage, configuration, visual, hooks, scripts)),
      context_{registry(storage, 0x43475B78u),
               registry(storage, 0x43475A9Cu),
               scripts.bridge(),
               configuration.other_esp_settings().feature_indicators,
               hooks.channel(),
               visual.indicators(),
               configuration.other_esp_settings().overlays,
               storage.data().native_cell<void*>(0x4346A890u),
               storage.data().native_cell<void*>(0x4346A87Cu),
               storage.offsets(),
               storage.data().native_cell<std::uint8_t>(0x43468E58u)} {}
void SimpleHudRuntimeOwner::bind_initializers(client::NativeStartupSources& sources) {
    if (&sources.data() != &storage_.data())
        throw std::invalid_argument("HUD startup uses another process storage");
    bind_simple_hud_owners_context(context_);
    sources.bind_initializer({0x4347DD87u, &context_, ping});
    sources.bind_initializer({0x43486BF7u, &context_, crosshair});
}
}
