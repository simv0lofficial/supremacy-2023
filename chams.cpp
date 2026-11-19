// Chams & Visual Effects Subsystem Implementation
#include "util/address.h"
#include "skeet/effects/blur.hpp"
#include "skeet/engine/interface_call.hpp"
#include <bit>
#include <cstring>
#include <immintrin.h>
#include "skeet/effects/blur_runtime.hpp"
#include <array>
#include <limits>
#include <new>
#include "skeet/effects/bullet_impact_command_owner.hpp"
#include "skeet/esp/entity_overlay.hpp"
#include <cmath>
#include "skeet/effects/camera.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/effects/camera_runtime_owner.hpp"
#include "skeet/client/runtime_storage.hpp"
#include "skeet/client/runtime_composition.hpp"
#include "skeet/client/player_runtime_owner.hpp"
#include "skeet/esp/effects_config.hpp"
#include <stdexcept>
#include "skeet/effects/entity_render_state.hpp"
#include "skeet/animation/entity_update.hpp"
#include "skeet/effects/entity_visibility.hpp"
#include "skeet/esp/player_runtime.hpp"
#include "skeet/effects/free_look_capture.hpp"
#include "skeet/effects/free_look_owner.hpp"
#include "skeet/movement/collision.hpp"
#include <xmmintrin.h>
#include "skeet/client/native_startup.hpp"
#include "skeet/effects/free_look_runtime_owner.hpp"
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/misc/config.hpp"
#include "skeet/effects/free_look_view.hpp"
#include "skeet/movement/steering_math.hpp"
#include "skeet/effects/glow.hpp"
#include "skeet/effects/glow_engine.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/effects/harmless_molotov.hpp"
#include "skeet/events/game_events.hpp"
#include <algorithm>
#include <emmintrin.h>
#include "skeet/effects/harmless_molotov_owner.hpp"
#include "skeet/effects/model_entity_relationships.hpp"
#include "skeet/effects/model_render.hpp"
#include "skeet/effects/model_hooks.hpp"
#include "skeet/anti_aim/command_owner.hpp"
#include "skeet/engine/native_data_image.hpp"
#include <cstdio>
#include <x86intrin.h>
#include "skeet/effects/model_render_sources.hpp"
#include "skeet/engine/startup_convars.hpp"
#include "skeet/effects/model_render_lifecycle.hpp"
#include <atomic>
#include "skeet/effects/owner_lifecycle.hpp"
#include "skeet/effects/owner_settings.hpp"
#include <string>
#include "skeet/effects/player_auxiliary_hooks.hpp"
#include "skeet/client/history_runtime_owner.hpp"
#include "skeet/animation/runtime_owner.hpp"
#include "skeet/engine/hook_runtime_owner.hpp"
#include "skeet/engine/netvar_initialization.hpp"
#include "skeet/effects/player_opacity.hpp"
#include "skeet/effects/player_opacity_owner.hpp"
#include "skeet/effects/projectile_ribbons.hpp"
#include "skeet/esp/grenade_environment.hpp"
#include "skeet/render/beam_geometry.hpp"
#include "skeet/effects/receive_proxies.hpp"
#include "skeet/client/entity_update_dispatch.hpp"
#include "skeet/effects/runtime_owner.hpp"
#include "skeet/effects/smoke_owner.hpp"
#include "skeet/effects/scope_owner.hpp"
#include "skeet/client/combat_runtime_factories.hpp"
#include "skeet/client/visual_runtime_owner.hpp"
#include "skeet/engine/build_material_bindings.hpp"
#include "skeet/engine/build_esp_bindings.hpp"
#include "skeet/engine/scene_runtime_owner.hpp"
#include "skeet/engine/hook_descriptor_storage.hpp"
#include "skeet/lagcomp/history_runtime.hpp"
#include "skeet/rage/spread_storage.hpp"
#include "skeet/rage/shot_diagnostics.hpp"
#include "skeet/effects/view.hpp"
#include "skeet/movement/command_runtime.hpp"
#include "skeet/effects/view_dispatch.hpp"
#include "skeet/engine/hooks.hpp"
#include "skeet/effects/view_fov.hpp"
#include "skeet/effects/visual_receive_owner.hpp"
#include "skeet/effects/world.hpp"
#include "skeet/effects/world_callbacks.hpp"


// ============================================================================
// Subcomponent: blur.cpp
// ============================================================================

namespace skeet::effects {

using skeet::read;
using skeet::write;
using skeet::field;
using skeet::ticks;
using skeet::truncate;

inline float sub(float a, float b) noexcept { return a - b; }
inline float mul(float a, float b) noexcept { return a * b; }
inline float neg(float value) noexcept { return -value; }
inline float negate(float value) noexcept { return -value; }


using engine::invoke;
namespace {
using i32 = std::int32_t;
/* [removed duplicate sub] */
i32 truncated(float value) noexcept {
    return _mm_cvttss_si32(_mm_set_ss(value));
}
i32 bounded(i32 value, i32 upper) noexcept {
    if (upper < 0)
        return upper;
    if (value < 0)
        return 0;
    return value > upper ? upper : value;
}
void* find_material(void* system, std::uint32_t wanted) {
    const auto lock = invoke<std::uint32_t>(system, 118);
    auto handle = invoke<std::uint16_t>(system, 86);
    void* found = nullptr;
    while (handle != invoke<std::uint16_t>(system, 88)) {
        auto* material = invoke<void*>(system, 89, handle);
        if (material && !invoke<bool>(material, 42)) {
            const auto* name = invoke<const char*>(material, 0);
            std::uint32_t hash = 0x811C9DC5u;
            if (name)
                for (; *name; ++name)
                    hash = (hash ^ std::uint8_t(*name)) * 0x01000193u;
            if ((hash ^ 0x92CA5DF1u) == wanted) {
                found = material;
                break;
            }
        }
        handle = invoke<std::uint16_t>(system, 87, handle);
    }
    invoke<void>(system, 119, lock);
    return found;
}
void copy_scene(const BlurEffect& effect, i32 width, i32 height) { // 433FA14F
    auto* context = invoke<void*>(effect.bindings.material_system, 115);
    if (context)
        invoke<void>(context, 2);
    i32 actual_width = width, actual_height = height;
    invoke<void>(context, 8, &actual_width, &actual_height);
    auto* texture = effect.textures[2];
    const auto texture_width = invoke<i32>(texture, 3);
    const auto texture_height = invoke<i32>(texture, 4);
    const std::array<i32, 4> source{0, 0, width, height};
    auto destination = source;
    if (actual_width > texture_width || actual_height > texture_height) {
        const float horizontal = float(texture_width) / float(actual_width);
        const float vertical = float(texture_height) / float(actual_height);
        destination[0] = bounded(truncated(float(source[0]) * horizontal), texture_width);
        destination[1] = bounded(truncated(float(source[1]) * vertical), texture_height);
        destination[2] =
            bounded(truncated(float(source[2]) * horizontal), sub(texture_width, destination[0]));
        destination[3] =
            bounded(truncated(float(source[3]) * vertical), sub(texture_height, destination[1]));
    }
    invoke<void>(context, 122, texture, i32(0), source.data(), destination.data());
    invoke<void>(context, 20, texture, i32(0));
    invoke<void>(context, 3);
    invoke<void>(context, 1);
}
void set_target(void* context, void* texture) { // 433CB550
    invoke<void>(context, 6, texture);
    const auto height = invoke<i32>(texture, 4);
    const auto width = invoke<i32>(texture, 3);
    invoke<void>(context, 40, i32(0), i32(0), width, height);
}
void screen_rectangle(void* context, void* material, i32 width, i32 height, float right,
                      float bottom, i32 texture_width, i32 texture_height) { // 4340189C
    invoke<void>(context, 114, material, i32(0), i32(0), width, height, 0.0f, 0.0f, right, bottom,
                 texture_width, texture_height, static_cast<void*>(nullptr), i32(1), i32(1));
}
void attribute(BlurMeshDescriptor& description, unsigned channel, i32 vertex,
               std::span<const float> values) {
    auto* bytes = static_cast<std::byte*>(description.vertex_data[channel]);
    const auto offset = std::int64_t(description.vertex_layout[channel]) * vertex;
    std::memcpy(bytes + offset, values.data(), values.size_bytes());
}
void rectangle_mesh(void* context, void* material, std::span<const BlurRectangle> rectangles,
                    i32 screen_width,
                    i32 screen_height) { // 4334EF43 and its MeshBuilder dependency
    for (i32 matrix = 0; matrix < 2; ++matrix) {
        invoke<void>(context, 22, matrix);
        invoke<void>(context, 23);
        invoke<void>(context, 33);
    }
    invoke<void>(context, 9, material, static_cast<void*>(nullptr));
    auto* mesh = invoke<void*>(context, 62, true, static_cast<void*>(nullptr),
                               static_cast<void*>(nullptr), static_cast<void*>(nullptr));
    const auto vertices = i32(rectangles.size() * 4), indices = i32(rectangles.size() * 6);
    invoke<void>(mesh, 10, i32(2));
    BlurMeshDescriptor description;
    invoke<void>(mesh, 17, vertices, indices, &description, false);
    const float reciprocal_width = 1.0f / float(screen_width),
                reciprocal_height = 1.0f / float(screen_height);
    i32 vertex = 0;
    for (const auto rectangle : rectangles) {
        const float x = float(rectangle.x), y = float(rectangle.y);
        const float far_x = float(rectangle.width) + x, far_y = float(rectangle.height) + y;
        const float x_step = rectangle.width > 1 ? (far_x - x) / float(rectangle.width - 1) : 0.0f;
        const float y_step =
            rectangle.height > 1 ? (far_y - y) / float(rectangle.height - 1) : 0.0f;
        const float left_uv = (x + 0.5f) - x_step * 0.5f;
        const float right_uv = (far_x + 0.5f) + x_step * 0.5f;
        const float top_uv = (y + 0.5f) - y_step * 0.5f;
        const float bottom_uv = (far_y + 0.5f) + y_step * 0.5f;
        i32 viewport_x, viewport_y, viewport_width, viewport_height;
        invoke<void>(context, 41, &viewport_x, &viewport_y, &viewport_width, &viewport_height);
        const float left = ((x - 0.5f) + (x - 0.5f)) / float(viewport_width) - 1.0f;
        const float right = ((float(i32(rectangle.x) + rectangle.width) - 0.5f) +
                             (float(i32(rectangle.x) + rectangle.width) - 0.5f)) /
                                float(viewport_width) -
                            1.0f;
        const float top = 1.0f - ((y - 0.5f) + (y - 0.5f)) / float(viewport_height);
        const float bottom = 1.0f - ((float(i32(rectangle.y) + rectangle.height) - 0.5f) +
                                     (float(i32(rectangle.y) + rectangle.height) - 0.5f)) /
                                        float(viewport_height);
        for (unsigned corner = 0; corner < 4; ++corner, ++vertex) {
            const bool first = corner == 0 || corner == 3;
            const bool lower = (corner & 2) != 0;
            const std::array<float, 3> position{first ? left : right, lower ? bottom : top, 0.0f};
            const std::array<float, 3> normal{0.0f, 0.0f, 1.0f};
            const std::array<float, 2> uv{reciprocal_width * (first ? left_uv : right_uv),
                                          reciprocal_height * (lower ? bottom_uv : top_uv)};
            const std::array<float, 3> tangent_s{0.0f, 1.0f, 0.0f}, tangent_t{1.0f, 0.0f, 0.0f};
            attribute(description, 0, vertex, position);
            attribute(description, 3, vertex, normal);
            attribute(description, 6, vertex, uv);
            attribute(description, 14, vertex, tangent_s);
            attribute(description, 15, vertex, tangent_t);
        }
    }
    i32 indices_written = 0;
    if (description.index_size) {
        if (description.indices) {
            if (!(reinterpret_cast<std::uintptr_t>(description.indices) & 3)) {
                const auto first = std::uint32_t(std::uint16_t(description.first_vertex));
                auto pair = (first | (first << 16)) + 2u;
                for (std::size_t i = 0; i < rectangles.size(); ++i, pair += 0x00040004u) {
                    const std::array<std::uint32_t, 3> quad{pair + 0xFFFEu, pair,
                                                            pair + 0x00030000u};
                    std::memcpy(description.indices + i * 6, quad.data(), sizeof(quad));
                }
            } else {
                auto first = std::uint16_t(description.first_vertex);
                for (std::size_t i = 0; i < rectangles.size();
                     ++i, first = std::uint16_t(first + 4)) {
                    const std::array<std::uint16_t, 6> quad{
                        first, std::uint16_t(first + 1), std::uint16_t(first + 2),
                        first, std::uint16_t(first + 2), std::uint16_t(first + 3)};
                    std::memcpy(description.indices + i * 6, quad.data(), sizeof(quad));
                }
            }
        }
        indices_written =
            std::bit_cast<i32>(std::uint32_t(indices) * std::uint32_t(description.index_size));
    }
    invoke<void>(mesh, 20, vertices, indices_written, &description);
    invoke<void>(mesh, 12, i32(-1), i32(0));
    for (i32 matrix = 0; matrix < 2; ++matrix) {
        invoke<void>(context, 22, matrix);
        invoke<void>(context, 24);
    }
}
}
void BlurEffect::draw(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height) {
    if (rectangles_.size() >= 8192)
        return;
    if (rectangles_.size() == rectangles_.capacity())
        rectangles_.reserve(rectangles_.size() + 1);
    rectangles_.push_back({std::bit_cast<std::int16_t>(std::uint16_t(x)),
                           std::bit_cast<std::int16_t>(std::uint16_t(y)),
                           std::bit_cast<std::int16_t>(std::uint16_t(width)),
                           std::bit_cast<std::int16_t>(std::uint16_t(height))});
}
bool BlurEffect::initialize_resources() {
    for (unsigned i = 0; i < 3; ++i)
        if (!materials[i])
            materials[i] = find_material(bindings.material_system, material_hashes[i]);
    if (!materials[3]) {
        materials[3] = invoke<void*>(
            bindings.material_system, 84, static_cast<const char*>(nullptr),
            static_cast<const char*>(nullptr), false, static_cast<const char*>(nullptr));
        if (materials[3])
            invoke<void>(materials[3], 12);
    }
    for (unsigned i = 0; i < 3; ++i)
        if (!textures[i]) {
            textures[i] = invoke<void*>(bindings.material_system, 91, texture_names[i].c_str(),
                                        texture_group.c_str(), true, std::int32_t(0));
            if (textures[i])
                invoke<void>(textures[i], 10);
        }
    return materials[0] && materials[1] && materials[2] && materials[3] && textures[0] &&
           textures[1] && textures[2];
}
void BlurEffect::release_resources() {
    for (auto& material : materials)
        if (material) {
            invoke<void>(material, 13);
            material = nullptr;
        }
    for (auto& texture : textures)
        if (texture) {
            invoke<void>(texture, 11);
            texture = nullptr;
        }
}
void BlurEffect::apply() {
    i32 width, height;
    invoke<void>(bindings.engine, 5, &width, &height);
    auto* context = invoke<void*>(bindings.material_system, 115);
    if (context)
        invoke<void>(context, 2);
    auto* previous = invoke<void*>(context, 7);
    invoke<void>(context, 6, static_cast<void*>(nullptr));
    copy_scene(*this, width, height);
    invoke<void>(context, 6, previous);
    invoke<void>(context, 119);
    const auto quarter_width = width / 4, quarter_height = height / 4;
    set_target(context, textures[0]);
    auto texture_height = invoke<i32>(textures[2], 4);
    auto texture_width = invoke<i32>(textures[2], 3);
    screen_rectangle(context, materials[0], quarter_width, quarter_height, float(sub(width, 4)),
                     float(sub(height, 4)), texture_width, texture_height);
    set_target(context, textures[1]);
    texture_height = invoke<i32>(textures[0], 4);
    texture_width = invoke<i32>(textures[0], 3);
    screen_rectangle(context, materials[1], quarter_width, quarter_height,
                     float(sub(quarter_width, 1)), float(sub(quarter_height, 1)), texture_width,
                     texture_height);
    set_target(context, textures[0]);
    texture_height = invoke<i32>(textures[0], 4);
    texture_width = invoke<i32>(textures[0], 3);
    screen_rectangle(context, materials[2], quarter_width, quarter_height,
                     float(sub(quarter_width, 1)), float(sub(quarter_height, 1)), texture_width,
                     texture_height);
    invoke<void>(context, 120);
    if (auto** variables = invoke<void**>(materials[3], 41)) {
        if (variables[6])
            invoke<void>(variables[6], 15, textures[0]);
        if (variables[5])
            invoke<void>(variables[5], 4, 1.0f);
    }
    rectangle_mesh(context, materials[3], rectangles_, width, height);
    invoke<void>(context, 3);
    invoke<void>(context, 1);
}
void BlurEffect::render() {
    if (rectangles_.empty())
        return;
    if (initialize_resources())
        apply();
    rectangles_.clear();
}
} // namespace skeet::effects

// ============================================================================
// Subcomponent: blur_runtime.cpp
// ============================================================================

namespace skeet::effects {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define BLUR_MEMBER __thiscall
#define BLUR_FRAME __fastcall
#else
#define BLUR_MEMBER
#define BLUR_FRAME
#endif
using OwnerEntry = void(BLUR_MEMBER*)(void*);
struct BlurInstance;
struct BlurFrameEndpoint : EffectFrameCallback {
    BlurInstance* instance;
};

void BLUR_MEMBER destroy_instance(void*);
void BLUR_MEMBER release_instance(void*);
void BLUR_MEMBER unchanged_event(void*) {} //433CCE8E is exactly RET.
void BLUR_FRAME render_instance(void*, std::uintptr_t, std::uintptr_t, std::uintptr_t,
                                std::uintptr_t);
const std::array<OwnerEntry, 9> blur_owner_table{destroy_instance, unchanged_event,  unchanged_event,
                                            unchanged_event,  release_instance, unchanged_event,
                                            unchanged_event,  unchanged_event,  unchanged_event};
const std::array<EffectFrameEntry, 1> frame_table{render_instance};

void* allocate_instance(engine::JobMemory memory, std::size_t size) { //43387448
    auto* raw = memory.allocate(memory.process_heap, 8, size + 0x20);
    if (!raw)
        return nullptr;
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15) & ~std::uintptr_t(15);
    const auto displacement = std::uint32_t(header - address);
    const auto allocation_size = std::uint32_t(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    return reinterpret_cast<void*>(header + 16);
}
void free_instance(engine::JobMemory memory, void* object) { //433837D0
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
struct BlurInstance {
    const OwnerEntry* vtable = nullptr;
    BlurFrameEndpoint frame{{nullptr}, this};
    BlurEffect** active;
    engine::JobMemory memory;
    alignas(BlurEffect) std::array<std::byte, sizeof(BlurEffect)> state;

    BlurInstance(engine::OwnerRegistryBinding owners, engine::OwnerRegistryBinding callbacks,
                 BlurEffect*& current, BlurBindings bindings)
        : active(&current), memory(owners.memory) {
        owners.append(this); //433676A0, before installing either native table.
        void* endpoint = reinterpret_cast<std::uintptr_t>(this) ==
                                 std::numeric_limits<std::uintptr_t>::max() - 11
                             ? nullptr
                             : &frame;
        engine::append_owner(callbacks.registry, &endpoint, callbacks.memory); //4347E391..E3AB
        vtable = blur_owner_table.data();
        frame.vtable = frame_table.data();
        ::new (state.data()) BlurEffect(bindings);
    }
    BlurEffect& effect() noexcept {
        return *std::launder(reinterpret_cast<BlurEffect*>(state.data()));
    }
    ~BlurInstance() {
        effect().~BlurEffect();
    }
};
void BLUR_MEMBER destroy_instance(void* object) { //433823D9/4332C63C
    auto* instance = static_cast<BlurInstance*>(object);
    const auto memory = instance->memory;
    *instance->active = nullptr;
    instance->~BlurInstance();
    free_instance(memory, instance);
}
void BLUR_MEMBER release_instance(void* object) { //433E2BC0
    static_cast<BlurInstance*>(object)->effect().release_resources();
}
void BLUR_FRAME render_instance(void* object, std::uintptr_t, std::uintptr_t, std::uintptr_t,
                                std::uintptr_t) { //433C8896
    static_cast<BlurFrameEndpoint*>(object)->instance->effect().render();
}
#undef BLUR_MEMBER
#undef BLUR_FRAME
}

void dispatch_effect_frame(const engine::OwnerRegistry& callbacks, std::uintptr_t first,
                           std::uintptr_t second, std::uintptr_t third,
                           std::uintptr_t fourth) { //433B8C46
    const auto entries = callbacks.entries();
    for (auto* object : entries) {
        auto* callback = static_cast<EffectFrameCallback*>(object);
        callback->vtable[0](object, first, second, third, fourth);
    }
}
void initialize_blur(engine::OwnerRegistryBinding owners, engine::OwnerRegistryBinding callbacks,
                     BlurEffect*& active,
                     BlurBindings bindings) { //4347F6DB
    auto* storage = allocate_instance(owners.memory, sizeof(BlurInstance));
    if (!storage) {
        active = nullptr;
        return;
    }
    auto* instance = ::new (storage) BlurInstance(owners, callbacks, active, bindings);
    active = &instance->effect();
}
} // namespace skeet::effects

// ============================================================================
// Subcomponent: bullet_impact_command_owner.cpp
// ============================================================================

namespace skeet::effects {
namespace {
/* [removed duplicate read] */
bool game_rule_override(const BulletImpactCommandBindings& bindings) {
    auto** rules = bindings.game_rules;
    return rules && *rules && read<std::uint8_t>(*rules, bindings.game_rule_override) != 0;
}
bool shotgun_patterns_enabled(const void* variable) {
    for (;;) {
        const auto* parent = read<const void*>(variable, 0x1C);
        if (parent == variable)
            break;
        variable = parent;
    }
    return (read<std::uint32_t>(variable, 0x30) ^
            std::uint32_t(reinterpret_cast<std::uintptr_t>(variable))) != 0;
}
/* [removed duplicate negate] */
struct ImpactBasis {
    RageVector forward, right, up;
};
ImpactBasis impact_basis(const std::array<float, 3>& angles, const RageTrigonometry& trig) {
    //43366128, with its actual scalar CRT boundaries and scalar operation order.
    const auto pitch = angles[0] * 0.01745329238474369f;
    const auto sp = trig.sine(pitch), cp = trig.cosine(pitch);
    const auto yaw = angles[1] * 0.01745329238474369f;
    const auto sy = trig.sine(yaw), cy = trig.cosine(yaw);
    const auto roll = angles[2] * 0.01745329238474369f;
    const auto sr = trig.sine(roll), cr = trig.cosine(roll);
    const auto minus_cr = cr * -1.0f, sr_sp = sr * sp, cr_sp = cr * sp;
    return {{cy * cp, sy * cp, negate(sp)},
            {minus_cr * negate(sy) - sr_sp * cy, minus_cr * cy - sr_sp * sy, (sr * -1.0f) * cp},
            {cr_sp * cy - negate(sy) * sr, cr_sp * sy - sr * cy, cr * cp}};
}
struct ImpactTraceFilter {
    void* const* vtable;
    std::array<std::uint32_t, 5> fields_04{};
    std::uint32_t mask = 0x4600400Bu, exit_mask = 0x4600400Bu;
    float minimum_damage = 1.0f;
    BallisticFilter binding() noexcept {
        return {this, 0, mask, exit_mask, minimum_damage, BallisticFilterKind::supplied};
    }
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(ImpactTraceFilter) == 0x24);
static_assert(offsetof(ImpactTraceFilter, mask) == 0x18);
#endif
}

void process_bullet_impacts(EffectsCommandOwner& owner, const client::CommandFrame& frame,
                            void* player, const client::Command& command) {
    auto& settings = owner.world.settings;
    const auto& bindings = owner.bindings;
    if (!settings.bullet_impacts.value || owner.world.suspended)
        return;
    auto* weapon = reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
    if (!weapon || !frame.weapon_data_address || game_rule_override(bindings))
        return;
    // The classification consumes the live item definition, not frame+D8.
    if (!esp::ammunition_weapon(read<std::uint16_t>(weapon, bindings.weapon_definition)))
        return;
    if (!(frame.flags & 0x100000u) && (!(frame.flags & 0x400u) || !(command.movement.buttons & 1u)))
        return;

    auto angles = command.angles;
    const auto& recoil = frame.recoil_angles_9c;
    const auto recoil_squared =
        (recoil[0] * recoil[0] + recoil[1] * recoil[1]) + recoil[2] * recoil[2];
    if (recoil_squared != 0.0f)
        for (std::size_t i = 0; i != 3; ++i)
            angles[i] = recoil[i] + angles[i];
    const auto basis = impact_basis(angles, bindings.trigonometry);
    auto direction = basis.forward;
    if (frame.flags & 0x4000u) {
        const rage::SpreadParameters parameters{std::uint32_t(frame.weapon_id_d8),
                                                std::bit_cast<std::int32_t>(frame.field_b0),
                                                frame.spread_d0,
                                                frame.inaccuracy_c4,
                                                frame.weapon_field_100,
                                                frame.pellet_count_19c,
                                                shotgun_patterns_enabled(bindings.shotgun_patterns),
                                                frame.weapon_kind_124};
        const auto sample = rage::generate_spread_sample(
            parameters, std::uint8_t(command.random_seed), bindings.spread_tables, bindings.random,
            bindings.shotgun_pattern, bindings.trigonometry);
        for (std::size_t i = 0; i != 3; ++i)
            direction[i] =
                (basis.right[i] * sample.first + basis.forward[i]) + basis.up[i] * sample.second;
        const auto squared = (direction[0] * direction[0] + direction[1] * direction[1]) +
                             direction[2] * direction[2];
        const auto root = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(squared)));
        const auto inverse = 1.0f / (root + 0x1p-23f);
        for (auto& value : direction)
            value *= inverse;
    }
    // Reload frame+C after the spread provider, as4339EE21 does. Genuine
    // weapon getters run here, before constructing/consuming the trace filter.
    auto query =
        rage::prepare_target_bullet(reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address)),
                                    bindings.weapon_definition, true);
    ImpactTraceFilter native_filter{bindings.trace_filter_vtable};
    const auto filter = native_filter.binding();
    rage::trace_target_direction(bindings.ballistics,
                                 BallisticEntity(reinterpret_cast<std::uintptr_t>(player)),
                                 frame.origin_dc, direction, filter, query);
    for (std::int32_t i = 0; i < std::bit_cast<std::int32_t>(query.point_count); ++i) {
        const RageVector minimum{-1.0f, -1.0f, -1.0f}, maximum{1.0f, 1.0f, 1.0f}, rotation{};
        const auto duration =
            _mm_cvtss_f32(_mm_cvtepi32_ps(_mm_cvtsi32_si128(settings.impact_duration.value)));
        engine::invoke<void>(bindings.debug_overlay, 1, &query.points[std::size_t(i)], &minimum,
                             &maximum, &rotation, std::int32_t(255), std::int32_t(0),
                             std::int32_t(0), std::int32_t(127), duration);
    }
}
void update_effects_entities(EffectsCommandOwner& owner, void* player, std::uintptr_t incoming) {
    update_entity_render_state_packet(owner.camera, owner.players, owner.entities, player,
                                      reinterpret_cast<const void*>(incoming), owner.world.settings,
                                      owner.models.settings.weapons.enabled.value);
}
} // namespace skeet::effects

// ============================================================================
// Subcomponent: camera.cpp
// ============================================================================

namespace skeet::effects {
using engine::invoke;
SavedCameraStateView CameraHistory::at(std::uint32_t index) {
    if (source_) {
        auto* player = source_->get(index);
        if (!player)
            throw std::bad_alloc();
        return {player->native.captured_camera_fov, player->native.captured_observer_mode};
    }
    auto& saved = players[index & 63];
    return {saved.fov, saved.observer_mode};
}
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
}

bool CameraRuntime::alive_or_death_camera(void* player) const { // 0x43386EB3
    if (read<std::uint8_t>(player, offsets.life_state) == 0)
        return true;
    const auto mode = read<std::int32_t>(player, offsets.observer_mode);
    return mode == 1 || mode == 2;
}
void CameraRuntime::alive_changed(bool enabled) { // 0x43322683..0x43322743
    if (!invoke<bool>(interfaces.engine, 26))
        return;
    auto* player = interfaces.local_player;
    if (!player)
        return;
    const auto third_person =
        alive_or_death_camera(player) && enabled && hotkey_active(alive_hotkey.binding, keys);
    invoke<void>(player, 312, third_person); // x86 vtable +0x4E0
    if (invoke<void*>(interfaces.input, 32, std::int32_t(-1)))
        write(interfaces.input, 0x100, 2); // re-read global interface after its call
}
void CameraRuntime::dead_changed() { // 0x433662E6..0x43366304
    if (!invoke<bool>(interfaces.engine, 26))
        return;
    if (auto* player = interfaces.local_player)
        update_observer(player);
}
void CameraRuntime::apply_observer_mode(void* player, std::int32_t mode) {
    if (auto proxy = interfaces.observer_proxy) {
        ObserverProxyData data;
        data.words[2] = std::bit_cast<std::uint32_t>(mode);
        data.words[5] = 1;
        proxy(&data, player, static_cast<std::byte*>(player) + offsets.observer_mode);
    }
}
void CameraRuntime::reset_camera(void* player) {
    if (invoke<void*>(interfaces.input, 32, std::int32_t(-1)))
        invoke<void>(interfaces.input, 36); // x86 vtable +0x90
    invoke<void>(player, 312, false);
}
void CameraRuntime::update_observer(void* player) { // 0x43393F51..0x433942FB
    if (!player)
        return;
    if (force_dead != 0 && !alive_or_death_camera(player) &&
        read<std::int32_t>(player, offsets.observer_mode) != 0) {
        const auto handle = read<std::uint32_t>(player, offsets.observer_target);
        if (handle != UINT32_MAX) {
            auto* target = invoke<void*>(interfaces.entity_list, 4, handle);
            if (target && invoke<bool>(target, 158)) {
                write(target, offsets.reset_field_3f51aa0a, 0);
                write(target, offsets.fov, 0);
            }
        }
        write(player, offsets.reset_field_3f51aa0a, 0);
        write(player, offsets.fov, 0);
        if (read<std::int32_t>(player, offsets.observer_mode) == 4) {
            apply_observer_mode(player, 5);
            reset_camera(player);
        }
        forced_observer = true;
        return;
    }
    if (!forced_observer)
        return;
    const auto handle = read<std::uint32_t>(player, offsets.observer_target);
    if (handle != UINT32_MAX) {
        auto* target = invoke<void*>(interfaces.entity_list, 4, handle);
        if (target && invoke<bool>(target, 158)) {
            // The native predicate can change the handle, so load it again.
            const auto current = read<std::uint32_t>(player, offsets.observer_target);
            const auto index = current == UINT32_MAX ? 0x1FFFu : std::uint16_t(current);
            write(target, offsets.fov, history.at(index).fov);
        }
    }
    const auto index = invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
    const auto& saved = history.at(index);
    write(player, offsets.fov, saved.fov);
    apply_observer_mode(player, saved.observer_mode);
    reset_camera(player);
    forced_observer = false;
}

} // namespace skeet::effects

// ============================================================================
// Subcomponent: camera_runtime_owner.cpp
// ============================================================================

namespace skeet::effects {
namespace {
client::ClientRuntimeStorage& require_owners(client::ClientRuntimeStorage& storage,
                                             const client::EngineRuntimeBindings& engine,
                                             client::ConfigurationRuntimeOwner& settings,
                                             client::PlayerRuntimeOwner& players) {
    if (&settings.storage() != &storage || &players.storage() != &storage ||
        &engine.variables.data() != &storage.data())
        throw std::invalid_argument("Camera contexts require the canonical source owners");
    const auto manifest = engine::build_interface_manifest();
    for (const auto cell : {0x4346A514u, 0x4346A890u}) {
        bool found = false;
        for (std::size_t index = 0; index < manifest.size(); ++index) {
            if (manifest[index].destination_cell != cell)
                continue;
            auto* current = storage.data().native_cell<void*>(cell);
            if (!current || current != engine.interfaces.objects[index])
                throw std::invalid_argument(
                    "Camera interface differs from the acquired engine bundle");
            found = true;
            break;
        }
        if (!found)
            throw std::logic_error("Camera interface is absent from the build manifest");
    }
    // CInput is acquired by initialize_build_game_state_bindings from the
    // actual client instruction operand, not a CreateInterface registration.
    if (!storage.data().native_cell<void*>(0x4346A894u))
        throw std::invalid_argument("Camera requires the acquired CInput object");
    if (!storage.offsets().words)
        throw std::invalid_argument("Camera requires initialized entity offsets");
    return storage;
}
std::uint32_t offset(const engine::NativeOffsetGlobals& globals, std::uint32_t pointer,
                     std::uint32_t key, std::uint32_t mask) {
    const auto* word = globals.pointers.at(pointer);
    if (!word)
        throw std::invalid_argument("Camera offset publication is empty");
    return *word ^ globals.keys.at(key) ^ mask;
}
CameraNetvars offsets(const engine::NativeOffsetGlobals& globals) {
    return {offset(globals, 0x43468E78u, 0x434684E4u, 0x9D920E35u),
            offset(globals, 0x4346A6CCu, 0x43468EF8u, 0x2F4E97C4u),
            offset(globals, 0x43468F84u, 0x43467E64u, 0x3B704B41u),
            offset(globals, 0x43468248u, 0x43469218u, 0xFBE8C5EAu),
            offset(globals, 0x43468D10u, 0x43468DCCu, 0x3F51AA0Au)};
}
}
CameraRuntimeOwner::CameraRuntimeOwner(client::ClientRuntimeStorage& storage,
                                       const client::EngineRuntimeBindings& engine,
                                       client::ConfigurationRuntimeOwner& settings,
                                       client::PlayerRuntimeOwner& players)
    : storage_(require_owners(storage, engine, settings, players)),
      interfaces_(storage.data().native_cell<void*>(0x4346A514u),
                  storage.data().native_cell<void*>(0x4346A890u),
                  storage.data().native_cell<void*>(0x4346A894u),
                  storage.data().native_cell<void*>(0x43475808u),
                  storage.data().native_cell<ObserverProxy>(0x4347BDE4u)),
      history_(players.cache()), runtime_(interfaces_, offsets(storage.offsets().globals), history_,
                                          settings.effects_settings().third_person_dead.value,
                                          settings.effects_settings().third_person_key.value,
                                          storage.data().native_cell<bool>(0x4347CA59u)) {
    runtime_.keys = storage.input_runtime().bitmap();
}
}

// ============================================================================
// Subcomponent: entity_render_state.cpp
// ============================================================================

namespace skeet::effects {
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
using Vector = lagcomp::Vector;

bool convar_is_zero(void* variable) noexcept {
    for (;;) {
        auto* parent = read<void*>(variable, 0x1C);
        if (parent == variable)
            break;
        variable = parent;
    }
    return read<std::uint32_t>(variable, 0x30) ==
           static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(variable));
}

bool same_vector(const Vector& first, const Vector& second) noexcept {
    return first[0] == second[0] && first[1] == second[1] && first[2] == second[2];
}

void set_angles(void* entity, const Vector& desired, const EntityRenderBindings& bindings) {
    const auto& offsets = bindings.offsets;
    if (!same_vector(read<Vector>(entity, offsets.angles_invalidated), desired)) {
        if (bindings.invalidate_physics)
            bindings.invalidate_physics(entity, 2);
        write(entity, offsets.angles_invalidated, desired);
    }
    write(entity, offsets.angles_copy, desired);
}

void synchronize_ragdoll(void* ragdoll, client::PlayerStateCache& cache,
                         const EntityRenderBindings& bindings) {
    const auto& offsets = bindings.offsets;
    const Vector angles{0.0f, read<float>(ragdoll, offsets.ragdoll_yaw), 0.0f};
    write(ragdoll, offsets.render_flags,
          read<std::uint32_t>(ragdoll, offsets.render_flags) | 0x4810u);
    const auto owner_handle = read<std::uint32_t>(ragdoll, offsets.ragdoll_owner);
    if (owner_handle != UINT32_MAX) {
        auto* owner = engine::invoke<void*>(bindings.entity_list, 4, owner_handle);
        if (owner && engine::invoke<bool>(owner, 158)) {
            const auto index =
                engine::invoke<std::uint32_t>(static_cast<std::byte*>(owner) + 8, 10);
            // As in native 4332631B, this branch requires a real nonzero player
            // index. The engine's IsPlayer predicate establishes that contract.
            if (!index)
                throw std::logic_error("Ragdoll owner requires a nonzero player index");
            auto& state = *cache.get(index);
            if (state.history.count) {
                const auto& snapshot = state.history.at(0).animation;
                animation::restore_entity_animation(snapshot, owner, bindings.animation_offsets);
                animation::restore_entity_animation(snapshot, ragdoll, bindings.animation_offsets);
            }
            // The intermediate array is present in the original, including
            // when the history is empty; the live owner supplies all 24 poses.
            std::array<float, 24> poses;
            std::memcpy(poses.data(),
                        static_cast<const std::byte*>(owner) +
                            bindings.animation_offsets.pose_parameters,
                        sizeof poses);
            std::memcpy(static_cast<std::byte*>(ragdoll) +
                            bindings.animation_offsets.pose_parameters,
                        poses.data(), sizeof poses);
            set_angles(owner, angles, bindings);
        }
    }
    write(ragdoll, offsets.reset_byte_8481ea04, std::uint8_t(0));
    write(ragdoll, offsets.origin_copy, read<Vector>(ragdoll, offsets.origin_source));
    if (!same_vector(read<Vector>(ragdoll, offsets.origin_source),
                     read<Vector>(ragdoll, offsets.origin_invalidated))) {
        if (bindings.invalidate_physics)
            bindings.invalidate_physics(ragdoll, 1);
        // Native reads the original source pointer after invalidation.
        write(ragdoll, offsets.origin_invalidated, read<Vector>(ragdoll, offsets.origin_source));
    }
    set_angles(ragdoll, angles, bindings);
}

void update_entries(client::PlayerStateCache& cache, const EntityRenderBindings& bindings,
                    std::span<const EntityRenderListEntry> entries,
                    const esp::EffectsSettings& settings, const std::uint8_t& weapon_models_enabled,
                    bool zero) {
    const auto& offsets = bindings.offsets;
    for (const auto& entry : entries) {
        if (entry.index <= 64 || (entry.index & 0xFFFF0000u))
            continue;
        auto* entity = engine::invoke<void*>(bindings.entity_list, 3, entry.index);
        if (!entity)
            continue;
        if ((zero || settings.disable_ragdolls.value) &&
            read<void*>(entity, 0) == bindings.ragdoll_vtable) {
            if (settings.disable_ragdolls.value) {
                write(entity, offsets.hidden_byte_1b5f47d5, std::uint8_t(0));
                write(entity, offsets.render_flags,
                      read<std::uint32_t>(entity, offsets.render_flags) | 0x4830u);
                write(entity, offsets.render_mode, std::uint8_t(10));
                write(entity, offsets.reset_byte_8481ea04, std::uint8_t(0));
            } else if (zero) {
                synchronize_ragdoll(entity, cache, bindings);
            }
        }
        if (weapon_models_enabled && engine::invoke<bool>(entity, 159))
            engine::invoke<void>(entity, 238, false);
    }
}

bool begin_update(CameraRuntime& camera, const EntityRenderBindings& bindings,
                  void* observer_player, const esp::EffectsSettings& settings,
                  const std::uint8_t& weapon_models_enabled, bool& zero) {
    camera.update_observer(observer_player);
    zero = convar_is_zero(bindings.ragdoll_convar);
    return zero || settings.disable_ragdolls.value || weapon_models_enabled;
}
}

void update_entity_render_state(CameraRuntime& camera, client::PlayerStateCache& cache,
                                const EntityRenderBindings& bindings, void* observer_player,
                                std::span<const EntityRenderListEntry> entries,
                                const esp::EffectsSettings& settings,
                                const std::uint8_t& weapon_models_enabled) {
    bool zero;
    if (!begin_update(camera, bindings, observer_player, settings, weapon_models_enabled, zero))
        return;
    update_entries(cache, bindings, entries, settings, weapon_models_enabled, zero);
}

void update_entity_render_state_packet(CameraRuntime& camera, client::PlayerStateCache& cache,
                                       const EntityRenderBindings& bindings, void* observer_player,
                                       const void* packet, const esp::EffectsSettings& settings,
                                       const std::uint8_t& weapon_models_enabled) {
    bool zero;
    if (!begin_update(camera, bindings, observer_player, settings, weapon_models_enabled, zero))
        return;
    const auto count = read<std::int32_t>(packet, 0x4044);
    if (count <= 0)
        return;
    const auto* entries = reinterpret_cast<const EntityRenderListEntry*>(
        static_cast<const std::byte*>(packet) + 0x44);
    update_entries(cache, bindings, {entries, static_cast<std::size_t>(count)}, settings,
                   weapon_models_enabled, zero);
}

} // namespace skeet::effects

// ============================================================================
// Subcomponent: entity_visibility.cpp
// ============================================================================

namespace skeet::effects {
namespace {
EntityVisibilityRuntime* active_runtime = nullptr;
EntityNetworkableRuntime* active_networkable = nullptr;
engine::VtableHookRegistry* networkable_registry = nullptr;
/* [removed duplicate read] */
/* [removed duplicate write] */
void* parent(void* variable) {
    for (;;) {
        auto* next = read<void*>(variable, 0x1C);
        if (next == variable)
            return variable;
        variable = next;
    }
}
std::int32_t integer(const void* variable) {
    const auto* root = parent(const_cast<void*>(variable));
    return std::bit_cast<std::int32_t>(read<std::uint32_t>(root, 0x30) ^
                                       std::uint32_t(reinterpret_cast<std::uintptr_t>(root)));
}
void integer(void* variable, std::int32_t value) {
    auto* root = parent(variable);
    const auto encoded =
        std::uint32_t(value) ^ std::uint32_t(reinterpret_cast<std::uintptr_t>(root));
    write(root, 0x30, encoded);
    write(root, 0x40, encoded);
}
void* ragdoll_variable(const EntityNetworkableBindings& bindings) {
    return reinterpret_cast<void*>(std::uintptr_t(bindings.encoded_ragdoll_convar ^ 0x3155EA46u));
}
void render_mode(void* entity, std::uint8_t mode, const EntityNetworkableBindings& bindings) {
    if (read<std::uint8_t>(entity, bindings.offsets.render.render_mode) == mode)
        return;
    write(entity, bindings.offsets.render.render_mode, mode);
    auto* render = engine::invoke<void*>(entity, 9);
    const auto effect = read<std::uint8_t>(entity, bindings.offsets.render_fx);
    engine::invoke<void>(render, 2, std::int32_t(effect), std::int32_t(mode),
                         std::numeric_limits<float>::max(), 0.0f);
}
// Engine adapter for the canonical43359AFA observer resolver. No selected
// entity or relationship answer is supplied by an algorithm callback.
struct EffectivePlayer {
    const EntityNetworkableBindings& bindings;
    esp::RuntimePlayer local{}, observed{};
    esp::PlayerRuntimeEngine engine{};
    explicit EffectivePlayer(const EntityNetworkableBindings& b) : bindings(b) {
        const auto player = *b.visibility.local_player;
        if (player) {
            const auto& offsets = b.visibility.relationships.offsets;
            local.engine_entity = player;
            local.life_state =
                read<std::uint8_t>(player, std::uint32_t(offsets.rejection_bytes[0]));
            local.move_type = read<std::uint8_t>(player, std::uint32_t(offsets.movement_type));
            local.observer_mode = read<std::int32_t>(player, b.offsets.observer_mode);
            local.observer_handle = read<std::int32_t>(player, b.offsets.observer_target);
        }
        engine.context = this;
        engine.local_player = player ? &local : nullptr;
        engine.by_handle = [](void* context, std::int32_t handle) -> esp::RuntimePlayer* {
            auto& self = *static_cast<EffectivePlayer*>(context);
            const auto entity = skeet::engine::invoke<void*>(self.bindings.entity_list, 4, handle);
            if (!entity)
                return nullptr;
            self.observed.engine_entity = entity;
            return &self.observed;
        };
        engine.is_player = [](void*, const esp::RuntimePlayer& player) {
            return skeet::engine::invoke<bool>(player.engine_entity, 158);
        };
    }
    void* selected() {
        const auto* player = esp::effective_view_player(engine);
        return player ? player->engine_entity : nullptr;
    }
};
#if defined(_M_IX86) || defined(__i386__)
std::uint32_t __fastcall render_entry(void* entity, void*, std::uintptr_t first,
                                      std::uintptr_t second) {
#else
std::uint32_t render_entry(void* entity, void*, std::uintptr_t first, std::uintptr_t second) {
#endif
    return filter_teammate_render(active_runtime->settings, active_runtime->bindings, entity, first,
                                  second);
}
#if defined(_M_IX86) || defined(__i386__)
void __fastcall networkable_entry(void* receiver, void*, std::int32_t stage) {
#else
void networkable_entry(void* receiver, void*, std::int32_t stage) {
#endif
    active_networkable->post_data_update(receiver, stage);
}
#if defined(_M_IX86) || defined(__i386__)
void __fastcall release_networkable_entry(void* receiver, void*) {
#else
void release_networkable_entry(void* receiver, void*) {
#endif
    release_hooked_networkable(*networkable_registry, receiver);
}
}
rage::TargetBindings
EntityVisibilityRelationships::for_player(void* actual_saved_player) const noexcept {
    return {actual_saved_player,         entity_list, game_type.get(),
            teammates_are_enemies.get(), adjustments, offsets};
}
std::uint32_t filter_teammate_render(const esp::EffectsSettings& settings,
                                     const EntityVisibilityBindings& bindings, void* entity,
                                     std::uintptr_t first, std::uintptr_t second) {
    if (settings.disable_teammates.value && !engine::invoke<bool>(bindings.engine, 82) &&
        !engine::invoke<bool>(bindings.engine, 93)) {
        if (void* local = *bindings.local_player) {
            const auto& base = bindings.relationships;
            const auto relation = base.for_player(entity);
            if (!rage::target_is_enemy(relation, local))
                return 0;
        }
    }
    const auto table = *static_cast<void***>(entity);
    const auto metadata = static_cast<engine::VtableHookMetadata*>(table[-2]);
    auto** original_table = metadata->original_table;
    return reinterpret_cast<engine::MemberFunction<std::uint32_t, std::uintptr_t, std::uintptr_t>>(
        original_table[194])(entity, first, second);
}
void EntityVisibilityRuntime::bind_player_table(engine::VtableHookMetadata& metadata) {
    active_runtime = this;
    metadata.replacement_table[194] = source_entry();
}
void* EntityVisibilityRuntime::source_entry() noexcept {
    return reinterpret_cast<void*>(render_entry);
}

void set_entity_fade(void* entity, float start, float duration,
                     const EntityNetworkableBindings& bindings) {
    const bool sample_clock = start == std::numeric_limits<float>::max();
    if (read<std::uint8_t>(entity, bindings.offsets.render_fx) == 15 && sample_clock)
        return;
    if (sample_clock)
        start = read<float>(bindings.globals, 0x10);
    write(entity, bindings.offsets.render_fx, std::uint8_t(15));
    auto* render = engine::invoke<void*>(entity, 9);
    const auto mode = read<std::uint8_t>(entity, bindings.offsets.render.render_mode);
    engine::invoke<void>(render, 2, std::int32_t(15), std::int32_t(mode), start, duration);
}

bool EntityNetworkableRuntime::visible_ragdoll_pass(void* entity) {
    // Complete433E70FB's visible bit, including the real434181CD relationship
    // chain. The local marker precedes every engine/demo/observer operation.
    if (read<std::uint8_t>(entity, bindings.offsets.local_player_flag) & 1)
        return (models.local_player.enabled.value & 1) != 0;
    const auto& base = bindings.visibility.relationships;
    bool teammate;
    if (engine::invoke<bool>(bindings.visibility.engine, 93))
        teammate = read<std::int32_t>(entity, std::uint32_t(base.offsets.team)) == 3;
    else {
        EffectivePlayer view(bindings);
        const auto local = view.selected();
        if (!local)
            teammate = false;
        else if (read<std::int32_t>(local, std::uint32_t(base.offsets.team)) == 1)
            teammate = read<std::int32_t>(entity, std::uint32_t(base.offsets.team)) == 3;
        else {
            const auto relation = base.for_player(local);
            teammate = !rage::target_is_enemy(relation, entity);
        }
    }
    const auto& group = teammate ? models.teammate : models.player;
    std::uint32_t flags = ((group.enabled.value & 1u) << 2) | ((group.behind_wall.value & 1u) << 3);
    if (!teammate && models.shadow.value &&
        integer(reinterpret_cast<void*>(
            std::uintptr_t(bindings.encoded_shadow_history_convar ^ 0x3AE06513u))) != 0)
        flags |= 16;
    return (flags & 4) != 0;
}

void EntityNetworkableRuntime::post_data_update(void* receiver, std::int32_t stage) {
    auto* entity = static_cast<std::byte*>(receiver) - 8;
    const auto& fields = bindings.offsets.render;
    const bool restore_convar = settings.disable_ragdolls.value != 0;
    std::int32_t saved_convar = 0;
    if (restore_convar) {
        saved_convar = integer(ragdoll_variable(bindings));
        integer(ragdoll_variable(bindings), 0);
    }
    auto owner = [&]() -> void* {
        const auto handle = read<std::uint32_t>(entity, fields.ragdoll_owner);
        return handle == UINT32_MAX ? nullptr
                                    : engine::invoke<void*>(bindings.entity_list, 4, handle);
    };
    if (stage == 0 && models.ragdolls.value)
        if (auto* player = owner(); player && visible_ragdoll_pass(player))
            engine::invoke<void>(entity, 238, false);
    bool hidden = settings.disable_ragdolls.value != 0;
    if (!hidden && settings.disable_teammates.value &&
        !engine::invoke<bool>(bindings.visibility.engine, 82) &&
        !engine::invoke<bool>(bindings.visibility.engine, 93)) {
        if (auto* player = owner()) {
            auto* local = *bindings.visibility.local_player;
            const auto& base = bindings.visibility.relationships;
            if (local &&
                read<std::uint8_t>(local, std::uint32_t(base.offsets.rejection_bytes[0])) == 0) {
                const auto relation = base.for_player(player);
                if (!rage::target_is_enemy(relation, local))
                    hidden = true;
            }
        }
    }
    if (stage == 2 && !hidden) {
        if (integer(ragdoll_variable(bindings)) == 0) {
            if (auto* player = owner(); player && engine::invoke<bool>(player, 158)) {
                const auto index =
                    engine::invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
                auto& state = *players.get(index);
                if (state.history.count)
                    animation::restore_entity_animation(state.history.at(0).animation, player,
                                                        bindings.animation_offsets);
                else {
                    std::array<float, 24> poses;
                    std::memcpy(poses.data(),
                                static_cast<const std::byte*>(player) +
                                    bindings.animation_offsets.pose_parameters,
                                sizeof poses);
                    std::memcpy(entity + bindings.animation_offsets.pose_parameters, poses.data(),
                                sizeof poses);
                }
            }
            const auto source = read<lagcomp::Vector>(entity, fields.origin_source);
            const auto destination = read<lagcomp::Vector>(entity, fields.origin_invalidated);
            if (source[0] != destination[0] || source[1] != destination[1] ||
                source[2] != destination[2]) {
                if (bindings.invalidate_physics)
                    bindings.invalidate_physics(entity, 1);
                write(entity, fields.origin_invalidated,
                      read<lagcomp::Vector>(entity, fields.origin_source));
            }
        }
    } else if (hidden) {
        write(entity, fields.hidden_byte_1b5f47d5, std::uint8_t(0));
        write(entity, fields.reset_byte_8481ea04, std::uint8_t(0));
        write(entity, fields.render_flags,
              read<std::uint32_t>(entity, fields.render_flags) | 0x4830u);
        render_mode(entity, 10, bindings);
    }
    const auto table = *static_cast<void***>(receiver);
    const auto* metadata = static_cast<engine::VtableHookMetadata*>(table[-2]);
    reinterpret_cast<engine::MemberFunction<void, std::int32_t>>(metadata->original_table[5])(
        receiver, stage);
    if (stage == 2) {
        if (hidden || integer(ragdoll_variable(bindings)) == 0) {
            if (!hidden) {
                write(entity, fields.render_flags,
                      read<std::uint32_t>(entity, fields.render_flags) | 0x4810u);
                if (bindings.invalidate_render_flags)
                    bindings.invalidate_render_flags(entity);
            }
            const auto* children = read<const std::uint32_t*>(entity, fields.ragdoll_yaw + 0xC);
            const auto count = read<std::uint32_t>(entity, fields.ragdoll_yaw + 0x18);
            const auto* end = children + count;
            for (; children != end; ++children) {
                if (*children == UINT32_MAX)
                    continue;
                auto* child = engine::invoke<void*>(bindings.entity_list, 4, *children);
                if (!child)
                    continue;
                write(child, fields.render_flags,
                      read<std::uint32_t>(child, fields.render_flags) |
                          (hidden ? 0x4830u : 0x4810u));
                render_mode(child, hidden ? 10 : 2, bindings);
                if (!hidden)
                    set_entity_fade(child, read<float>(bindings.globals, 0x10), 0.6f, bindings);
                if (bindings.invalidate_render_flags)
                    bindings.invalidate_render_flags(child);
            }
        }
    }
    if (restore_convar)
        integer(ragdoll_variable(bindings), saved_convar);
}
void EntityNetworkableRuntime::bind_networkable_table(engine::VtableHookMetadata& metadata) {
    active_networkable = this;
    networkable_registry = &bindings.hook_registry;
    bind_static_networkable_table(metadata);
}
void EntityNetworkableRuntime::bind_static_networkable_table(engine::VtableHookMetadata& metadata) {
    metadata.replacement_table[1] = reinterpret_cast<void*>(release_networkable_entry);
    metadata.replacement_table[5] = reinterpret_cast<void*>(networkable_entry);
}
void release_hooked_networkable(engine::VtableHookRegistry& registry, void* receiver) {
    engine::uninstall_vtable_hook(registry, receiver);
    engine::invoke<void>(receiver, 1);
}
} // namespace skeet::effects

// ============================================================================
// Subcomponent: free_look_capture.cpp
// ============================================================================

namespace skeet::effects {
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
float add(float a, float b) noexcept {
    return _mm_cvtss_f32(_mm_add_ss(_mm_set_ss(a), _mm_set_ss(b)));
}
/* [removed duplicate sub] */
/* [removed duplicate mul] */
/* [removed duplicate neg] */
void* base_trace_entity(void* candidate, const FreeLookCaptureBindings& b) {
    if (engine::invoke<bool>(b.engine_info, 3, candidate))
        return nullptr;
    return engine::invoke<void*>(candidate, 7);
}
struct CaptureTraceFilter {
    void* const* vtable;
    void* skip;
    std::uint32_t collision_group = 13, additional = 0;
    const FreeLookCaptureBindings* bindings;
};
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_CAPTURE_MEMBER __thiscall
#else
#define SKEET_CAPTURE_MEMBER
#endif
bool SKEET_CAPTURE_MEMBER accepts(void* receiver, void* candidate, std::uint32_t mask) {
    return free_look_capture_accepts(receiver, candidate, mask,
                                     *static_cast<CaptureTraceFilter*>(receiver)->bindings);
}
#undef SKEET_CAPTURE_MEMBER
struct alignas(16) CaptureEngineRay {
    std::array<float, 4> start{}, delta{}, start_offset{}, extents{};
    const void* world_axis_transform = nullptr;
    std::uint8_t is_ray = 0, swept = 1;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(CaptureTraceFilter, skip) == 4);
static_assert(offsetof(CaptureTraceFilter, collision_group) == 8);
static_assert(offsetof(CaptureTraceFilter, bindings) == 0x10);
static_assert(offsetof(CaptureEngineRay, world_axis_transform) == 0x40);
static_assert(offsetof(CaptureEngineRay, is_ray) == 0x44);
#endif
}

bool free_look_capture_accepts(void* receiver, void* candidate, std::uint32_t mask,
                               const FreeLookCaptureBindings& b) {
    if (!reinterpret_cast<engine::MemberFunction<bool, void*, std::uint32_t>>(
            b.trace_filter_vtable[0])(receiver, candidate, mask))
        return false;
    auto* entity = base_trace_entity(candidate, b);
    while (entity) {
        if (engine::invoke<bool>(entity, 162))
            return false;
        if (engine::invoke<bool>(entity, 158))
            return false;
        const auto handle = read<std::uint32_t>(entity, b.owner_handle);
        if (handle == UINT32_MAX)
            break;
        entity = engine::invoke<void*>(b.entity_list, 4, handle);
    }
    return true;
}

void capture_free_look_view_hook(void* player, void** original_table, void* setup,
                                 const FreeLookCaptureBindings& b) {
    reinterpret_cast<engine::MemberFunction<void, void*>>(original_table[337])(player, setup);
    if (engine::invoke<void*>(b.input, 32, std::int32_t(-1))) {
        std::array<float, 3> camera;
        engine::invoke<void>(b.input, 34, camera.data());
        constexpr float radians = 0.01745329238474369f;
        const float pitch = mul(camera[0], radians);
        const float sp = b.trigonometry.sine(pitch), cp = b.trigonometry.cosine(pitch);
        const float yaw = mul(camera[1], radians);
        const float sy = b.trigonometry.sine(yaw), cy = b.trigonometry.cosine(yaw);
        //43366128 calculates all three bases, including both roll imports.
        //The right and up vectors are local scratch and have no later reader.
        b.trigonometry.sine(0.0f);
        b.trigonometry.cosine(0.0f);
        const std::array<float, 3> forward{mul(cy, cp), mul(sy, cp), neg(sp)};
        std::array<float, 3> eye;
        engine::invoke<void>(player, 212, eye.data());
        eye[2] = sub(eye[2], sub(eye[2], read<float>(setup, 0xC0)));

        CaptureEngineRay ray;
        for (std::size_t axis = 0; axis != 3; ++axis) {
            ray.delta[axis] = sub(sub(eye[axis], mul(camera[2], forward[axis])), eye[axis]);
            ray.start[axis] = add(eye[axis], 0.0f);
            ray.start_offset[axis] = -0.0f;
            ray.extents[axis] = 9.0f;
        }
        const float squared =
            add(add(mul(ray.delta[1], ray.delta[1]), mul(ray.delta[0], ray.delta[0])),
                mul(ray.delta[2], ray.delta[2]));
        ray.swept = squared != 0.0f;
        //Slots1..3 are genuine engine entries. Only the recovered slot0 adds
        //the source owner-chain policy, with the exact native filter prefix.
        std::array<void*, 4> table{reinterpret_cast<void*>(&accepts), b.trace_filter_vtable[1],
                                   b.trace_filter_vtable[2], b.trace_filter_vtable[3]};
        CaptureTraceFilter filter{table.data(), player, 13, 0, &b};
        MovementCollisionTrace trace{};
        trace.hitgroup = -1;
        trace.hitbox = -1; //native fraction starts at zero
        engine::invoke<void>(b.engine_trace, 5, &ray, 0x200400Bu, &filter, &trace);
        if (1.0f > trace.fraction) {
            auto* input = b.input; //native reload after TraceRay
            write(input, 0xB4, mul(trace.fraction, read<float>(input, 0xB4)));
        }
    }
    auto* owner = b.free_look;
    const auto* frame = b.frame;
    auto& capture = free_look_owner_state(*owner).view;
    //Native first snapshots setup+B8, then writes three words sequentially.
    const auto captured = read<std::array<std::uint32_t, 3>>(setup, 0xB8);
    for (std::size_t axis = 0; axis != 3; ++axis)
        std::memcpy(&capture.origin[axis], &captured[axis], sizeof captured[axis]);
    capture.frame = *frame;
}

} //namespace skeet::effects

// ============================================================================
// Subcomponent: free_look_owner.cpp
// ============================================================================

namespace skeet::effects {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_FREE_OWNER_MEMBER __thiscall
#else
#define SKEET_FREE_OWNER_MEMBER
#endif
using PrimaryEntry = void(SKEET_FREE_OWNER_MEMBER*)(void*);
void SKEET_FREE_OWNER_MEMBER destroy_entry(void*);
void SKEET_FREE_OWNER_MEMBER reset_entry(void*);
void SKEET_FREE_OWNER_MEMBER forward_reset_entry(void*);
void SKEET_FREE_OWNER_MEMBER free_unchanged(void*) {}
//Complete primary4346FA68;433CF2F5 forwards slot7 to the current slot5.
const std::array<PrimaryEntry, 9> primary_table{destroy_entry, free_unchanged,      free_unchanged,
                                                free_unchanged,free_unchanged,      reset_entry,
                                                free_unchanged,forward_reset_entry, free_unchanged};
}

class FreeLookOwner {
public:
    const PrimaryEntry* vtable = nullptr;
    std::uintptr_t preserved_second_word; //native+4 is not written by this ctor
    FreeLookViewObserver observer;
    FreeLookViewRuntime runtime;
    engine::JobMemory memory;
    FreeLookOwner** active;
    FreeLookOwnerState* canonical;

    FreeLookOwner(engine::OwnerRegistryBinding lifecycle, engine::OwnerRegistryBinding updates,
                  FreeLookOwnerBindings bindings)
        : observer(runtime, false),
          runtime(bindings.state.command, bindings.state.view, bindings.view),
          memory(lifecycle.memory), active(&bindings.active), canonical(&bindings.state) {
        lifecycle.append(this); //43484A3A ->433676A0, primary word still zero
        network::register_update_observer(updates, observer.endpoint); //owner+8 ->433B8C1C
        vtable = primary_table.data(); //43484A4E, native4346FA68
        observer.initialize_vtable(); //43484A57, native4347269C
        canonical->command.delta = {0.0f, 0.0f, 0.0f}; //43484A5E..43484A62 ONLY
    }
};
#if UINTPTR_MAX == UINT32_MAX
// Clang's x86 layout extension verifies this reference-bearing native prefix.
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(FreeLookOwner, preserved_second_word) == 4);
static_assert(offsetof(FreeLookOwner, observer) == 8);
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
static_assert(sizeof(FreeLookOwner) <= native_free_look_owner_allocation_size);
#endif

namespace {
void SKEET_FREE_OWNER_MEMBER destroy_entry(void* object) {
    auto* owner = static_cast<FreeLookOwner*>(object);
    destroy_free_look_owner(owner, *owner->active, owner->memory);
}
void SKEET_FREE_OWNER_MEMBER reset_entry(void* object) {
    static_cast<FreeLookOwner*>(object)->runtime.reset(); //4341781B
}
void SKEET_FREE_OWNER_MEMBER forward_reset_entry(void* object) {
    engine::invoke<void>(object, 5); //433CF2F5: current primary slot5, not a reset copy
}
void initialize_entry(void* context) {
    auto& source = *static_cast<FreeLookInitialization*>(context);
    create_free_look_owner(source.lifecycle, source.updates, source.bindings);
}
#undef SKEET_FREE_OWNER_MEMBER
}

std::size_t free_look_owner_size() noexcept {
    //The production x86 wrapper fits the exact native0x38 allocation. Pointer
    //width changes the semantic wrapper's storage requirement on other hosts.
    return sizeof(FreeLookOwner) > native_free_look_owner_allocation_size
               ? sizeof(FreeLookOwner)
               : native_free_look_owner_allocation_size;
}
FreeLookOwner& construct_free_look_owner(void* storage, engine::OwnerRegistryBinding lifecycle,
                                         engine::OwnerRegistryBinding updates,
                                         FreeLookOwnerBindings bindings) {
    return *::new (storage) FreeLookOwner(lifecycle, updates, bindings);
}
FreeLookOwner* create_free_look_owner(engine::OwnerRegistryBinding lifecycle,
                                      engine::OwnerRegistryBinding updates,
                                      FreeLookOwnerBindings bindings) {
    const auto memory = lifecycle.memory;
    const auto allocation_size = free_look_owner_size() + 0x20u; //43387448; x86=0x58
    auto* raw = memory.allocate(memory.process_heap, 8, allocation_size);
    if (!raw) {
        bindings.active = nullptr; //43484A67 XOR EBX,EBX, then recorded43484A69 store
        return nullptr;
    }
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15u) & ~std::uintptr_t(15u);
    const auto displacement = static_cast<std::uint32_t>(header - address);
    const auto total = static_cast<std::uint32_t>(allocation_size);
    std::memcpy(reinterpret_cast<void*>(header + 8u), &displacement, sizeof displacement);
    std::memcpy(reinterpret_cast<void*>(header + 12u), &total, sizeof total);
    //Native state occupied this same zeroed heap allocation. The source's
    //command and capture consumers borrow one shared state projection.
    std::memset(&bindings.state, 0, sizeof bindings.state);
    auto& owner = construct_free_look_owner(reinterpret_cast<void*>(header + 16u), lifecycle,
                                            updates, bindings);
    bindings.active = &owner; //mem_int3.h:516, live EBX is this constructed owner
    return &owner;
}
void destroy_free_look_owner(FreeLookOwner* owner, FreeLookOwner*& active,
                             engine::JobMemory memory) {
    active = nullptr; //43406BFF is before433837D0's null-object guard
    if (!owner)
        return;
    owner->~FreeLookOwner();
    auto* bytes = reinterpret_cast<std::byte*>(owner);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
FreeLookOwnerState& free_look_owner_state(FreeLookOwner& owner) noexcept {
    return *owner.canonical;
}
FreeLookViewRuntime& free_look_view_runtime(FreeLookOwner& owner) noexcept {
    return owner.runtime;
}
network::UpdateObserver& free_look_view_endpoint(FreeLookOwner& owner) noexcept {
    return owner.observer.endpoint;
}
void* free_look_lifecycle_object(FreeLookOwner& owner) noexcept {
    return &owner;
}
void bind_free_look_initializer(client::NativeStartupSources& sources,
                                FreeLookInitialization& source) {
    sources.bind_initializer({0x43484A2Bu, &source, initialize_entry});
}

} //namespace skeet::effects

// ============================================================================
// Subcomponent: free_look_runtime_owner.cpp
// ============================================================================

namespace skeet::effects {
namespace {
client::ClientRuntimeStorage& checked(client::ClientRuntimeStorage& storage,
                                      const client::EngineRuntimeBindings& engine,
                                      client::ConfigurationRuntimeOwner& config,
                                      client::CrtRuntimeOwner& crt) {
    auto& data = storage.data();
    if (&engine.variables.data() != &data || &config.storage() != &storage ||
        &crt.storage() != &storage)
        throw std::invalid_argument("Free-look must share the canonical process owners");
    if (!data.native_cell<void*>(0x4346A894u) || !data.native_cell<void*>(0x43469264u) ||
        !data.native_cell<void*>(0x43468304u) || !data.native_cell<void* const*>(0x434758B4u) ||
        !data.native_cell<std::int32_t*>(0x434678F8u) || !engine.variables.at(0x43467AA0u).get() ||
        !engine.variables.at(0x4346A7DCu).get())
        throw std::invalid_argument(
            "Free-look requires its acquired camera, trace and clock inputs");
    return storage;
}
engine::OwnerRegistryBinding registry(client::ClientRuntimeStorage& storage,
                                      std::uint32_t address) {
    return {storage.owner_registries().at(address), storage.memory()};
}
}
FreeLookRuntimeOwner::FreeLookRuntimeOwner(client::ClientRuntimeStorage& storage,
                                           const client::EngineRuntimeBindings& engine,
                                           client::ConfigurationRuntimeOwner& config,
                                           client::CrtRuntimeOwner& crt)
    : storage_(checked(storage, engine, config, crt)),
      view_{storage.data().native_cell<void*>(0x4346A894u),
            storage.data().native_cell<void*>(0x43469264u),
            storage.data().native_cell<void*>(0x43475808u),
            static_cast<std::uint32_t>(static_cast<std::ptrdiff_t>(engine::DecodedOffset(
                storage.offsets().globals, 0x43468E78u, 0x434684E4u, 0x9D920E35u, 0))),
            *storage.data().native_cell<std::int32_t*>(0x434678F8u),
            engine.variables.at(0x43467AA0u),
            engine.variables.at(0x4346A7DCu),
            storage.data().native_cell<void*>(0x43468304u),
            storage.data().native_cell<void* const*>(0x434758B4u),
            config.free_look().key.value.binding,
            config.effects_settings().third_person_alive.value,
            config.effects_settings().third_person_key.value.binding,
            storage.input_runtime().bitmap(),
            crt.trigonometry()},
      initialization_{registry(storage, 0x43475B78u),
                      registry(storage, 0x43475988u),
                      {state_, view_, storage.data().native_cell<FreeLookOwner*>(0x43475A64u)}} {}
void FreeLookRuntimeOwner::bind_initializers(client::NativeStartupSources& sources) {
    bind_free_look_initializer(sources, initialization_);
}
}

// ============================================================================
// Subcomponent: free_look_view.cpp
// ============================================================================

namespace skeet::effects {
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
/* [removed duplicate add] */
/* [removed duplicate sub] */
/* [removed duplicate mul] */
/* [removed duplicate neg] */
float clamp(float value, float low, float high) noexcept {
    return _mm_cvtss_f32(
        _mm_min_ss(_mm_max_ss(_mm_set_ss(value), _mm_set_ss(low)), _mm_set_ss(high)));
}
void normalize(float* angles) noexcept {
    angles[0] = normalize_movement_angle(angles[0]);
    angles[1] = normalize_movement_angle(angles[1]);
    angles[2] = normalize_movement_angle(angles[2]);
}
const void* parent(const void* variable) noexcept {
    for (;;) {
        auto* next = read<void*>(variable, 0x1C);
        if (next == variable)
            return variable;
        variable = next;
    }
}
float encoded_float(const void* variable) noexcept {
    auto* root = parent(variable);
    return std::bit_cast<float>(read<std::uint32_t>(root, 0x2C) ^
                                std::uint32_t(reinterpret_cast<std::uintptr_t>(root)));
}
bool encoded_integer_is_zero(const void* variable) noexcept {
    auto* root = parent(variable);
    return read<std::uint32_t>(root, 0x30) == std::uint32_t(reinterpret_cast<std::uintptr_t>(root));
}
//4346EDC0's four entries only forward to this external engine filter table.
struct FreeLookTraceFilter {
    void* const* vtable;
    void* skip;
    std::uint32_t collision_group = 0, additional = 0;
};
struct alignas(16) FreeLookEngineRay {
    std::array<float, 4> start{}, delta{}, start_offset{}, extents{};
    const void* world_axis_transform = nullptr;
    std::uint8_t is_ray = 0, swept = 1;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(FreeLookTraceFilter, skip) == 4);
static_assert(offsetof(FreeLookEngineRay, world_axis_transform) == 0x40);
static_assert(offsetof(FreeLookEngineRay, is_ray) == 0x44);
#endif
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_FREE_PHASE __thiscall
#define SKEET_FREE_FAST __fastcall
#else
#define SKEET_FREE_PHASE
#define SKEET_FREE_FAST
#endif
void SKEET_FREE_PHASE free_empty_phase(void*) {}
void SKEET_FREE_FAST empty_player(void*, void*, std::uintptr_t, std::uintptr_t, std::uintptr_t) {
} //4337D653
void SKEET_FREE_FAST free_empty_post(void*, void*) {}
void SKEET_FREE_FAST free_view(void* receiver, float* origin, float* angles, float*) {
    static_cast<FreeLookViewObserver*>(receiver)->runtime->update(origin, angles);
}
const network::UpdateObserverVtable free_look_vtable{
    {free_empty_phase, free_empty_phase, free_empty_phase, free_empty_phase, free_empty_phase},
    empty_player,
    free_view,
    free_empty_post};
#undef SKEET_FREE_PHASE
#undef SKEET_FREE_FAST
}

void FreeLookViewRuntime::reset() noexcept {
    state.delta = {0.0f, 0.0f, 0.0f};
    state.active = 0;
}
bool FreeLookViewRuntime::third_person_active() {
    if (engine::invoke<void*>(bindings.input, 32, std::int32_t(-1)))
        return true;
    return bindings.third_person && hotkey_active(bindings.third_person_key, bindings.keys);
}
void FreeLookViewRuntime::update(float* origin, float* angles) {
    if (!hotkey_active(bindings.free_look_key, bindings.keys) || !state.active) {
        reset();
        return;
    }
    auto* player = bindings.local_player;
    if (!player || read<std::uint8_t>(player, bindings.life_state)) {
        reset();
        return;
    }
    angles[0] = add(angles[0], state.delta[0]);
    angles[1] = add(state.delta[1], angles[1]);
    normalize(angles);
    angles[0] = clamp(angles[0], -89.0f, 89.0f);
    if (third_person_active() && capture.frame == bindings.frame) {
        //The input output is intentionally discarded; this virtual call still
        //precedes the player query and can change live engine bindings.
        std::array<float, 3> unused;
        engine::invoke<void>(bindings.input, 34, unused.data());
        const float saved_height = capture.origin[2];
        std::array<float, 3> eye;
        engine::invoke<void>(player, 212, eye.data());
        origin[0] = eye[0];
        origin[1] = eye[1];
        origin[2] = eye[2];
        origin[2] = sub(origin[2], sub(origin[2], saved_height));

        constexpr float radians = 0.01745329238474369f;
        const float yaw = mul(angles[1], radians);
        const float sine_yaw = bindings.trigonometry.sine(yaw);
        const float cosine_yaw = bindings.trigonometry.cosine(yaw);
        const float pitch = mul(angles[0], radians);
        const float sine_pitch = bindings.trigonometry.sine(pitch);
        const float cosine_pitch = bindings.trigonometry.cosine(pitch);
        const std::array<float, 3> forward{mul(cosine_pitch, cosine_yaw),
                                           mul(cosine_pitch, sine_yaw), neg(sine_pitch)};
        const float distance = encoded_float(bindings.camera_distance);
        FreeLookEngineRay ray;
        for (std::size_t axis = 0; axis != 3; ++axis) {
            ray.start[axis] = add(origin[axis], 0.0f);
            ray.delta[axis] = sub(sub(origin[axis], mul(forward[axis], distance)), origin[axis]);
            ray.start_offset[axis] = -0.0f;
            ray.extents[axis] = 9.0f;
        }
        const float squared =
            add(add(mul(ray.delta[1], ray.delta[1]), mul(ray.delta[0], ray.delta[0])),
                mul(ray.delta[2], ray.delta[2]));
        ray.swept = squared != 0.0f;
        FreeLookTraceFilter filter{bindings.trace_filter_vtable, player};
        MovementCollisionTrace trace{};
        trace.fraction = 1.0f;
        trace.hitgroup = -1;
        trace.hitbox = -1;
        engine::invoke<void>(bindings.engine_trace, 5, &ray, 0x200400Bu, &filter, &trace);
        const float actual_distance =
            1.0f > trace.fraction ? mul(trace.fraction, distance) : distance;
        const float backward = neg(actual_distance);
        origin[0] = add(mul(backward, forward[0]), origin[0]);
        origin[1] = add(mul(backward, forward[1]), origin[1]);
        origin[2] = add(mul(backward, forward[2]), origin[2]);
    }
    if (third_person_active())
        return;
    const bool mode_zero = encoded_integer_is_zero(bindings.camera_mode);
    const float yaw_scale = mode_zero ? 1.0f : -1.0f;
    const float yaw_minimum = mode_zero ? -20.0f : -60.0f;
    const float yaw_maximum = mode_zero ? 60.0f : 20.0f;
    auto* prediction = bindings.prediction;
    const auto previous = read<std::uint8_t>(prediction, 8);
    write(prediction, 8, std::uint8_t(1));
    std::array<float, 3> local_angles{angles[0], angles[1], angles[2]};
    local_angles[0] = add(clamp(neg(state.delta[0]), -30.0f, 10.0f), local_angles[0]);
    local_angles[1] =
        add(clamp(mul(state.delta[1], yaw_scale), yaw_minimum, yaw_maximum), local_angles[1]);
    normalize(local_angles.data());
    local_angles[0] = clamp(local_angles[0], -89.0f, 89.0f);
    engine::invoke<void>(player, 278, origin, local_angles.data());
    write(prediction, 8, previous);
}
FreeLookViewObserver::FreeLookViewObserver(FreeLookViewRuntime& runtime, bool initialize_table)
    : endpoint{initialize_table ? &free_look_vtable : nullptr}, runtime(&runtime) {}
void FreeLookViewObserver::initialize_vtable() noexcept {
    endpoint.vtable = &free_look_vtable;
}

} //namespace skeet::effects

// ============================================================================
// Subcomponent: glow.cpp
// ============================================================================

#if defined(_M_IX86) && !defined(__i386__)
#error The captured glow registration ABI requires the x86 Clang/GNU inline-assembly adapter.
#endif

namespace skeet::effects {
namespace {
/* [removed duplicate field] */
template <class T> void store(void* object, std::uint32_t offset, T value) {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(value));
}
std::array<float, 4> color(std::uint32_t packed, float scale = 0.003921568859368563f) {
    std::array<float, 4> result;
    for (unsigned i = 0; i != 4; ++i)
        result[i] = float(std::uint8_t(packed >> (i * 8))) * scale;
    return result;
}
std::int32_t register_glow(GlowEngine& engine, void* entity, const float* rgb, float alpha,
                           bool occluded) {
#if defined(__i386__)
    // The captured client entry consumes alpha in XMM3 and three zero floats
    // in XMM0..2. Its remaining arguments use the Microsoft fastcall ABI.
    struct Call {
        void* function;
        GlowRegistry* manager;
        void* entity;
        const float* color;
        std::uint32_t occluded, unoccluded;
        std::int32_t split;
        float alpha;
    };
    const Call call{
        engine.registration_function, engine.registry, entity, rgb, occluded, 0, -1, alpha};
    std::int32_t result;
    asm volatile("pushl %%esi\n\t"
                 "movl %[packet], %%esi\n\t"
                 "movl 4(%%esi), %%ecx\n\t"
                 "xorl %%edx, %%edx\n\t"
                 "xorps %%xmm0, %%xmm0\n\t"
                 "xorps %%xmm1, %%xmm1\n\t"
                 "xorps %%xmm2, %%xmm2\n\t"
                 "movss 28(%%esi), %%xmm3\n\t"
                 "pushl 24(%%esi)\n\t"
                 "pushl 20(%%esi)\n\t"
                 "pushl 16(%%esi)\n\t"
                 "pushl 12(%%esi)\n\t"
                 "pushl 8(%%esi)\n\t"
                 "call *0(%%esi)\n\t"
                 "popl %%esi"
                 : "=a"(result)
                 : [packet] "r"(&call)
                 : "ecx", "edx", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                   "memory", "cc");
    return result;
#else
    return reinterpret_cast<GlowEngine::HostRegistration>(engine.registration_function)(
        engine.registry, entity, rgb, alpha, occluded, false, -1);
#endif
}
}

void update_glow(GlowEngine& engine, void* entity, std::uint32_t index,
                 const std::array<float, 4>& rgba) {
    auto* manager = engine.registry;
    if (!manager || !engine.registration_function || index >= std::uint32_t(manager->count))
        return;
    auto* object = manager->objects + index;
    if (!object || (object->rgba == rgba && object->render_occluded != 0) ||
        object->entity != entity || object->next_free != -2)
        return;
    const auto saved = manager->first_free;
    manager->first_free = std::int32_t(index);
    register_glow(engine, entity, rgba.data(), rgba[3], true);
    manager->first_free = saved;
}
void clear_glow(GlowEngine& engine, void* entity, std::uint32_t index) {
    auto* manager = engine.registry;
    if (!manager || !engine.registration_function || index >= std::uint32_t(manager->count))
        return;
    auto* object = manager->objects + index;
    if (!object || (object->rgba[3] == 0.0f && object->render_occluded == 0) ||
        object->entity != entity || object->next_free != -2)
        return;
    const auto saved = manager->first_free;
    manager->first_free = std::int32_t(index);
    const std::array<float, 3> zero{};
    register_glow(engine, entity, zero.data(), 0, false);
    manager->first_free = saved;
}
void GlowRuntime::clear_bomb() {
    if (bomb_handle == -1)
        return;
    void* bomb = entities.by_handle(bomb_handle);
    if (bomb) {
        const auto index = field<std::int32_t>(bomb, entities.bomb_glow_offset());
        if (index > 0)
            clear_glow(engine, bomb, std::uint32_t(index));
    }
    bomb_handle = -1;
}
void GlowRuntime::update() {
    using engine::invoke;
    void* local_player;
    if (players.glow.value && !entities.suspended() && entities.activation() &&
        (local_player = entities.local_player()) && engine.registry) {
        const auto rgba = color(players.glow_color.value);
        const auto limit = entities.max_clients();
        for (std::int32_t index = 1; index <= limit; ++index) {
            void* entity = entities.by_index(index);
            if (!entity || entities.life_state(entity) || invoke<std::int32_t>(entity, 122) <= 0 ||
                invoke<bool>(static_cast<std::byte*>(entity) + 8, 9) ||
                entities.suppressed(entity) || entities.eligibility_state(entity))
                continue;
            if (!players.teammates.value && !entities.enemy(local_player, entity))
                continue;
            if (entities.disable_visuals(std::uint32_t(index)))
                continue;
            update_glow(engine, entity, entities.player_glow_index(entity), rgba);
        }
    }
    bool enable_weapons = true;
    if (!(world.dropped_weapons.value & 4) || entities.suspended() || !entities.activation()) {
        if (!weapons_active)
            goto bomb;
        enable_weapons = false;
    }
    weapons_active = enable_weapons;
    if (engine.registry) {
        const auto rgba = color(world.dropped_weapon_color.value);
        const auto limit = entities.entity_limit();
        for (std::int32_t index = entities.max_clients() + 1; index < limit; ++index) {
            if (std::uint32_t(index) & 0xFFFF0000u)
                continue;
            void* entity = entities.by_index(index);
            if (!entity || !invoke<bool>(entity, 166))
                continue;
            const auto glow = entities.weapon_glow_index(entity);
            if (enable_weapons)
                update_glow(engine, entity, glow, rgba);
            else
                clear_glow(engine, entity, glow);
        }
    }
bomb:
    auto* planted_bombs = entities.planted_bombs();
    if (!planted_bombs)
        return;
    if (world.bomb.enabled.value && !entities.suspended() && entities.activation() &&
        field<std::int32_t>(planted_bombs, 0xC)) {
        auto* entries = field<void**>(planted_bombs, 0);
        void* entity = entries[0];
        if (entity && entities.bomb_active(entity)) {
            const auto handle = invoke<const std::int32_t*>(entity, 2);
            if (*handle != bomb_handle)
                clear_bomb();
            auto rgba = color(world.bomb.color.value, 0.003921568393707275f);
            rgba[3] = 1.0f;
            const auto glow_offset = entities.bomb_glow_offset();
            const auto index = field<std::int32_t>(entity, glow_offset);
            if (index == -1) {
                const auto assigned = engine.registration_function
                                          ? register_glow(engine, entity, rgba.data(), 1, true)
                                          : -1;
                store(entity, glow_offset, assigned);
            } else
                update_glow(engine, entity, std::uint32_t(index), rgba);
            bomb_handle = *handle;
            return;
        }
    }
    clear_bomb();
}

} // namespace skeet::effects

// ============================================================================
// Subcomponent: glow_engine.cpp
// ============================================================================

namespace skeet::effects {
namespace {
/* [removed duplicate field] */
}
std::uint32_t GlowEntityEngine::offset(std::uint32_t pointer, std::uint32_t key,
                                       std::uint32_t consumer) const {
    const auto& globals = bindings_.offsets.globals;
    return engine::decode_receive_offset(globals.pointers.at(pointer), globals.keys.at(key),
                                         consumer);
}
void* GlowEntityEngine::by_index(std::int32_t index) const {
    return engine::invoke<void*>(bindings_.entity_list, 3, index);
}
void* GlowEntityEngine::by_handle(std::int32_t handle) const {
    return engine::invoke<void*>(bindings_.entity_list, 4, handle);
}
std::int32_t GlowEntityEngine::entity_limit() const {
    return field<std::int32_t>(bindings_.entity_list, 0x24);
}
bool GlowEntityEngine::life_state(void* entity) const {
    return field<std::uint8_t>(entity, offset(0x43468E78u, 0x434684E4u, 0x9D920E35u)) != 0;
}
bool GlowEntityEngine::suppressed(void* entity) const {
    return field<std::uint8_t>(entity, offset(0x4346A5DCu, 0x43468430u, 0xCDE290D2u)) != 0;
}
bool GlowEntityEngine::eligibility_state(void* entity) const {
    return field<std::uint8_t>(entity, offset(0x43468BF8u, 0x43467B8Cu, 0xD070684Cu)) != 0;
}
std::uint32_t GlowEntityEngine::player_glow_index(void* entity) const {
    return field<std::uint32_t>(entity, offset(0x434670F4u, 0x43467B94u, 0x442A38AEu));
}
std::uint32_t GlowEntityEngine::weapon_glow_index(void* entity) const {
    return field<std::uint32_t>(entity, offset(0x4346A62Cu, 0x43467D24u, 0x442A38AEu));
}
bool GlowEntityEngine::bomb_active(void* entity) const {
    return field<std::uint8_t>(entity, offset(0x4346A6C4u, 0x43468EF4u, 0x0E5BECE5u)) != 0;
}
std::uint32_t GlowEntityEngine::bomb_glow_offset() const {
    return offset(0x43467E54u, 0x4346A72Cu, 0xA7B0989Eu);
}
bool GlowEntityEngine::enemy(void* local, void* target) const {
    // Reuse the complete4339EF8C predicate. Its two property operands and
    // decoded ConVar objects are bound at this actual call, after eligibility.
    rage::TargetOffsets offsets{};
    offsets.team = offset(0x434688B0u, 0x4346A518u, 0x8B5586F6u);
    offsets.survival_team = offset(0x434678ACu, 0x43467B7Cu, 0x430C8993u);
    const rage::TargetBindings targeting{local,
                                         bindings_.entity_list,
                                         bindings_.game_type,
                                         bindings_.teammates_are_enemies,
                                         bindings_.player_adjustments,
                                         offsets};
    return rage::target_is_enemy(targeting, target);
}
bool GlowEntityEngine::disable_visuals(std::uint32_t index) const noexcept {
    return bindings_.player_adjustments.records[index].adjustments.disable_visuals != 0;
}
} //namespace skeet::effects

// ============================================================================
// Subcomponent: harmless_molotov.cpp
// ============================================================================

namespace skeet::effects {
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
/* [removed duplicate parent] */
/* [removed duplicate integer] */
float floating(const void* p) {
    p = parent(p);
    return std::bit_cast<float>(read<std::uint32_t>(p, 0x2c) ^
                                std::uint32_t(reinterpret_cast<std::uintptr_t>(p)));
}
void** original(void* receiver) {
    const auto table = *static_cast<void***>(receiver);
    return static_cast<engine::VtableHookMetadata*>(table[-2])->original_table;
}
std::array<float, 3> to_hsv(const std::array<float, 3>& rgb) { //433EDB18
    const auto high = maximum_number(maximum_number(rgb[0], rgb[1]), rgb[2]);
    const auto low = minimum_number(minimum_number(rgb[0], rgb[1]), rgb[2]);
    const auto delta = high - low;
    const auto saturation = high == 0.0f ? 0.0f : delta / high;
    if (saturation == 0.0f)
        return {-1.0f, saturation, high};
    float hue;
    if (rgb[0] == high)
        hue = (rgb[1] - rgb[2]) / delta;
    else if (rgb[1] == high)
        hue = (rgb[2] - rgb[0]) / delta + 2.0f;
    else
        hue = (rgb[0] - rgb[1]) / delta + 4.0f;
    hue *= 60.0f;
    if (hue < 0.0f)
        hue += 360.0f;
    return {hue, saturation, high};
}
void from_hsv(const std::array<float, 3>& hsv, std::array<float, 3>& rgb) { //43406FDD
    const auto s = hsv[1], v = hsv[2];
    if (s == 0.0f) {
        rgb = {v, v, v};
        return;
    }
    const auto h = (hsv[0] == 360.0f ? 0.0f : hsv[0]) / 60.0f;
    const auto sector = _mm_cvttss_si32(_mm_set_ss(h));
    const auto f = h - static_cast<float>(sector);
    const auto low = (1.0f - s) * v, falling = (1.0f - s * f) * v,
               rising = (1.0f - (1.0f - f) * s) * v;
    switch (sector) {
    case 0:
        rgb = {v, rising, low};
        break;
    case 1:
        rgb = {falling, v, low};
        break;
    case 2:
        rgb = {low, v, rising};
        break;
    case 3:
        rgb = {low, falling, v};
        break;
    case 4:
        rgb = {rising, low, v};
        break;
    case 5:
        rgb = {v, low, falling};
        break;
    }
}
bool teammates(void* first, void* second, const rage::TargetBindings& b) { //433F93D2
    if (!first || !second)
        return false;
    if (first == second)
        return true;
    if (integer(b.game_type) == 6) {
        const auto a = read<std::int32_t>(first, std::uint32_t(b.offsets.survival_team));
        const auto c = read<std::int32_t>(second, std::uint32_t(b.offsets.survival_team));
        return a != -1 && c != -1 && a == c;
    }
    const auto a = read<std::int32_t>(first, std::uint32_t(b.offsets.team));
    return (a == 2 || a == 3) && a == read<std::int32_t>(second, std::uint32_t(b.offsets.team)) &&
           integer(b.teammates_are_enemies) == 0;
}
HarmlessMolotovRuntime** molotov_active_slot = nullptr;
engine::VtableHookRegistry* entity_hooks =
    nullptr; //same global hook registry outlives the effect owner
std::uint32_t destroy_particle_primary(engine::VtableHookRegistry& hooks, void* receiver,
                                       std::uintptr_t flags) {
    auto* bytes = static_cast<std::byte*>(receiver);
    engine::uninstall_vtable_hook(hooks, bytes + 4);
    engine::uninstall_vtable_hook(hooks, bytes + 8);
    engine::uninstall_vtable_hook(hooks, receiver);
    return engine::invoke<std::uint32_t>(receiver, 0, flags);
}
std::uint32_t destroy_particle_secondary(engine::VtableHookRegistry& hooks, void* receiver,
                                         std::uintptr_t flags) {
    auto* bytes = static_cast<std::byte*>(receiver);
    engine::uninstall_vtable_hook(hooks, receiver);
    engine::uninstall_vtable_hook(hooks, bytes + 4);
    engine::uninstall_vtable_hook(hooks, bytes - 4);
    return engine::invoke<std::uint32_t>(receiver, 0, flags);
}
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_MOLOTOV_ENTRY __fastcall
#else
#define SKEET_MOLOTOV_ENTRY
#endif
std::uint32_t SKEET_MOLOTOV_ENTRY molotov_render_entry(void* receiver, void*, std::uintptr_t a,
                                               std::uintptr_t b) {
    if (auto* runtime = *molotov_active_slot)
        return runtime->render(receiver, a, b);
    return reinterpret_cast<engine::MemberFunction<std::uint32_t, std::uintptr_t, std::uintptr_t>>(
        original(receiver)[9])(receiver, a, b);
}
std::uint32_t SKEET_MOLOTOV_ENTRY create_entry(void* receiver, void*, const char* name,
                                               void* created) {
    if (auto* runtime = *molotov_active_slot)
        return runtime->created(receiver, name, created);
    return reinterpret_cast<engine::MemberFunction<std::uint32_t, const char*, void*>>(
        original(receiver)[110])(receiver, name, created);
}
std::uint32_t SKEET_MOLOTOV_ENTRY primary_entry(void* receiver, void*, std::uintptr_t flags) {
    return destroy_particle_primary(*entity_hooks, receiver, flags);
}
std::uint32_t SKEET_MOLOTOV_ENTRY secondary_entry(void* receiver, void*, std::uintptr_t flags) {
    return destroy_particle_secondary(*entity_hooks, receiver, flags);
}
#undef SKEET_MOLOTOV_ENTRY
}
HarmlessMolotovState::HarmlessMolotovState(bool initialize) {
    if (initialize)
        initialize_native_storage();
}
void HarmlessMolotovState::initialize_native_storage() {
    definition_hashes = {0xCE18A28F, 0x084260DE, 0x347C6739, 0x9C63D7D8, 0x91410D5F,
                         0xD3DD5603, 0xB96369ED, 0xC9490DC3, 0x20EFCF5C, 0x0A3CC8AF};
    colors.reserve(1024);
    alphas.reserve(1024);
}
void HarmlessMolotovState::release_native_storage() noexcept {
    std::vector<ParticleAlphaBackup>().swap(alphas);
    std::vector<ParticleColorBackup>().swap(colors);
    std::vector<std::uint32_t>().swap(definition_hashes);
}
void HarmlessMolotovRuntime::apply_collection(void* collection) {
    std::uint32_t hash = 0;
    if (const auto owner = read<void*>(collection, 0x48))
        if (const auto definition = read<void*>(owner, 0x50))
            hash = events::event_hash(engine::invoke<const char*>(definition, 0));
    const bool change_color =
        std::find(state.definition_hashes.begin(), state.definition_hashes.end(), hash) !=
        state.definition_hashes.end();
    const bool change_alpha = state.alpha < 1.0f;
    const auto count = read<std::int32_t>(collection, 0x30);
    if ((change_color || change_alpha) && count <= read<std::int32_t>(collection, 0x50) &&
        count > 0) {
        for (std::int32_t i = 0; i < read<std::int32_t>(collection, 0x30); ++i) {
            if (change_color) {
                auto* values = read<float*>(collection, 0xf8) +
                               (i / 4 * read<std::int32_t>(collection, 0x158) + (i & 3));
                const std::array<float, 3> before{values[0], values[4], values[8]};
                state.colors.push_back({values,
                                        {std::bit_cast<std::uint32_t>(before[0]),
                                         std::bit_cast<std::uint32_t>(before[1]),
                                         std::bit_cast<std::uint32_t>(before[2])}});
                const auto hsv = to_hsv(before);
                auto amount = _mm_set_ss(float(settings.molotov_percent.value) * 0.01f);
                amount = _mm_min_ss(_mm_max_ss(amount, _mm_setzero_ps()), _mm_set_ss(1.0f));
                const auto saturation = (state.hsv[1] - hsv[1]) * _mm_cvtss_f32(amount) + hsv[1];
                std::array<float, 3> changed;
                from_hsv({state.hsv[0], saturation, hsv[2]}, changed);
                values[0] = changed[0];
                values[4] = changed[1];
                values[8] = changed[2];
            }
            if (change_alpha) {
                auto* alpha = read<float*>(collection, 0xfc) +
                              (i / 4 * read<std::int32_t>(collection, 0x15c) + (i & 3));
                state.alphas.push_back({alpha, std::bit_cast<std::uint32_t>(*alpha)});
                *alpha = state.alpha * (*alpha);
            }
        }
    }
    for (void* child = read<void*>(collection, 0x8c); child; child = read<void*>(child, 0x70))
        apply_collection(child);
}
std::uint32_t HarmlessMolotovRuntime::render(void* receiver, std::uintptr_t first,
                                             std::uintptr_t second) {
    if (settings.harmless_molotovs.value) {
        state.colors.clear();
        state.alphas.clear();
        for (unsigned i = 0; i < 3; ++i)
            state.color[i] = float(std::uint8_t(settings.harmless_molotov_color.value >> (i * 8))) *
                             0.003921568859368563f;
        state.hsv = to_hsv(state.color);
        state.alpha = float(std::uint8_t(settings.harmless_molotov_color.value >> 24)) *
                      0.003921568859368563f;
        apply_collection(static_cast<std::byte*>(receiver) + 8);
    }
    const auto result =
        reinterpret_cast<engine::MemberFunction<std::uint32_t, std::uintptr_t, std::uintptr_t>>(
            original(receiver)[9])(receiver, first, second);
    if (settings.harmless_molotovs.value) {
        for (const auto& saved : state.colors) {
            write(saved.address, 0, saved.values[0]);
            write(saved.address, 0x10, saved.values[1]);
            write(saved.address, 0x20, saved.values[2]);
        }
        state.colors.clear();
        for (const auto& saved : state.alphas)
            write(saved.address, 0, saved.value);
        state.alphas.clear();
    }
    return result;
}
std::uint32_t HarmlessMolotovRuntime::created(void* receiver, const char* name, void* created) {
    if (settings.harmless_molotovs.value && events::event_hash(name) == 0x19E08E80u &&
        !engine::invoke<bool>(bindings.engine, 82) && !engine::invoke<bool>(bindings.engine, 93)) {
        auto* local = *bindings.local_player;
        void* owner = nullptr;
        const auto handle = read<std::uint32_t>(receiver, bindings.owner_handle);
        if (handle != UINT32_MAX) {
            auto* entity = engine::invoke<void*>(bindings.entity_list, 4, handle);
            if (entity && engine::invoke<bool>(entity, 158))
                owner = entity;
        }
        if (local && owner && local != owner &&
            teammates(local, owner, bindings.relationships.capture(local)) &&
            (integer(reinterpret_cast<void*>(
                 std::uintptr_t(bindings.encoded_friendly_fire ^ 0x211963A6u))) == 0 ||
             floating(reinterpret_cast<void*>(std::uintptr_t(
                 bindings.encoded_grenade_damage_reduction ^ 0xD5F0540Cu))) == 0.0f)) {
            auto* first = engine::invoke<void*>(static_cast<std::byte*>(created) + 8, 0);
            auto* second = engine::invoke<void*>(static_cast<std::byte*>(created) + 4, 5);
            if (first && second) {
                engine::install_vtable_hook(bindings.hook_registry, created,
                                            *bindings.entity_hooks[0]);
                engine::install_vtable_hook(bindings.hook_registry, first,
                                            *bindings.entity_hooks[1]);
                engine::install_vtable_hook(bindings.hook_registry, second,
                                            *bindings.entity_hooks[2]);
            }
        }
    }
    return reinterpret_cast<engine::MemberFunction<std::uint32_t, const char*, void*>>(
        original(receiver)[110])(receiver, name, created);
}
std::uint32_t HarmlessMolotovRuntime::destroy_primary(void* receiver, std::uintptr_t flags) {
    return destroy_particle_primary(bindings.hook_registry, receiver, flags);
}
std::uint32_t HarmlessMolotovRuntime::destroy_secondary(void* receiver, std::uintptr_t flags) {
    return destroy_particle_secondary(bindings.hook_registry, receiver, flags);
}
void HarmlessMolotovRuntime::bind_tables(engine::VtableHookMetadata& creation) {
    publish();
    bind_table(HarmlessMolotovHook::creation, creation);
    bind_table(HarmlessMolotovHook::primary, *bindings.entity_hooks[0]);
    bind_table(HarmlessMolotovHook::secondary, *bindings.entity_hooks[1]);
    bind_table(HarmlessMolotovHook::renderable, *bindings.entity_hooks[2]);
}
void HarmlessMolotovRuntime::bind_table(HarmlessMolotovHook kind,
                                        engine::VtableHookMetadata& metadata) {
    molotov_active_slot = &bindings.active;
    entity_hooks = &bindings.hook_registry;
    bind_static_table(kind, metadata);
}
void HarmlessMolotovRuntime::bind_static_table(HarmlessMolotovHook kind,
                                               engine::VtableHookMetadata& metadata) {
    switch (kind) {
    case HarmlessMolotovHook::creation:
        metadata.replacement_table[110] = reinterpret_cast<void*>(create_entry);
        break;
    case HarmlessMolotovHook::primary:
        metadata.replacement_table[0] = reinterpret_cast<void*>(primary_entry);
        break;
    case HarmlessMolotovHook::secondary:
        metadata.replacement_table[0] = reinterpret_cast<void*>(secondary_entry);
        break;
    case HarmlessMolotovHook::renderable:
        metadata.replacement_table[9] = reinterpret_cast<void*>(molotov_render_entry);
        break;
    }
}
void HarmlessMolotovRuntime::publish() noexcept {
    molotov_active_slot = &bindings.active;
    bindings.active = this;
}
void unbind_harmless_molotov_runtime() noexcept {
    *molotov_active_slot = nullptr;
}
void HarmlessMolotovRuntime::unbind() noexcept {
    unbind_harmless_molotov_runtime();
}
} // namespace skeet::effects

// ============================================================================
// Subcomponent: harmless_molotov_owner.cpp
// ============================================================================

namespace skeet::effects {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define MOLOTOV_MEMBER __thiscall
#define MOLOTOV_FAST __fastcall
#else
#define MOLOTOV_MEMBER
#define MOLOTOV_FAST
#endif
using OwnerEntry = void(MOLOTOV_MEMBER*)(void*);
void MOLOTOV_MEMBER molotov_destroy(void*);
void MOLOTOV_MEMBER molotov_empty(void*) {} //4347D93C/433CCE8E
void MOLOTOV_MEMBER molotov_phase_empty(void*) {}
void MOLOTOV_FAST molotov_player_view_empty(void*, void*, std::uintptr_t, std::uintptr_t, std::uintptr_t) {
} //4337D653
void MOLOTOV_FAST molotov_view_empty(void*, float*, float*, float*) {} //4334A269
void MOLOTOV_FAST molotov_post_view_empty(void*, void*) {}
const std::array<OwnerEntry, 9> molotov_primary_table{molotov_destroy, molotov_empty, molotov_empty, molotov_empty, molotov_empty,
                                              molotov_empty,   molotov_empty, molotov_empty, molotov_empty}; //4346FB44
const network::UpdateObserverVtable molotov_update_table{
    {molotov_phase_empty, molotov_phase_empty, molotov_phase_empty, molotov_phase_empty, molotov_phase_empty},
    molotov_player_view_empty,
    molotov_view_empty,
    molotov_post_view_empty}; //434709A4
struct MolotovOwner {
    const OwnerEntry* vtable = nullptr;
    std::uint32_t untouched_4 = 0;
    network::UpdateObserver updates{nullptr};
    HarmlessMolotovOwnerBindings bindings;
    engine::JobMemory memory;
    HarmlessMolotovState state{false};
    HarmlessMolotovRuntime runtime;
    MolotovOwner(const HarmlessMolotovOwnerRegistries& registry, ConfigRegistries& config,
                 HarmlessMolotovOwnerBindings context)
        : bindings(context), memory(registry.lifecycle.memory),
          runtime{context.settings, state, context.effects} {
        registry.lifecycle.append(this);
        network::register_update_observer(registry.updates, updates);
        vtable = molotov_primary_table.data();
        updates.vtable = &molotov_update_table;
        auto& settings = bindings.settings;
        initialize_boolean_config(config.descriptors, settings.harmless_molotovs, 0xC82CBD3Bu);
        initialize_color_config(config.descriptors, settings.harmless_molotov_color, 0xF9200DEFu,
                                0xFFFF5800u);
        initialize_integer_config(config.descriptors, settings.molotov_percent, 0x921CAA0Au, 65);
        state.initialize_native_storage();
    }
};
void MOLOTOV_MEMBER molotov_destroy(void* object) { //whole433E51E8 ->43326010 ->433837D0
    unbind_harmless_molotov_runtime();
    if (!object)
        return;
    auto* owner = static_cast<MolotovOwner*>(object);
    owner->state.release_native_storage();
    const auto memory = owner->memory;
    owner->~MolotovOwner();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
#undef MOLOTOV_MEMBER
#undef MOLOTOV_FAST
}
std::size_t harmless_molotov_owner_size() noexcept {
    return sizeof(MolotovOwner);
}
void construct_harmless_molotov_owner(void* storage, const HarmlessMolotovOwnerRegistries& registry,
                                      ConfigRegistries& config,
                                      HarmlessMolotovOwnerBindings bindings) {
    auto* owner = ::new (storage) MolotovOwner(registry, config, bindings);
    owner->runtime.publish();
}
} // namespace skeet::effects

// ============================================================================
// Subcomponent: model_entity_relationships.cpp
// ============================================================================

namespace skeet::effects {
namespace {
/* [removed duplicate field] */
std::int32_t convar_integer(engine::NativeConVarReference reference) noexcept {
    const void* object = reference.get();
    for (;;) {
        const auto* parent = field<const void*>(object, 0x1C);
        if (parent == object)
            break;
        object = parent;
    }
    return std::bit_cast<std::int32_t>(field<std::uint32_t>(object, 0x30) ^
                                       std::uint32_t(reinterpret_cast<std::uintptr_t>(object)));
}
struct Live {
    const ModelEntityRelationships& value;
    void* engine() const noexcept {
        return value.engine;
    }
    void* entity_list() const noexcept {
        return value.entity_list;
    }
    void* local_player() const noexcept {
        return value.local_player;
    }
    const ModelRelationshipOffsets& offsets() const noexcept {
        return value.offsets;
    }
    std::int32_t game_type() const noexcept {
        return convar_integer(value.game_type);
    }
    bool teammates_are_enemies() const noexcept {
        return convar_integer(value.teammates_are_enemies) != 0;
    }
};
struct Snapshot {
    const ModelEntityBindings& value;
    void* engine() const noexcept {
        return value.engine;
    }
    void* entity_list() const noexcept {
        return value.entity_list;
    }
    void* local_player() const noexcept {
        return value.local_player;
    }
    ModelRelationshipOffsets offsets() const noexcept {
        return {value.life_state,      value.move_type, value.observer_mode,
                value.observer_target, value.team,      value.survival_team};
    }
    std::int32_t game_type() const noexcept {
        return value.game_type;
    }
    bool teammates_are_enemies() const noexcept {
        return value.teammates_are_enemies;
    }
};
template <class Inputs> bool teammate(void* entity, const Inputs& b) {
    const auto& offsets = b.offsets();
    if (engine::invoke<bool>(b.engine(), 93))
        return field<std::int32_t>(entity, offsets.team) == 3;
    auto* local = b.local_player();
    if (local && field<std::uint8_t>(local, offsets.life_state) != 0 &&
        field<std::uint8_t>(local, offsets.move_type) == 10) {
        const auto mode = field<std::int32_t>(local, offsets.observer_mode);
        const auto handle = field<std::uint32_t>(local, offsets.observer_target);
        if ((mode == 4 || mode == 5) && handle != UINT32_MAX) {
            auto* target = engine::invoke<void*>(b.entity_list(), 4, handle);
            if (target && engine::invoke<bool>(target, 158))
                local = target;
        }
    }
    if (!local)
        return false;
    const auto local_team = field<std::int32_t>(local, offsets.team);
    if (local_team == 1)
        return field<std::int32_t>(entity, offsets.team) == 3;
    bool enemy = false;
    if (entity && local != entity) {
        if (b.game_type() == 6) {
            const auto survival = field<std::int32_t>(local, offsets.survival_team);
            enemy =
                survival == -1 || survival != field<std::int32_t>(entity, offsets.survival_team);
        } else if (local_team == 2 || local_team == 3) {
            const auto team = field<std::int32_t>(entity, offsets.team);
            enemy = (local_team == 2 && team == 3) || (local_team == 3 && team == 2) ||
                    b.teammates_are_enemies();
        }
    }
    return !enemy;
}
}
bool model_teammate(void* entity, const ModelEntityRelationships& bindings) {
    return teammate(entity, Live{bindings});
}
bool model_teammate(void* entity, const ModelEntityBindings& bindings) {
    return teammate(entity, Snapshot{bindings});
}
} //namespace skeet::effects

// ============================================================================
// Subcomponent: model_hooks.cpp
// ============================================================================

namespace skeet::effects {
namespace {
EffectsCommandOwner* const* active_material_owner = nullptr;
/* [removed duplicate field] */
std::int32_t convar_integer(const void* variable) {
    for (;;) {
        const auto* parent = field<const void*>(variable, 0x1C);
        if (parent == variable)
            break;
        variable = parent;
    }
    return std::bit_cast<std::int32_t>(
        field<std::uint32_t>(variable, 0x30) ^
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(variable)));
}
ModelRenderHooks& hooks(void* receiver) {
    auto* table = *static_cast<void***>(receiver);
    auto* descriptor = static_cast<ModelHookDescriptor*>(table[-2]);
    auto* owner = static_cast<ModelRenderRuntime*>(descriptor->owner);
    return *owner->hooks;
}
void* active_material(std::size_t index) {
    if (!active_material_owner)
        throw std::logic_error("Canonical model material owner slot is not bound");
    auto* owner = *active_material_owner;
    if (!owner)
        return nullptr;
    return owner->models.prepare_material(&owner->models.materials[index]);
}
}
void bind_model_material_owner(EffectsCommandOwner* const& active) noexcept {
    active_material_owner = &active;
}
void* player_model_material() {
    return active_material(0);
}
void* hands_model_material() {
    return active_material(5);
}
void* weapon_viewmodel_material() {
    return active_material(6);
}
std::span<const ModelMaterialProviderEntry, 3> model_material_provider_entries() {
    static const std::array<ModelMaterialProviderEntry, 3> entries{
        {{0x43336B64, &player_model_material},
         {0x434019FB, &hands_model_material},
         {0x43417FC3, &weapon_viewmodel_material}}};
    return entries;
}
std::span<const ModelHookReplacement, 5> studio_model_hook_replacements() {
    static const std::array<ModelHookReplacement, 5> entries{
        {{29, 0x433CCB15, reinterpret_cast<void*>(&studio_model_draw_hook)},
         {30, 0x43401A33, reinterpret_cast<void*>(&studio_prop_hook)},
         {47, 0x433B0A83, reinterpret_cast<void*>(&model_prop50_hook)},
         {49, 0x433C930C, reinterpret_cast<void*>(&model_prop48_hook)},
         {48, 0x4337D6DC, reinterpret_cast<void*>(&model_prop60_hook)}}};
    return entries;
}
std::span<const ModelHookReplacement, 1> model_occlusion_hook_replacements() {
    static const std::array<ModelHookReplacement, 1> entries{
        {{16, 0x4336C334, reinterpret_cast<void*>(&model_unoccluded_hook)}}};
    return entries;
}
FakeModelPose capture_fake_model_pose(const ModelFakePoseBindings& bindings) {
    FakeModelPose pose{};
    const auto* record = bindings.record;
    // The original 434685B8 gate is Body yaw, now owned by the source AA config.
    // Reading the old BSS cell here leaves the fake pose permanently disabled.
    if (!record || !record->global.enabled.value || !record->global.body_yaw.value)
        return pose;
    const auto count = record->state.value_1b0;
    if (count == 0)
        return pose;
    if (count > 256 || count > record->finisher.matrices_520.size() ||
        &record->precalculation.common != static_cast<const anti_aim::State*>(&record->state) ||
        &record->finisher.common != &record->state)
        throw std::logic_error(
            "Source fake model pose requires its canonical AA state and bounded matrix storage");
    pose.available = true;
    pose.capture_tick = record->state.value_1b4;
    if (!bindings.current_tick)
        throw std::logic_error("Fake model pose requires its live outgoing-command alias");
    pose.current_tick = *bindings.current_tick;
    pose.blend = bindings.blend;
    pose.observed_yaw = record->state.reference_yaw_258;
    pose.reference_yaw = record->precalculation.feet_yaw_194;
    pose.observed_angles = bindings.observed_angles;
    pose.reference_angles = bindings.reference_angles;
    pose.transform_angles = record->finisher.absolute_angles_1b8;
    pose.rotation_radians = bindings.rotation_radians;
    pose.bones = record->finisher.matrices_520.first(count);
    return pose;
}
ModelRenderHooks::ModelRenderHooks(ModelRenderRuntime& models, WorldRuntime& world,
                                   const ModelRenderHookInputs& inputs, ModelHookDescriptor& studio,
                                   ModelHookDescriptor& occlusion)
    : models_(models), world_(world), inputs_(inputs), studio_(studio), occlusion_(occlusion) {}
ModelRenderHooks::~ModelRenderHooks() {
    if (models_.hooks == this)
        models_.hooks = nullptr;
}
void ModelRenderHooks::bind_replacements() {
    if (!studio_.replacement_table || !occlusion_.replacement_table)
        throw std::logic_error(
            "Actual model hook descriptor tables must be prepared before lifecycle initialization");
    bind_studio_replacements();
    bind_occlusion_replacements();
}
void ModelRenderHooks::bind_studio_replacements() {
    if (!studio_.replacement_table)
        throw std::logic_error("Studio descriptor has not been prepared");
    models_.hooks = this;
    for (const auto& entry : studio_model_hook_replacements())
        studio_.replacement_table[entry.slot] = entry.source_entry;
}
void ModelRenderHooks::bind_occlusion_replacements() {
    if (!occlusion_.replacement_table)
        throw std::logic_error("Occlusion descriptor has not been prepared");
    models_.hooks = this;
    for (const auto& entry : model_occlusion_hook_replacements())
        occlusion_.replacement_table[entry.slot] = entry.source_entry;
}
void ModelRenderHooks::draw(void* studio, const ModelDrawArguments& args) {
    auto entities = inputs_.entities;
    entities.local_player = inputs_.local_player;
    const bool effects =
        world_.suspended == 0 && field<std::int32_t>(studio, 0x260) != 2 && !(args.flags & 0x204);
    bool history = false;
    FakeModelPose fake{};
    if (effects) {
        entities.game_type = convar_integer(inputs_.game_type);
        entities.teammates_are_enemies = convar_integer(inputs_.teammates_are_enemies) != 0;
        if (models_.settings.shadow.value)
            history = convar_integer(inputs_.shadow_history) != 0;
        if (models_.settings.local_fake.enabled.value)
            fake = capture_fake_model_pose(inputs_.fake_pose);
    }
    const ModelDrawFrame frame{inputs_.classification,
                               entities,
                               inputs_.trigonometry,
                               inputs_.rotation_math,
                               inputs_.local_player_flag,
                               inputs_.immunity,
                               inputs_.ragdoll_owner,
                               inputs_.attachment_owner,
                               world_.suspended != 0,
                               inputs_.player_effects != 0,
                               inputs_.player_adjustments,
                               history,
                               fake.available,
                               fake.available ? &fake : nullptr,
                               inputs_.shadows,
                               inputs_.bone_runtime,
                               inputs_.shot_entity_handle,
                               &inputs_.shot,
                               world_.settings.wall_transparency.value};
    models_.draw_model(studio, args, frame);
}
void ModelRenderHooks::unoccluded(void* list, void* view) {
    auto bindings = inputs_.occlusion;
    bindings.player_renderable_vtable = inputs_.classification.renderable_vtables[0];
    bindings.shot_entity_handle = inputs_.shot_entity_handle;
    models_.build_unoccluded_list(list, view, bindings);
}
void ModelRenderHooks::prop50(void* receiver, std::uintptr_t first, std::int32_t count,
                              void* records, std::uintptr_t last) {
    models_.draw_prop_records_50(receiver, first, count, records, last, world_.settings,
                                 world_.suspended != 0);
}
void ModelRenderHooks::prop48(void* receiver, std::uintptr_t first, std::int32_t count,
                              void* records, std::int32_t stride, std::uintptr_t last) {
    models_.draw_prop_records_48(receiver, first, count, records, stride, last, world_.settings,
                                 world_.suspended != 0);
}
void ModelRenderHooks::prop60(void* receiver, std::uintptr_t first, std::int32_t count,
                              ModelPropBatch* batches, std::int32_t stride, std::uint32_t flags) {
    models_.draw_prop_batches_60(receiver, first, count, batches, stride, flags,
                                 inputs_.classification.renderable_vtables[3], world_.settings,
                                 world_.suspended != 0);
}
void ModelRenderHooks::prop_studio(void* receiver, std::uintptr_t first, std::uintptr_t second,
                                   std::uintptr_t third) {
    models_.draw_prop_studio(receiver, first, second, third, world_.settings,
                             world_.suspended != 0);
}
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_MODEL_HOOK_ABI __fastcall
#else
#define SKEET_MODEL_HOOK_ABI
#endif
void SKEET_MODEL_HOOK_ABI studio_model_draw_hook(void* receiver, void*, void* result, void* info,
                                                 void* bones, void* flex, void* delayed,
                                                 void* origin, std::uint32_t flags) {
    hooks(receiver).draw(receiver, {result, info, bones, flex, delayed, origin, flags});
}
void SKEET_MODEL_HOOK_ABI studio_prop_hook(void* receiver, void*, std::uintptr_t a,
                                           std::uintptr_t b, std::uintptr_t c) {
    hooks(receiver).prop_studio(receiver, a, b, c);
}
void SKEET_MODEL_HOOK_ABI model_prop50_hook(void* receiver, void*, std::uintptr_t a, std::int32_t n,
                                            void* p, std::uintptr_t z) {
    hooks(receiver).prop50(receiver, a, n, p, z);
}
void SKEET_MODEL_HOOK_ABI model_prop48_hook(void* receiver, void*, std::uintptr_t a, std::int32_t n,
                                            void* p, std::int32_t stride, std::uintptr_t z) {
    hooks(receiver).prop48(receiver, a, n, p, stride, z);
}
void SKEET_MODEL_HOOK_ABI model_prop60_hook(void* receiver, void*, std::uintptr_t a, std::int32_t n,
                                            ModelPropBatch* p, std::int32_t stride,
                                            std::uint32_t flags) {
    hooks(receiver).prop60(receiver, a, n, p, stride, flags);
}
void SKEET_MODEL_HOOK_ABI model_unoccluded_hook(void* receiver, void*, void* view) {
    hooks(receiver).unoccluded(receiver, view);
}
#undef SKEET_MODEL_HOOK_ABI
} // namespace skeet::effects

// ============================================================================
// Subcomponent: model_render.cpp
// ============================================================================

namespace skeet::effects {
using engine::invoke;
void*& ModelRenderableTables::operator[](std::size_t index) {
    //4/6 are consumed owner captures at4348F0B5..F0FB; the other seven
    //remain live canonical RTTI/descriptor cells, including clone tables.
    static constexpr std::array<std::uint32_t, 9> cells{0x434758ACu, 0x434759A8u, 0x43468908u,
                                                        0x43467190u, 0u,          0x43475B44u,
                                                        0u,          0x4346A444u, 0x43469284u};
    if (data_ && cells.at(index))
        return data_->native_cell<void*>(cells[index]);
    return standalone_.at(index);
}
void* const& ModelRenderableTables::operator[](std::size_t index) const {
    return const_cast<ModelRenderableTables*>(this)->operator[](index);
}
namespace {
constexpr float byte_scale = 0.0039215688593685626983642578125f; // 0x4346EFA0
/* [removed duplicate read] */
// Host pointer size follows the bound engine. This is 0x2C bytes on x86.
struct KeyValues {
    union {
        std::uint32_t symbol;
        std::uint32_t symbol_type;
    };
    char* string_value;
    wchar_t* wide_value;
    std::uint32_t value;
    std::uint8_t type, escape;
    std::uint16_t symbol_high;
    void* system;
    bool case_sensitive;
    KeyValues *peer, *sub, *chain;
    std::uint32_t untouched;
};
void* variable(void* material, std::uint32_t& token) {
    const char empty[] = "";
    return invoke<void*>(material, 47, empty, &token);
}
void integer(void* material, std::uint32_t& token, std::int32_t value) {
    if (auto* v = variable(material, token))
        invoke<void>(v, 5, value);
}
void floating(void* material, std::uint32_t& token, float value) {
    if (auto* v = variable(material, token))
        invoke<void>(v, 4, value);
}
void reset(void* material, std::uint32_t& token) {
    if (auto* v = variable(material, token))
        invoke<void>(v, 19);
}
void vector(void* material, std::uint32_t& token, std::uint32_t color) {
    if (auto* v = variable(material, token))
        for (std::int32_t component = 0; component != 3; ++component)
            invoke<void>(v, 26, float((color >> (component * 8)) & 255) * byte_scale, component);
}
void environment(void* material, std::uint32_t& token, void* texture) {
    if (auto* v = variable(material, token)) {
        if (texture)
            invoke<void>(v, 15, texture);
        else
            invoke<void>(v, 6, "env_cubemap");
    }
}
void override_material(void* studio, void* material) {
    invoke<void>(studio, 33, material, std::int32_t(0), std::int32_t(-1));
}
void original_draw(void* studio, const ModelDrawArguments& args) {
    // The hook retains the original interface at the RTTI-adjacent -8 cell.
    const auto vtable = *static_cast<void***>(studio);
    auto* original = read<void*>(reinterpret_cast<const std::byte*>(vtable) - 8, 0);
    auto** original_vtable = read<void**>(original, 8);
    using Draw =
        engine::MemberFunction<void, void*, void*, void*, void*, void*, void*, std::uint32_t>;
    reinterpret_cast<Draw>(original_vtable[29])(studio, args.results, args.info, args.bones,
                                                args.flex_weights, args.delayed_flex_weights,
                                                args.origin, args.flags);
}
}

ModelMaterialInterfaces::ModelMaterialInterfaces(engine::NativeDataImage& data, void* key_values)
    : material_system(data.native_cell<void*>(0x4346A4E0u)), key_values_system(key_values),
      create_variable(data.native_cell<CreateMaterialVariable>(0x43467A68u)),
      clock(data.native_cell<void*>(0x43467118u)) {}
ModelShotState::ModelShotState(engine::NativeDataImage& data)
    : expires(data.native_cell<double>(0x4347BDE8u)),
      bones_begin(data.native_cell<void*>(0x434797A0u)),
      bones_end(data.native_cell<void*>(0x434797A4u)),
      origin(data.native_cell<std::array<float, 3>>(0x4347BDF0u)) {}
ModelRenderRuntime::ModelRenderRuntime(ModelMaterialInterfaces& i,
                                       const esp::ColoredModelsSettings& s)
    : interfaces(i), settings(s) {
    const esp::ModelColorGroup* groups[] = {&s.player,           &s.teammate, &s.local_player,
                                            &s.local_fake,       &s.on_shot,  &s.hands,
                                            &s.weapon_viewmodel, &s.weapons};
    for (std::size_t n = 0; n != materials.size(); ++n) {
        materials[n].kind = &groups[n]->material;
        materials[n].secondary = &groups[n]->secondary_color;
    }
    materials[4].on_shot = true;
}
void ModelRenderRuntime::initialize_resources() {
    void** textures[] = {&base_texture, &metallic_environment, &glow_environment,
                         &fullscreen_texture};
    for (std::size_t n = 0; n != 4; ++n) {
        if (*textures[n] && interfaces.material_system) {
            invoke<void>(*textures[n], 11);
            *textures[n] = nullptr;
        }
        *textures[n] = invoke<void*>(interfaces.material_system, 91, texture_names[n].c_str(), "",
                                     false, std::int32_t(0));
        if (*textures[n]) {
            texture_names[n].clear();
            invoke<void>(*textures[n], 10);
        }
    }
    void** references[] = {&solid_material, &shadow_material};
    for (std::size_t n = 0; n != 2; ++n) {
        if (*references[n] && interfaces.material_system) {
            invoke<void>(*references[n], 13);
            *references[n] = nullptr;
        }
        const auto lock = invoke<std::uint32_t>(interfaces.material_system, 118);
        auto handle = invoke<std::uint16_t>(interfaces.material_system, 86);
        void* found = nullptr;
        while (handle != invoke<std::uint16_t>(interfaces.material_system, 88)) {
            auto* material = invoke<void*>(interfaces.material_system, 89, handle);
            if (material && !invoke<bool>(material, 42)) {
                const auto* name = invoke<const char*>(material, 0);
                std::uint32_t hash = 0x811C9DC5u;
                if (name)
                    for (; *name; ++name)
                        hash = (hash ^ std::uint8_t(*name)) * 0x1000193u;
                if ((hash ^ 0x92CA5DF1u) == material_name_hashes[n]) {
                    found = material;
                    break;
                }
            }
            handle = invoke<std::uint16_t>(interfaces.material_system, 87, handle);
        }
        invoke<void>(interfaces.material_system, 119, lock);
        *references[n] = found;
        if (found) {
            material_name_hashes[n] = 0;
            invoke<void>(found, 12);
        }
    }
}
void* ModelRenderRuntime::create_material() { // 0x4336BB4B
    auto* storage = invoke<void*>(interfaces.key_values_system, 2, std::int32_t(0x2C));
    KeyValues* values = nullptr;
    if (storage) {
        values = static_cast<KeyValues*>(storage);
        values->symbol_type = 0;
        values->string_value = nullptr;
        values->wide_value = nullptr;
        values->value = 0;
        values->type = 0;
        values->escape = 0;
        values->symbol_high = 0;
        values->case_sensitive = false;
        values->peer = nullptr;
        values->sub = nullptr;
        values->chain = nullptr;
        values->system = interfaces.key_values_system;
        std::uint32_t insensitive = UINT32_MAX;
        const auto sensitive =
            invoke<std::uint32_t>(values->system, 10, &insensitive, "VertexLitGeneric", true);
        values->symbol_type = (insensitive & 0xFFFFFFu) | ((sensitive & 255u) << 24);
        values->symbol_high = std::uint16_t(sensitive >> 8);
    }
    char name[64]{};
    std::snprintf(name, sizeof name, "m%llu", static_cast<unsigned long long>(__rdtsc()));
    return invoke<void*>(interfaces.material_system, 83, static_cast<const char*>(name), values);
}
void ModelRenderRuntime::resolve_variable(void* material, const char* name, std::uint32_t& token) {
    if (invoke<void*>(material, 47, name, &token) || !interfaces.create_variable)
        return;
    if (auto* v = interfaces.create_variable(material, name)) {
        invoke<void>(v, 19);
        invoke<void>(v, 5, std::int32_t(1));
        invoke<void>(material, 84, v);
        token = 0;
        invoke<void*>(material, 47, name, &token);
    }
}
void* ModelRenderRuntime::prepare_material(ModelMaterial* cache) { // 0x433C6694
    if (!cache)
        return nullptr;
    const auto kind = cache->kind->value;
    if (kind == 1)
        return solid_material;
    if (kind == 6)
        return nullptr;
    if (!cache->material)
        cache->material = create_material();
    auto* result = cache->material;
    if (!result)
        return solid_material; // explicit native 0x433C7753 branch
    if ((kind == 0 && cache->applied_kind == 0) ||
        (kind != 0 && cache->applied_kind == kind &&
         cache->applied_color == cache->secondary->value))
        return result;

    const auto lock = invoke<std::uint32_t>(interfaces.material_system, 118);
    auto* real = invoke<void*>(result, 99);
    if (cache->material) {
        invoke<void>(invoke<void*>(cache->material, 99), 37);
        auto* variable_material = invoke<void*>(cache->material, 99);
        if (cache->material && invoke<void*>(variable_material, 69)) {
            constexpr const char* names[] = {"$allowdiffusemodulation",
                                             "$envmap",
                                             "$envmaptint",
                                             "$envmapfresnel",
                                             "$envmapcontrast",
                                             "$basemapalphaphongmask",
                                             "$phong",
                                             "$phongboost",
                                             "$phongexponent",
                                             "$phongtint",
                                             "$phongfresnelranges"};
            constexpr std::size_t order[] = {0, 1, 3, 2, 4, 5, 6, 7, 8, 9, 10};
            for (auto n : order)
                resolve_variable(variable_material, names[n], cache->variable_tokens[n]);
        }
    }
    if (invoke<void*>(real, 69)) {
        cache->applied_kind = kind;
        if (kind == 2 || kind == 3 || kind == 4 || kind == 5) {
            auto** parameters = invoke<void**>(real, 41);
            if (parameters && invoke<std::int32_t>(real, 40) > 6 && parameters[6] && base_texture)
                invoke<void>(parameters[6], 15, base_texture);
            auto& t = cache->variable_tokens;
            integer(real, t[0], 1);
            invoke<void>(real, 29, std::int32_t(0x80), kind == 5);
            invoke<void>(real, 29, std::int32_t(0x100000), true);
            const auto secondary = cache->secondary->value;
            if (kind == 2 || kind == 3) {
                integer(real, t[5], 1);
                integer(real, t[6], 1);
                if (kind == 3) {
                    reset(real, t[10]);
                    reset(real, t[9]);
                    floating(real, t[7], 0.0f);
                    environment(real, t[1], metallic_environment);
                    floating(real, t[4], 0.0f);
                    vector(real, t[2], secondary);
                    floating(real, t[3], 1.0f - float(secondary >> 24) * byte_scale);
                } else {
                    floating(real, t[7], 1.0f);
                    integer(real, t[8], std::int32_t(std::max(1u, 255u - (secondary >> 24))));
                    vector(real, t[9], secondary);
                    if (auto* v = variable(real, t[10])) {
                        invoke<void>(v, 26, 1.0f, std::int32_t(0));
                        invoke<void>(v, 26, 0.0f, std::int32_t(1));
                        invoke<void>(v, 26, 0.0f, std::int32_t(2));
                    }
                }
            } else {
                integer(real, t[6], 0);
                environment(real, t[1], kind == 4 ? glow_environment : nullptr);
                floating(real, t[4], kind == 4 ? 0.95f : 0.0f);
                floating(real, t[3], 1.0f);
                vector(real, t[2], secondary);
            }
            cache->applied_color = secondary;
            invoke<void>(real, 53);
        }
    }
    invoke<void>(interfaces.material_system, 119, lock);
    return result;
}
void ModelRenderRuntime::invalidate_material(void* material) {
    auto* real = invoke<void*>(material, 99);
    for (auto& cache : materials)
        if (cache.material && invoke<void*>(cache.material, 99) == real) {
            cache.applied_kind = -1;
            cache.applied_color = UINT32_MAX;
        }
}
void ModelRenderRuntime::modulate(void* studio, std::uint32_t color) {
    const float rgb[] = {float(color & 255) * byte_scale, float((color >> 8) & 255) * byte_scale,
                         float((color >> 16) & 255) * byte_scale, 1.0f};
    invoke<void>(studio, 27, rgb);
}
void ModelRenderRuntime::limit_alpha(void* studio, std::uint32_t color) {
    const auto alpha = float(color >> 24) * byte_scale;
    if (alpha < read<float>(studio, 0x24C))
        invoke<void>(studio, 28, alpha);
}
ModelDrawState ModelRenderRuntime::save_state(void* studio) {
    ModelDrawState result;
    std::memcpy(result.bytes.data(), static_cast<std::byte*>(studio) + 0x240, 0x38);
    return result;
}
void ModelRenderRuntime::restore_state(void* studio, const ModelDrawState& state) {
    std::memcpy(static_cast<std::byte*>(studio) + 0x240, state.bytes.data(), 0x38);
}
RenderedModel ModelRenderRuntime::classify(const void* info, const ModelClassification& b) {
    auto* renderable = read<void*>(info, 0x18);
    if (!renderable)
        return RenderedModel::other;
    auto* table = *static_cast<void**>(renderable);
    const auto& t = b.renderable_vtables;
    if (table == t[0] || table == t[1])
        return RenderedModel::player;
    if (table == t[2])
        return RenderedModel::player_attachment;
    if (table == t[3])
        return RenderedModel::ragdoll;
    if (table == t[4] || table == t[5])
        return RenderedModel::viewmodel;
    const auto name = [&]() -> const char* {
        auto* model = read<const std::byte*>(info, 0);
        if (!model)
            return nullptr;
        const auto relative = read<std::int32_t>(model, 0x190);
        if (relative) {
            const auto second = read<std::int32_t>(model + relative, 0x14);
            if (second)
                return reinterpret_cast<const char*>(model + relative + second);
        }
        return reinterpret_cast<const char*>(model + 0xC);
    };
    if (table == t[6]) {
        if (auto* unknown = invoke<void*>(renderable, 0)) {
            if (auto* entity = invoke<void*>(unknown, 7))
                if (read<std::uint32_t>(entity, b.entity_flags) & 0x810)
                    return RenderedModel::flagged_viewmodel;
        }
        if (const auto* s = name(); s && std::strlen(s) > 23 && s[0] == 'w' && s[8] == 'v' &&
                                    s[9] == '_' && s[10] == 'm' && s[17] == 'a' && s[21] == '/')
            return RenderedModel::hands;
    } else {
        if (table == t[7]) {
            if (auto* unknown = invoke<void*>(renderable, 0)) {
                if (auto* entity = invoke<void*>(unknown, 7); entity && invoke<bool>(entity, 159)) {
                    const auto handle = read<std::uint32_t>(entity, b.owner_handle);
                    if (handle == UINT32_MAX || !invoke<void*>(b.entity_list, 4, handle))
                        return RenderedModel::unowned_weapon;
                    return RenderedModel::owned_weapon;
                }
            }
        }
        if (table == t[8]) {
            if (const auto* s = name();
                s && std::strlen(s) >= 18 && s[0] == 'w' && s[8] == 'w' && s[9] == '_')
                return RenderedModel::world_weapon_model;
        }
    }
    return RenderedModel::other;
}
PlayerModelColors player_model_colors(const esp::ColoredModelsSettings& settings, bool local,
                                      bool teammate, bool history, bool ragdoll) noexcept {
    PlayerModelColors p{};
    if (local) {
        p.flags = std::uint8_t(1 | ((settings.local_player.enabled.value & 1) << 2));
        p.visible_color = settings.local_player.primary_color.value;
    } else {
        const auto& group = teammate ? settings.teammate : settings.player;
        p.flags =
            std::uint8_t(((group.enabled.value << 2) & 4) | ((group.behind_wall.value & 1) << 3));
        if (!teammate && settings.shadow.value && history)
            p.flags |= 16;
        p.visible_color = group.primary_color.value;
        p.occluded_color = group.behind_wall_color.value;
    }
    if (ragdoll)
        p.flags = std::uint8_t((p.flags & ~24u) | 1u);
    return p;
}
ModelPassSettings ModelRenderRuntime::player_pass(bool local, bool teammate, bool history,
                                                  bool ragdoll) noexcept {
    const auto colors = player_model_colors(settings, local, teammate, history, ragdoll);
    return {colors.flags, colors.visible_color, colors.occluded_color,
            &materials[local      ? 2
                       : teammate ? 1
                                  : 0]};
}
void ModelRenderRuntime::apply_single_pass(void* studio, std::size_t group) {
    const esp::ModelColorGroup* groups[] = {
        &settings.player,  &settings.teammate, &settings.local_player,     &settings.local_fake,
        &settings.on_shot, &settings.hands,    &settings.weapon_viewmodel, &settings.weapons};
    if (auto* material = prepare_material(&materials.at(group)))
        override_material(studio, material);
    modulate(studio, groups[group]->primary_color.value);
    limit_alpha(studio, groups[group]->primary_color.value);
}
bool ModelRenderRuntime::draw_viewmodel(void* studio, const ModelDrawArguments& args,
                                        RenderedModel category) {
    if (category != RenderedModel::hands && category != RenderedModel::viewmodel)
        return false;
    const auto saved = save_state(studio);
    if (category == RenderedModel::hands) {
        if (settings.hands.enabled.value)
            apply_single_pass(studio, 5);
    } else if (settings.weapon_viewmodel.enabled.value) {
        const auto* model = read<const std::byte*>(args.info, 0);
        std::uint32_t hash = 0;
        if (model) {
            const auto relative = read<std::int32_t>(model, 0x190);
            const auto second = relative ? read<std::int32_t>(model + relative, 0x14) : 0;
            const auto* name = reinterpret_cast<const char*>(
                relative && second ? model + relative + second : model + 12);
            hash = 0x811C9DC5u;
            for (; *name; ++name)
                hash = (hash ^ std::uint8_t(*name)) * 0x1000193u;
            hash ^= 0x92CA5DF1u;
        }
        auto* material = prepare_material(&materials[6]);
        if (material) {
            if (hash == 0xF8DBA852u || hash == 0x9F053037u) {
                invoke<void>(studio, 33, material, std::int32_t(3), std::int32_t(0));
                invoke<void>(studio, 33, material, std::int32_t(3), std::int32_t(1));
                invoke<void>(studio, 33, hash == 0xF8DBA852u ? material : nullptr, std::int32_t(3),
                             std::int32_t(2));
                invoke<void>(studio, 33, static_cast<void*>(nullptr), std::int32_t(3),
                             std::int32_t(3));
            } else
                override_material(studio, material);
        }
        modulate(studio, settings.weapon_viewmodel.primary_color.value);
        limit_alpha(studio, settings.weapon_viewmodel.primary_color.value);
    }
    original_draw(studio, args);
    restore_state(studio, saved);
    return true;
}
bool ModelRenderRuntime::teammate(void* entity, const ModelEntityBindings& b) {
    return model_teammate(entity, b);
}
bool ModelRenderRuntime::draw_weapon(void* studio, const ModelDrawArguments& args,
                                     RenderedModel category, const ModelEntityBindings& b) {
    if (category != RenderedModel::owned_weapon && category != RenderedModel::world_weapon_model)
        return false;
    const auto saved = save_state(studio);
    auto* renderable = read<std::byte*>(args.info, 0x18);
    auto* entity = renderable ? renderable - 4 : nullptr;
    while (entity) {
        if (invoke<bool>(entity, 158))
            break;
        const auto handle = read<std::uint32_t>(entity, b.move_parent);
        if (handle == UINT32_MAX) {
            entity = nullptr;
            break;
        }
        entity = static_cast<std::byte*>(invoke<void*>(b.entity_list, 4, handle));
    }
    if (entity && entity == b.local_player) {
        if (settings.weapon_viewmodel.enabled.value)
            apply_single_pass(studio, 6);
    } else if (settings.weapons.enabled.value) {
        if (!entity || !teammate(entity, b) || settings.teammate.enabled.value)
            apply_single_pass(studio, 7);
    }
    original_draw(studio, args);
    restore_state(studio, saved);
    return true;
}
void ModelRenderRuntime::draw_on_shot(void* studio, const ModelDrawState& saved,
                                      const ModelDrawArguments& args, const ModelShotState& shot) {
    if (!settings.on_shot.enabled.value)
        return;
    const auto duration = double(settings.on_shot_duration.value) * 0.1;
    const auto remaining = shot.expires - invoke<double>(interfaces.clock, 5);
    if (remaining <= 0.0 || remaining > duration || shot.bones_begin == shot.bones_end)
        return;
    auto* material = prepare_material(&materials[4]);
    if (material)
        invoke<void>(material, 29, std::int32_t(0x8000), true);
    const auto color = settings.on_shot.primary_color.value;
    modulate(studio, color);
    auto alpha = float(color >> 24) * byte_scale;
    if (remaining < 0.25)
        alpha = std::min(float(remaining * 4.0), alpha);
    if (alpha < read<float>(studio, 0x24C))
        invoke<void>(studio, 28, alpha);
    if (material)
        override_material(studio, material);
    auto draw_args = args;
    draw_args.bones = shot.bones_begin;
    draw_args.origin = const_cast<float*>(shot.origin.data());
    draw_args.flags |= 0x824;
    original_draw(studio, draw_args);
    restore_state(studio, saved);
}
void ModelRenderRuntime::draw_local_fake(void* studio, const ModelDrawState& saved,
                                         const ModelDrawArguments& args, const FakeModelPose& pose,
                                         const RageTrigonometry& trig) {
    if (!settings.local_fake.enabled.value)
        return;
    std::array<Matrix3x4, 256> matrices;
    const auto origin = read<RageVector>(args.origin, 0);
    if (!build_fake_model_matrices(pose, origin, matrices, trig))
        return;
    auto* material = prepare_material(&materials[3]);
    const auto color = settings.local_fake.primary_color.value;
    modulate(studio, color);
    const auto alpha = float(color >> 24) * byte_scale;
    if (alpha < read<float>(studio, 0x24C) ||
        (color >> 24) > (settings.local_player.primary_color.value >> 24))
        invoke<void>(studio, 28, alpha);
    if (material)
        override_material(studio, material);
    const auto field = read<std::uint32_t>(args.info, 0x10);
    const std::uint32_t zero = 0;
    std::memcpy(static_cast<std::byte*>(args.info) + 0x10, &zero, 4);
    auto draw_args = args;
    draw_args.bones = matrices.data();
    draw_args.flags |= 0x824;
    original_draw(studio, draw_args);
    restore_state(studio, saved);
    std::memcpy(static_cast<std::byte*>(args.info) + 0x10, &field, 4);
}
bool ModelRenderRuntime::draw_shadow(void* studio, const ModelDrawArguments& args,
                                     const ModelShadowPose& pose, ModelShadowBones& bones) {
    lagcomp::AnimationState saved;
    lagcomp::capture_animation(saved, bones.layers, bones.layer_count, bones.pose_parameters);
    lagcomp::restore_animation(pose.animation_record->animation, bones.layers, bones.layer_count,
                               bones.pose_parameters);
    std::array<Matrix3x4, animation::maximum_bones> matrices;
    const animation::BoneRequest request{pose.angles, pose.origin, pose.time, 0xFFF00, -1, 0, true};
    const auto built = animation::build_bone_matrices(bones.state, bones.model, request, matrices,
                                                      bones.engine, bones.trigonometry);
    lagcomp::restore_animation(saved, bones.layers, bones.layer_count, bones.pose_parameters);
    if (!built)
        return false;
    draw_shadow_matrices(studio, args, pose, matrices);
    return true;
}
bool ModelRenderRuntime::draw_shadow(void* studio, const ModelDrawArguments& args,
                                     const ModelShadowPose& pose, void* player,
                                     const animation::BoneRuntime& runtime) {
    const auto layer_offset = runtime.fields.animation_layers_b538bba5;
    auto* layers = read<void*>(player, layer_offset);
    const auto count = read<std::int32_t>(player, layer_offset + 0xC);
    auto poses = runtime.pose_parameters(player);
    lagcomp::AnimationState saved;
    lagcomp::capture_animation(saved, layers, count, poses.data());
    lagcomp::restore_animation(pose.animation_record->animation, layers, count, poses.data());
    std::array<Matrix3x4, animation::maximum_bones> matrices;
    const animation::BoneRequest request{pose.angles, pose.origin, pose.time, 0xFFF00, -1, 0, true};
    const auto built = runtime.build_bone_matrices(player, request, matrices);
    auto* restored_layers = read<void*>(player, layer_offset);
    const auto restored_count = read<std::int32_t>(player, layer_offset + 0xC);
    lagcomp::restore_animation(saved, restored_layers, restored_count,
                               runtime.pose_parameters(player).data());
    if (!built)
        return false;
    draw_shadow_matrices(studio, args, pose, matrices);
    return true;
}
void ModelRenderRuntime::draw_shadow_matrices(void* studio, const ModelDrawArguments& args,
                                              const ModelShadowPose& pose,
                                              std::span<Matrix3x4> matrices) {
    const auto alpha = read<float>(studio, 0x24C);
    modulate(studio, settings.shadow_color.value);
    limit_alpha(studio, settings.shadow_color.value);
    if (solid_material) {
        invoke<void>(solid_material, 29, std::int32_t(0x8000), settings.player.behind_wall.value);
        override_material(studio, solid_material);
    }
    auto draw_args = args;
    draw_args.bones = matrices.data();
    draw_args.origin = const_cast<float*>(pose.origin.data());
    draw_args.flags |= 0x824;
    original_draw(studio, draw_args);
    override_material(studio, nullptr);
    invoke<void>(studio, 28, alpha);
}
void ModelRenderRuntime::apply_player_passes(void* studio, const ModelDrawState& saved,
                                             const ModelDrawArguments& args,
                                             const ModelPassSettings& pass, std::int32_t opacity) {
    const auto original_alpha = read<float>(studio, 0x24C);
    if (!(pass.flags & 1)) {
        if (pass.flags & 8) {
            modulate(studio, pass.occluded_color);
            if (auto* material = prepare_material(pass.material)) {
                invoke<void>(material, 29, std::int32_t(0x8000), true);
                invoke<void>(material, 29, std::int32_t(0x4000), true);
                override_material(studio, material);
            }
            const auto alpha = opacity == 100 ? float(pass.occluded_color >> 24) * byte_scale
                                              : 1.0f - float(opacity) * 0.01f;
            if (alpha < original_alpha ||
                original_alpha == float(pass.visible_color >> 24) * byte_scale)
                invoke<void>(studio, 28, alpha);
            original_draw(studio, args);
            override_material(studio, nullptr);
        } else if (opacity != 100) {
            const auto alpha = 1.0f - float(opacity) * 0.01f;
            if (alpha < original_alpha)
                invoke<void>(studio, 28, alpha);
            // studiohdr/material list: info+4 -> +8, skin index info+0x1C.
            auto* header = read<void*>(args.info, 4);
            if (header) {
                auto* skin_materials = read<void*>(header, 8);
                if (skin_materials) {
                    auto* skin = static_cast<std::byte*>(skin_materials) +
                                 read<std::uint32_t>(args.info, 0x1C) * 0x20;
                    const auto count = read<std::int32_t>(skin, 8);
                    auto** list = read<void**>(skin, 12);
                    for (std::int32_t n = 0; n < count; ++n) {
                        invoke<void>(list[n], 29, std::int32_t(0x8000), true);
                        invoke<void>(list[n], 29, std::int32_t(0x4000), true);
                    }
                    original_draw(studio, args);
                    override_material(studio, nullptr);
                    for (std::int32_t n = 0; n < count; ++n) {
                        invoke<void>(list[n], 29, std::int32_t(0x8000), false);
                        invoke<void>(list[n], 29, std::int32_t(0x4000), false);
                    }
                }
            }
        }
    }
    restore_state(studio, saved);
    if (pass.flags & 4) {
        modulate(studio, pass.visible_color);
        if (auto* material = prepare_material(pass.material)) {
            invoke<void>(material, 29, std::int32_t(0x8000), false);
            invoke<void>(material, 29, std::int32_t(0x4000), false);
            override_material(studio, material);
        }
        const auto alpha = float(pass.visible_color >> 24) * byte_scale;
        if (alpha < original_alpha)
            invoke<void>(studio, 28, alpha);
    }
}
void ModelRenderRuntime::apply_gated_player(void* studio, const ModelDrawState& saved,
                                            const ModelDrawArguments& args, void* entity,
                                            const ModelPassSettings& pass,
                                            const ModelDrawFrame& frame) {
    if (!frame.player_effects)
        return;
    const auto index = invoke<std::uint32_t>(static_cast<std::byte*>(entity) + 8, 10);
    if (index - 1u < 64u && frame.player_adjustments.records[index].adjustments.disable_visuals)
        return;
    if (!(pass.flags & 2) && (pass.flags & 16) &&
        read<std::uint8_t>(entity, frame.entities.life_state) == 0) {
        if (!index)
            throw std::logic_error("Native shadow draw requires a nonzero entity index");
        ModelShadowPose pose;
        if (frame.shadows.select(index, pose, frame.rotation_math))
            draw_shadow(studio, args, pose, entity, frame.bone_runtime);
    }
    apply_player_passes(studio, saved, args, pass, frame.wall_transparency);
}
void ModelRenderRuntime::draw_model(void* studio, const ModelDrawArguments& args,
                                    const ModelDrawFrame& frame) {
    const auto saved = save_state(studio);
    if (!frame.suppress_effects && read<std::int32_t>(studio, 0x260) != 2 &&
        !(args.flags & 0x204)) {
        const auto category = classify(args.info, frame.classification);
        if (draw_viewmodel(studio, args, category) ||
            draw_weapon(studio, args, category, frame.entities))
            return;
        auto* renderable = read<std::byte*>(args.info, 0x18);
        auto* entity = renderable ? renderable - 4 : nullptr;
        const auto packet = [&](void* player, bool ragdoll = false) {
            const auto local = read<std::uint8_t>(player, frame.local_player_flag) != 0;
            return player_pass(local, !local && teammate(player, frame.entities),
                               frame.history_available, ragdoll);
        };
        if (category == RenderedModel::player) {
            if (entity && invoke<bool>(entity, 158) &&
                read<std::uint8_t>(entity, frame.entities.life_state) == 0 &&
                invoke<std::int32_t>(entity, 122) > 0 && !invoke<bool>(entity + 8, 9) &&
                read<std::uint8_t>(entity, frame.immunity) == 0) {
                if (settings.local_fake.enabled.value && frame.fake_controller &&
                    read<std::uint8_t>(entity, frame.local_player_flag) != 0 && frame.fake_pose)
                    draw_local_fake(studio, saved, args, *frame.fake_pose, frame.trigonometry);
                const auto pass = packet(entity);
                if (pass.flags & 0x1c)
                    apply_gated_player(studio, saved, args, entity, pass, frame);
            }
        } else if (category == RenderedModel::ragdoll) {
            if (entity) {
                const auto handle = read<std::uint32_t>(entity, frame.ragdoll_owner);
                if (handle != UINT32_MAX) {
                    auto* owner = invoke<void*>(frame.entities.entity_list, 4, handle);
                    if (owner && settings.ragdolls.value) {
                        const auto pass = packet(owner, true);
                        if (pass.flags & 4)
                            apply_gated_player(studio, saved, args, owner, pass, frame);
                    }
                }
            }
        } else if (category == RenderedModel::player_attachment) {
            if (entity) {
                const auto handle = read<std::uint32_t>(entity, frame.attachment_owner);
                if (handle != UINT32_MAX) {
                    auto* owner = invoke<void*>(frame.entities.entity_list, 4, handle);
                    if (owner && invoke<bool>(owner, 158)) {
                        auto pass = packet(owner);
                        pass.flags = std::uint8_t((pass.flags & ~8u) | 2u);
                        if ((pass.flags & 4) && frame.player_effects) {
                            const auto index =
                                invoke<std::uint32_t>(static_cast<std::byte*>(owner) + 8, 10);
                            if (index - 1u < 64u && !frame.player_adjustments.records[index]
                                                         .adjustments.disable_visuals) {
                                modulate(studio, pass.visible_color);
                                if (auto* material = prepare_material(pass.material)) {
                                    invoke<void>(material, 29, std::int32_t(0x8000), false);
                                    invoke<void>(material, 29, std::int32_t(0x4000), false);
                                    override_material(studio, material);
                                }
                                limit_alpha(studio, pass.visible_color);
                            }
                        }
                    }
                }
            }
        } else if (frame.shot_entity_handle != UINT32_MAX) {
            auto* marker = invoke<void*>(frame.entities.entity_list, 4, frame.shot_entity_handle);
            if (marker && renderable == invoke<void*>(marker, 5)) {
                if (frame.shot)
                    draw_on_shot(studio, saved, args, *frame.shot);
                restore_state(studio, saved);
                return;
            }
        }
    }
    original_draw(studio, args);
    restore_state(studio, saved);
}
void ModelRenderRuntime::update_local_transparency(void* player, void* renderable,
                                                   const LocalTransparencyInterfaces& i,
                                                   const LocalTransparencyNetvars& n,
                                                   bool suppressed, bool player_effects,
                                                   float frame_time) {
    local_alpha_overridden = false;
    if (suppressed)
        return;
    if (!player)
        throw std::logic_error("Native local transparency requires a valid player");
    if (read<std::uint8_t>(player, n.field_af649dac) != 0 &&
        read<std::uint8_t>(player, n.field_cacaeecc - 12) != 0) {
        local_alpha = 1.0f;
        return;
    }
    std::uint32_t alpha = 255;
    if (settings.local_player.enabled.value && player_effects)
        alpha = settings.local_player.primary_color.value >> 24;
    const auto conditions = std::uint32_t(settings.local_transparency.value);
    bool translucent = false;
    if (conditions & 1) {
        const auto handle = read<std::uint32_t>(player, n.active_weapon);
        std::int32_t type = 11;
        if (handle != UINT32_MAX)
            if (auto* weapon = invoke<void*>(i.entity_list, 4, handle))
                type = invoke<std::int32_t>(weapon, 455);
        translucent = type == 9;
    }
    if (conditions & 2)
        translucent = translucent || read<std::uint8_t>(player, n.field_cacaeecc) != 0 ||
                      read<std::uint8_t>(player, n.field_c07d1150) != 0;
    if (translucent && alpha > 100)
        alpha = 100;
    auto target = float(alpha) / 255.0f;
    target = _mm_cvtss_f32(
        _mm_min_ss(_mm_max_ss(_mm_set_ss(target), _mm_setzero_ps()), _mm_set_ss(1.0f)));
    if (!invoke<void*>(i.input, 32, std::int32_t(-1))) {
        local_alpha = target;
        return;
    }
    const auto difference = target - local_alpha, step = frame_time * 5.0f;
    if (difference > step)
        local_alpha += step;
    else if (-step > difference)
        local_alpha -= step;
    else
        local_alpha = target;
    const auto rounded = local_alpha * 255.0f + 8388608.0f;
    const auto byte = std::uint8_t(std::bit_cast<std::uint32_t>(rounded));
    invoke<void>(player, 396, byte == 255 ? std::int32_t(0) : std::int32_t(4), true);
    invoke<void>(player, 395, std::uint32_t(byte));
    const auto handle = *invoke<const std::int16_t*>(renderable, 7);
    if (handle != -1)
        invoke<void>(i.model_cache, 3, std::uint32_t(std::uint16_t(handle)), byte != 255);
    local_alpha_overridden = byte != 255;
}
void ModelRenderRuntime::build_unoccluded_list(void* list, void* view,
                                               const ModelOcclusionInterfaces& i) {
    const auto vtable = *static_cast<void***>(list);
    auto* descriptor = read<void*>(reinterpret_cast<const std::byte*>(vtable) - 8, 0);
    auto** original = read<void**>(descriptor, 8);
    reinterpret_cast<engine::MemberFunction<void, void*>>(original[16])(list, view);
    if (!settings.disable_model_occlusion.value ||
        (!settings.player.behind_wall.value && !settings.teammate.behind_wall.value) ||
        read<std::int32_t>(view, 0x2c) != 0)
        return;
    auto* storage = read<ModelOcclusionStorage*>(view, 4);
    if (!storage)
        return;
    void* marker = nullptr;
    if (i.shot_entity_handle != UINT32_MAX)
        if (auto* entity = invoke<void*>(i.entity_list, 4, i.shot_entity_handle))
            marker = invoke<void*>(entity, 5);
    auto index = read<std::uint32_t>(list, 0x30);
    while (index != 0xffff) {
        auto* items = read<ModelOcclusionItem*>(list, 0x24);
        auto& item = items[index];
        if (item.renderable &&
            (*static_cast<void**>(item.renderable) == i.player_renderable_vtable ||
             (marker && item.renderable == marker))) {
            bool other = (item.flags & 0xc0) != 0x40;
            if (other && item.metadata && (read<std::uint16_t>(item.metadata, 10) & 0x1e0) &&
                read<std::uint8_t>(item.metadata, 14) != 255)
                other = false;
            const auto bucket = std::uint32_t(other) ^ 1;
            auto& count = storage->counts[bucket];
            auto& records = storage->buckets[bucket];
            bool duplicate = false;
            for (std::int32_t n = 0; n < count; ++n)
                if (records[std::size_t(n)].renderable == item.renderable) {
                    duplicate = true;
                    break;
                }
            if (!duplicate && count < 4096) {
                auto& record = records[std::size_t(count)];
                record.field_04 = 0;
                record.field_06 = 255;
                record.renderable = item.renderable;
                record.flags = std::uint8_t(((item.flags >> 8) & 0x3f) |
                                            ((item.flags & 0xc0) == 0x80 ? 0x40 : 0));
                record.model_type = 0;
                if (auto* model = invoke<void*>(item.renderable, 8)) {
                    const auto handle = invoke<std::uint16_t>(i.model_info, 44, model);
                    if (handle != 0xffff)
                        record.model_type =
                            invoke<std::uint8_t>(i.model_type_registry, 70, std::uint32_t(handle));
                }
                ++count;
            }
        }
        index = read<ModelOcclusionItem*>(list, 0x24)[index].next;
    }
}
} // namespace skeet::effects

// ============================================================================
// Subcomponent: model_render_fake.cpp
// ============================================================================

namespace skeet::effects {
namespace {
float normalize(float angle) {
    if (std::isnan(angle))
        return 0.0f;
    if (std::fabs(angle) > 180.0f) {
        const auto quotient = std::fabs(angle / 360.0f);
        auto turns = float(std::ceil(double(quotient)));
        if (turns - quotient > 0.5f)
            turns -= 1.0f;
        const auto correction = turns * 360.0f;
        angle = angle < 0.0f ? angle + correction : angle - correction;
    }
    return angle;
}
/* [removed duplicate negate] */
Matrix3x4 rotation(float sy, float cy, float sp, float cp, float sr, float cr) {
    return {{{cp * cy, (sr * cy) * sp - cr * sy, (cr * cy) * sp + sr * sy, 0.0f},
             {cp * sy, (sr * sy) * sp + cr * cy, (cr * sy) * sp - sr * cy, 0.0f},
             {negate(sp), sr * cp, cr * cp, 0.0f}}};
}
Matrix3x4 concatenate(const Matrix3x4& left, const Matrix3x4& right) {
    Matrix3x4 result;
    for (unsigned r = 0; r < 3; ++r)
        for (unsigned col = 0; col < 4; ++col)
            result.rows[r][col] =
                ((left.rows[r][2] * right.rows[2][col] + left.rows[r][1] * right.rows[1][col]) +
                 left.rows[r][0] * right.rows[0][col]) +
                (col == 3 ? left.rows[r][3] : 0.0f);
    return result;
}
}
bool build_fake_model_matrices(const FakeModelPose& pose, const RageVector& origin,
                               std::span<Matrix3x4> output, const RageTrigonometry& trig) {
    const auto delta = std::bit_cast<std::int32_t>(std::uint32_t(pose.capture_tick) -
                                                   std::uint32_t(pose.current_tick));
    const auto sign = delta < 0 ? UINT32_MAX : 0u;
    const auto absolute = std::bit_cast<std::int32_t>((std::uint32_t(delta) ^ sign) - sign);
    if (!pose.available || pose.bones.empty() || absolute > 16)
        return false;
    if (pose.blend <= 0.0f) {
        auto yaw = pose.observed_yaw - pose.reference_yaw;
        // Unlike the vector normalizer, this caller retains a NaN yaw.
        if (!std::isnan(yaw))
            yaw = normalize(yaw);
        if (!(std::fabs(yaw) >= 5.0f)) {
            RageVector angles;
            for (unsigned n = 0; n < 3; ++n)
                angles[n] = normalize(pose.observed_angles[n] - pose.reference_angles[n]);
            if ((angles[0] * angles[0] + angles[1] * angles[1]) + angles[2] * angles[2] < 25.0f)
                return false;
        }
    }
    if (!(pose.blend > 0.0f)) {
        for (std::size_t n = 0; n < pose.bones.size(); ++n) {
            output[n] = pose.bones[n];
            for (unsigned r = 0; r < 3; ++r)
                output[n].rows[r][3] = output[n].rows[r][3] + origin[r];
        }
        return true;
    }
    constexpr float rad = 0.01745329238474369f;
    const auto yaw = pose.transform_angles[1] * rad, pitch = pose.transform_angles[0] * rad,
               roll = pose.transform_angles[2] * rad;
    const auto sy = trig.sine(yaw), cy = trig.cosine(yaw), sp = trig.sine(pitch),
               cp = trig.cosine(pitch), sr = trig.sine(roll), cr = trig.cosine(roll);
    const auto forward = rotation(sy, cy, sp, cp, sr, cr);
    Matrix3x4 inverse;
    for (unsigned r = 0; r < 3; ++r) {
        for (unsigned col = 0; col < 3; ++col)
            inverse.rows[r][col] = forward.rows[col][r];
        inverse.rows[r][3] = negate((inverse.rows[r][0] * 0.0f + inverse.rows[r][1] * 0.0f) +
                                    inverse.rows[r][2] * 0.0f);
    }
    const auto sy2 = trig.sine(pose.rotation_radians), cy2 = trig.cosine(pose.rotation_radians);
    const auto sp2 = trig.sine(0.0f), cp2 = trig.cosine(0.0f), sr2 = trig.sine(0.0f),
               cr2 = trig.cosine(0.0f);
    auto rotated = rotation(sy2, cy2, sp2, cp2, sr2, cr2);
    rotated.rows[0][3] = origin[0];
    rotated.rows[1][3] = origin[1];
    rotated.rows[2][3] = origin[2];
    const auto transform = concatenate(rotated, inverse);
    for (std::size_t n = 0; n < pose.bones.size(); ++n)
        output[n] = concatenate(transform, pose.bones[n]);
    return true;
}
} // namespace skeet::effects

// ============================================================================
// Subcomponent: model_render_history.cpp
// ============================================================================

namespace skeet::effects {
namespace {
float distance_square(const RageVector& a, const RageVector& b) {
    const auto y = a[1] - b[1], x = a[0] - b[0], z = a[2] - b[2];
    return (y * y + x * x) + z * z;
}
float maxss(float a, float b) {
    return _mm_cvtss_f32(_mm_max_ss(_mm_set_ss(a), _mm_set_ss(b)));
}
float minss(float a, float b) {
    return _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(a), _mm_set_ss(b)));
}
/* [removed duplicate add] */
std::uint32_t record_count(const lagcomp::LagTimeline& timeline) {
    return timeline.record_count;
}
std::uint32_t record_count(const lagcomp::RecordRing& ring) {
    return ring.count;
}
template <class Timeline, class Timing>
bool select_pose(const Timeline& timeline, Timing timing, ModelShadowPose& output,
                 const lagcomp::RotationMath& math) {
    const auto count = record_count(timeline);
    if (count <= 1)
        return false;
    if (!(distance_square(timeline.at(0).header.origin, timeline.at(count - 1).header.origin) >
          25.0f))
        return false;
    const auto t = timing();
    float now = t.current_time;
    if (t.local_alive && t.prediction_enabled)
        now = ((float(t.tick_base) * t.interval - t.interval) + t.prediction_counter * t.interval) -
              t.prediction_correction;
    const auto interpolation =
        maximum_number(t.interpolation, t.interpolation_ratio / t.update_rate);
    auto requested =
        interpolation + float(timeline.at(count - 1).header.simulation_tick) * t.interval;
    requested = minss(maxss(requested, now - 0.18437500298023224f), now - (-0.18437500298023224f));
    const auto latency = lagcomp::mean_history_timing_ticks(
        {std::uint32_t(t.latency_sample_head), t.latency_sample_count, t.latency_samples});
    const auto corrected =
        lagcomp::corrected_command_tick(t.command_tick, t.previous_command, t.current_command);
    const auto seconds = _mm_cvttss_si32(_mm_set_ss(float(add(latency, corrected)) * t.interval -
                                                    float(t.maximum_unlag_ms) * 0.001f));
    requested = maxss(float(seconds), requested);
    const auto tick = _mm_cvttss_si32(_mm_set_ss(requested / t.interval + 0.5f));
    const lagcomp::LagRecord* previous = nullptr;
    const lagcomp::LagRecord* selected = nullptr;
    for (std::uint32_t n = 0; n < count; ++n) {
        selected = &timeline.at(n);
        if (previous && distance_square(previous->header.origin, selected->header.origin) > 4096.0f)
            return false;
        if (selected->header.simulation_tick <= tick || n + 1 == count)
            break;
        previous = selected;
    }
    ModelShadowPose pose{
        selected, requested, selected->header.origin, {0.0f, selected->header.animation_yaw, 0.0f}};
    if (previous && selected->header.simulation_tick < tick &&
        selected->header.simulation_tick < previous->header.simulation_tick) {
        const auto old_time = float(selected->header.simulation_tick) * t.interval;
        const auto fraction = (requested - old_time) /
                              (float(previous->header.simulation_tick) * t.interval - old_time);
        for (unsigned n = 0; n < 3; ++n)
            pose.origin[n] = (previous->header.origin[n] - selected->header.origin[n]) * fraction +
                             selected->header.origin[n];
        if (previous->header.animation_yaw != selected->header.animation_yaw) {
            const auto first =
                lagcomp::history_angle_quaternion({0, selected->header.animation_yaw, 0}, math);
            const auto second =
                lagcomp::history_angle_quaternion({0, previous->header.animation_yaw, 0}, math);
            pose.angles = lagcomp::history_quaternion_angles(
                lagcomp::slerp_history_quaternion(
                    first, lagcomp::align_history_quaternion(first, second), fraction, math),
                math);
        }
    }
    output = pose;
    return true;
}
/* [removed duplicate field] */
float convar_float(engine::NativeConVarReference reference) {
    const auto* variable = reference.get();
    for (;;) {
        const auto* parent = field<const void*>(variable, 0x1C);
        if (parent == variable)
            break;
        variable = parent;
    }
    return std::bit_cast<float>(field<std::uint32_t>(variable, 0x2C) ^
                                std::uint32_t(reinterpret_cast<std::uintptr_t>(variable)));
}
}
bool select_model_shadow_pose(const lagcomp::LagTimeline& timeline, const ModelShadowTiming& timing,
                              ModelShadowPose& output, const lagcomp::RotationMath& math) {
    return select_pose(timeline, [&] { return timing; }, output, math);
}
ModelShadowTimingSource::ModelShadowTimingSource(engine::NativeDataImage& data,
                                                 const engine::NativeOffsetGlobals& offsets)
    : data_(data), offsets_(offsets) {}
ModelShadowTiming ModelShadowTimingSource::capture() const {
    ModelShadowTiming timing{};
    const auto variable = [&](std::uint32_t cell, std::uint32_t key) {
        return engine::NativeConVarReference(data_.native_cell<std::uint32_t>(cell), key);
    };
    auto* local = data_.native_cell<const void*>(0x43475808u);
    if (local)
        timing.local_alive =
            field<std::uint8_t>(local, engine::DecodedOffset(offsets_, 0x43468E78u, 0x434684E4u,
                                                             0x9D920E35u, 0)) == 0;
    //43407DE2 uses the float value: unordered (NaN) is enabled, exactly like !=0.
    if (timing.local_alive)
        timing.prediction_enabled = convar_float(variable(0x4346A850u, 0x478720C2u)) != 0.0f;
    timing.interval = *data_.native_cell<const float*>(0x4346A338u);
    if (timing.local_alive && timing.prediction_enabled) {
        timing.tick_base = field<std::int32_t>(
            local, engine::DecodedOffset(offsets_, 0x434675C8u, 0x4346A5B8u, 0x3EDECE61u, 0));
        timing.prediction_counter = field<float>(data_.native_cell<void*>(0x43467B04u), 0x24);
        timing.prediction_correction = data_.native_cell<float>(0x43476928u);
    } else
        timing.current_time = *data_.native_cell<const float*>(0x43467EB8u);
    timing.update_rate = convar_float(variable(0x4346747Cu, 0x03B4819Cu));
    timing.interpolation = convar_float(variable(0x4346A748u, 0x7D9DC86Fu));
    timing.interpolation_ratio = convar_float(variable(0x4346A330u, 0xBE64D223u));
    const auto& samples = data_.native_cell<lagcomp::TickTimingSamples>(0x43478C70u);
    timing.latency_sample_head = std::bit_cast<std::int32_t>(samples.first);
    timing.latency_sample_count = samples.count;
    timing.latency_samples = samples.values;
    timing.command_tick = field<std::int32_t>(data_.native_cell<void*>(0x43475A60u), 4);
    timing.previous_command = data_.native_cell<const std::int32_t*>(0x43468F9Cu);
    timing.current_command = data_.native_cell<const std::int32_t*>(0x434670A8u);
    timing.maximum_unlag_ms = data_.native_cell<std::int32_t>(0x43468230u);
    return timing;
}
ModelShadowSource::ModelShadowSource(client::PlayerStateCache& cache,
                                     const ModelShadowTimingSource& timing) noexcept
    : cache_(&cache), live_(&timing) {}
ModelShadowSource::ModelShadowSource(const std::array<const lagcomp::LagTimeline*, 64>& timelines,
                                     const ModelShadowTiming& timing) noexcept
    : timelines_(&timelines), timing_(&timing) {}
bool ModelShadowSource::select(std::uint32_t index, ModelShadowPose& pose,
                               const lagcomp::RotationMath& math) const {
    if (cache_) {
        auto* state = cache_->get(index);
        // Native433B8A2B's failed allocation is followed by an unchecked load.
        // Source propagation keeps that unresolved OOM edge explicit.
        if (!state)
            throw std::bad_alloc();
        return select_pose(state->history, [&] { return live_->capture(); }, pose, math);
    }
    const auto* timeline = (*timelines_)[index & 63u];
    return timeline && select_model_shadow_pose(*timeline, *timing_, pose, math);
}
} // namespace skeet::effects

// ============================================================================
// Subcomponent: model_render_lifecycle.cpp
// ============================================================================

namespace skeet::effects {
namespace {
void restore_hook(ModelHookDescriptor& hook) {
    if (hook.replacement_table && hook.object)
        std::atomic_ref<void**>(*hook.object)
            .exchange(hook.original_table, std::memory_order_seq_cst);
}
void release_material(void*& material) {
    if (material) {
        engine::invoke<void>(material, 13);
        material = nullptr;
    }
}
void release_texture(void*& texture) {
    if (texture) {
        engine::invoke<void>(texture, 11);
        texture = nullptr;
    }
}
}
void ModelRenderLifecycle::initialize_bindings() {
    bindings.camera.observer_proxy = bindings.initial_observer_proxy;
    if (bindings.first_object) {
        bindings.first_hook.object = static_cast<void***>(bindings.first_object);
        bindings.first_hook.owner = &models;
    }
    if (bindings.second_object) {
        bindings.second_hook.object = static_cast<void***>(bindings.second_object);
        bindings.second_hook.owner = &models;
    }
}
void ModelRenderLifecycle::reset() {
    state.field_450 = 0;
    world.material_flags = 0;
    world.previous_prop_transparency = 100;
    for (auto& material : models.materials) {
        material.applied_color = UINT32_MAX;
        material.applied_kind = -1;
    }
    bindings.shot_entity_handle = UINT32_MAX;
}
void ModelRenderLifecycle::reset_thunk() {
    reset();
}
void ModelRenderLifecycle::clear_shot_marker() {
    bindings.shot_entity_handle = UINT32_MAX;
}
void update_shot_entity_bones(void* entity_list, std::uint32_t handle,
                              ShotEntityBoneOffsets offsets, const std::uint32_t* current_counter) {
    if (handle == UINT32_MAX)
        return;
    if (void* entity = engine::invoke<void*>(entity_list, 4, handle))
        update_shot_entity_bones_for_entity(entity, offsets, current_counter);
}
void update_shot_entity_bones_for_entity(void* entity, ShotEntityBoneOffsets offsets,
                                         const std::uint32_t* current_counter) {
    const std::uint32_t mask = 0xFFF00;
    auto* bytes = static_cast<std::byte*>(entity);
    std::memcpy(bytes + offsets.bone_accessor + 8, &mask, 4);
    std::memcpy(bytes + offsets.bone_accessor + 12, &mask, 4);
    if (current_counter)
        std::memcpy(bytes + offsets.recent_counter, current_counter, 4);
}
void ModelRenderLifecycle::shutdown() {
    world.post_processing.restore();
    restore_hook(bindings.first_hook);
    restore_hook(bindings.second_hook);
    auto& effects = world.settings;
    effects.brightness.value = 0;
    effects.wall_transparency.value = 100;
    colors.props.value = 0;
    effects.prop_transparency.value = 100;
    effects.third_person_alive.value = 0;
    colors.player.enabled.value = 0;
    colors.player.behind_wall.value = 0;
    colors.hands.enabled.value = 0;
    colors.teammate.enabled.value = 0;
    colors.teammate.behind_wall.value = 0;
    colors.local_player.enabled.value = 0;
    colors.shadow.value = 0;
    colors.on_shot.enabled.value = 0;
    colors.weapon_viewmodel.enabled.value = 0;
    effects.disable_ragdolls.value = 0;
    if (engine::invoke<bool>(bindings.camera.engine, 26)) {
        if (engine::invoke<void*>(bindings.camera.input, 32, std::int32_t(-1))) {
            engine::invoke<void>(bindings.camera.input, 36);
            if (bindings.camera.local_player)
                engine::invoke<void>(bindings.camera.local_player, 312, false);
        }
        world.update();
    }
    for (auto& material : models.materials)
        release_material(material.material);
    for (void** reference : std::array<void**, 2>{&models.solid_material, &models.shadow_material})
        if (models.interfaces.material_system)
            release_material(*reference);
    for (void** reference :
         std::array<void**, 4>{&models.base_texture, &models.metallic_environment,
                               &models.glow_environment, &models.fullscreen_texture})
        if (models.interfaces.material_system)
            release_texture(*reference);
    if (bindings.shot_entity_handle != UINT32_MAX) {
        if (void* marker = engine::invoke<void*>(bindings.camera.entity_list, 4,
                                                 bindings.shot_entity_handle)) {
            bindings.shot_entity_handle = UINT32_MAX;
            engine::invoke<void>(static_cast<std::byte*>(marker) + 8, 1);
        }
    }
    if (state.allocation_760) {
        engine::invoke<void>(bindings.allocator, 5, state.allocation_760);
        state.allocation_760 = nullptr;
    }
}
} // namespace skeet::effects

// ============================================================================
// Subcomponent: model_render_props.cpp
// ============================================================================

namespace skeet::effects {
namespace {
/* [dedup byte_scale] */
constexpr std::array<float, 4> night_color{0.35f, 0.34f, 0.36f, 1.0f}; //4346DBF0
/* [removed duplicate read] */
template <class... Args> void original(void* object, std::size_t slot, Args... args) {
    auto** table = *static_cast<void***>(object);
    const auto* descriptor = read<void*>(reinterpret_cast<const std::byte*>(table) - 8, 0);
    auto** methods = read<void**>(descriptor, 8);
    reinterpret_cast<engine::MemberFunction<void, Args...>>(methods[slot])(object, args...);
}
std::array<float, 4> prop_color(std::uint32_t packed) {
    return {float(packed & 255) * byte_scale, float((packed >> 8) & 255) * byte_scale,
            float((packed >> 16) & 255) * byte_scale, 1.0f};
}
void multiply(void* record, const std::array<float, 4>& color) {
    auto* bytes = static_cast<std::byte*>(record);
    for (std::size_t channel = 0; channel < 3; ++channel) {
        const auto offset = 0x38 + channel * 4;
        const float result = read<float>(record, offset) * color[channel];
        std::memcpy(bytes + offset, &result, sizeof result);
    }
}
}
void ModelRenderRuntime::draw_prop_records_50(void* renderer, std::uintptr_t argument,
                                              std::int32_t count, void* records,
                                              std::uintptr_t last,
                                              const esp::EffectsSettings& effects,
                                              bool suppressed) {
    if (!suppressed && records && (settings.props.value || effects.brightness.value == 2)) {
        const auto color =
            settings.props.value ? prop_color(settings.props_color.value) : night_color;
        auto* record = static_cast<std::byte*>(records);
        for (std::int32_t n = 0; n < count; ++n, record += 0x50)
            multiply(record, color);
    }
    original(renderer, 47, argument, count, records, last);
}
void ModelRenderRuntime::draw_prop_records_48(void* renderer, std::uintptr_t argument,
                                              std::int32_t count, void* records,
                                              std::int32_t stride, std::uintptr_t last,
                                              const esp::EffectsSettings& effects,
                                              bool suppressed) {
    if (!suppressed && records && stride == 0x48) {
        if (settings.props.value || effects.brightness.value == 2) {
            const auto color =
                settings.props.value ? prop_color(settings.props_color.value) : night_color;
            auto* record = static_cast<std::byte*>(records);
            for (std::int32_t n = 0; n < count; ++n, record += 0x48)
                multiply(record, color);
        }
        if (effects.wall_transparency.value != 100) {
            const float alpha = float(effects.wall_transparency.value) * 0.01f;
            auto* record = static_cast<std::byte*>(records);
            for (std::int32_t n = 0; n < count; ++n, record += 0x48)
                std::memcpy(record + 0x44, &alpha, sizeof alpha);
        }
    }
    original(renderer, 49, argument, count, records, stride, last);
}
void ModelRenderRuntime::draw_prop_batches_60(void* renderer, std::uintptr_t argument,
                                              std::int32_t count, ModelPropBatch* batches,
                                              std::int32_t stride, std::uint32_t flags,
                                              void* ragdoll_renderable_vtable,
                                              const esp::EffectsSettings& effects,
                                              bool suppressed) {
    if (!(flags & 0x200) && !suppressed && batches && stride == 0x60 &&
        (settings.props.value || effects.brightness.value == 2)) {
        const auto color =
            settings.props.value ? prop_color(settings.props_color.value) : night_color;
        for (std::int32_t batch = 0; batch < count; ++batch) {
            auto* record = static_cast<std::byte*>(batches[batch].vertices);
            if (!record)
                continue;
            for (std::int32_t n = 0; n < batches[batch].count; ++n, record += 0x60) {
                if (!settings.props.value) {
                    void* renderable = read<void*>(record, 0x48);
                    if (renderable && read<void*>(renderable, 0) == ragdoll_renderable_vtable)
                        continue;
                }
                multiply(record, color);
            }
        }
    }
    original(renderer, 48, argument, count, batches, stride, flags);
}
void ModelRenderRuntime::draw_prop_studio(void* studio_render, std::uintptr_t first,
                                          std::uintptr_t second, std::uintptr_t third,
                                          const esp::EffectsSettings& effects, bool suppressed) {
    const auto saved = save_state(studio_render);
    if (!suppressed && (settings.props.value || effects.brightness.value == 2)) {
        const auto color =
            settings.props.value ? prop_color(settings.props_color.value) : night_color;
        engine::invoke<void>(studio_render, 27, color.data());
    }
    original(studio_render, 30, first, second, third);
    restore_state(studio_render, saved);
}
} // namespace skeet::effects

// ============================================================================
// Subcomponent: owner_lifecycle.cpp
// ============================================================================

namespace skeet::effects {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define EFFECT_MEMBER __thiscall
#define EFFECT_FAST __fastcall
#else
#define EFFECT_MEMBER
#define EFFECT_FAST
#endif
using OwnerEntry = void(EFFECT_MEMBER*)(void*);
struct EffectsInstance;
struct HudEndpoint : hud::HudFrameCallback {
    EffectsInstance* owner;
};
struct FrameEndpoint : engine::FrameUpdateEndpoint {
    EffectsInstance* owner;
};

void EFFECT_MEMBER destroy(void*);
void EFFECT_MEMBER effect_unchanged(void*) {} //433CCE8E
void EFFECT_MEMBER resources(void*);
void EFFECT_MEMBER initialize(void*);
void EFFECT_MEMBER shutdown(void*);
void EFFECT_MEMBER reset(void*);
void EFFECT_MEMBER reset_forwarder(void*);
void EFFECT_MEMBER clear_marker(void*);
void EFFECT_FAST hud_entry(void*, ui::RenderQueue*, void*, std::int32_t*);
void EFFECT_FAST frame_entry(void*, std::uint8_t);
const std::array<OwnerEntry, 9> effects_owner_table{destroy,    effect_unchanged,       resources,
                                            initialize, shutdown,        reset,
                                            effect_unchanged,  reset_forwarder, clear_marker};
const std::array<hud::HudFrameEntry, 1> effects_hud_table{hud_entry};
const std::array<engine::FrameUpdateEntry, 1> effects_frame_table{frame_entry};

struct EffectsInstance {
    const OwnerEntry* vtable = nullptr;
    client::NativeCommandPhaseReceiver command_receiver;
    HudEndpoint hud{{nullptr}, this};
    client::NativeEntityObserverReceiver entity_receiver;
    WorldUpdateObserver updates;
    FrameEndpoint frame{{nullptr}, this};
    EffectsOwnerBindings bindings;
    engine::JobMemory memory;
    events::ServerImpactBindings impact_bindings;
    events::ServerImpactRuntime impacts;
    std::uint8_t impact_listener_enabled = 0; //native full+74C
    events::EventListener impact_listener;

    static void impact(void* object, events::GameEventReader& event,
                       events::EventPlayerIndices& players) {
        events::process_server_impact(static_cast<EffectsInstance*>(object)->impacts, event,
                                      players);
    }
    EffectsInstance(const EffectsOwnerRegistries& registry, ConfigRegistries& config,
                    EffectsOwnerBindings context)
        : updates(context.updates), bindings(context), memory(registry.lifecycle.memory),
          impact_bindings{context.command.bindings.debug_overlay, context.command.world.suspended},
          impacts{context.command.world.settings, impact_bindings},
          impact_listener{events::bullet_impact_hash, this, impact} {
        bind_model_material_owner(bindings.active);
        //4348E6FA..4348E72A. Every endpoint joins its actual list before the
        // native table-publication block; no list is sorted or reconstructed.
        registry.lifecycle.append(this);
        registry.commands.append(command_receiver, &bindings.command); //4348E702,+8
        hud::register_hud_frame(registry.hud, &hud);
        registry.entities.append_effects(entity_receiver, bindings.command); //4348E712,+18
        network::register_update_observer(registry.updates, updates.endpoint);
        engine::register_frame_update(registry.frames, &frame);
        registry.empty_ticks.append(bindings.command); //4348E72A
        updates.initialize_vtable(); //4348E733
        frame.vtable = effects_frame_table.data(); //4348E73F
        vtable = effects_owner_table.data(); //4348E74B
        registry.commands.publish(command_receiver,
                                  client::CommandOwnerId::owner_434700e4); //4348E751
        hud.vtable = effects_hud_table.data(); //4348E758
        registry.entities.publish_effects(entity_receiver); //4348E75F
        registry.empty_ticks.publish(bindings.command); //4348E766

        auto& command = bindings.command;
        auto& world = command.world;
        auto& models = command.models;
        auto& callbacks = bindings.updates;
        callbacks.state.second_hook_installed = 0;
        models.local_alpha = 1.0f;
        models.local_alpha_overridden = false;
        initialize_model_effects_owner_settings(config, world.settings, bindings.lifecycle.colors);
        bindings.lifecycle.state.field_450 = 0;
        callbacks.state.camera_pending = 0;
        world.material_flags = 0;
        models.solid_material = models.shadow_material = nullptr;
        models.material_name_hashes = {0xF4A7A78Au, 0x27DB215Fu};
        models.base_texture = models.metallic_environment = models.glow_environment =
            models.fullscreen_texture = nullptr;
        models.texture_names = {"vgui/white", "editor/cube_vertigo_blurred_hdr",
                                "models/effects/cube_white", "_rt_Fullscreen"};
        world.post_processing.saved = 0;
        world.post_processing.active = false;
        world.cascaded_shadows.saved = 0;
        world.cascaded_shadows.active = false;
        for (auto& variable : world.sky) {
            variable.saved = 0;
            variable.active = false;
        }
        world.previous_wall_alpha = 1.0f;
        world.previous_prop_transparency = world.previous_prop_transparency_2 = 100;
        world.previous_night_rgb = 0;
        callbacks.state.sky_name = "sky_csgo_night02";
        callbacks.state.saved_sky_name = nullptr;
        bindings.lifecycle.state.allocation_760 = nullptr;
        //4348F055..F0AF are native owner captures of the then-current genuine
        //ConVars. Keep the global encoded cells authoritative until this cut.
        auto& data = bindings.data;
        const auto variable = [&](std::uint32_t cell, std::uint32_t key) {
            return engine::NativeConVarReference(data.native_cell<std::uint32_t>(cell), key).get();
        };
        world.post_processing.variable = variable(0x43468F44u, 0x684EE759u);
        world.cascaded_shadows.variable = variable(0x43468F98u, 0xADB120F0u);
        world.sky[0].variable = variable(0x4346A37Cu, 0x38DF1D5Eu);
        world.sky[1].variable = variable(0x4346925Cu, 0x89E7CF90u);
        world.sky[2].variable = variable(0x434693A8u, 0x27F664A6u);
        world.sky[3].variable = variable(0x43468A94u, 0x24CE2F36u);
        auto& classification = bindings.classification;
        classification.renderable_vtables[6] = data.native_cell<void*>(0x4346A81Cu); //4348F0B5..BA
        auto* attachment = data.native_cell<void*>(0x43468D28u); //4348F0C0
        data.native_cell<void*>(0x4346A81Cu) = nullptr; //4348F0C5, consumed only here
        classification.renderable_vtables[4] = attachment; //4348F0FB
    }
    void set_impact_listener(std::uint8_t value) {
        if (value == impact_listener_enabled)
            return;
        impact_listener_enabled = value;
        if (value)
            bindings.event_registry.add(impact_listener);
        else
            bindings.event_registry.remove(impact_listener);
    }
    void release_storage() { //whole433D6BCF, in its original reverse order
        std::string().swap(bindings.updates.state.sky_name);
        set_impact_listener(0);
        auto& models = bindings.command.models;
        std::array<void**, 4> textures{&models.base_texture, &models.metallic_environment,
                                       &models.glow_environment, &models.fullscreen_texture};
        for (std::size_t i = textures.size(); i != 0; --i) {
            auto& texture = *textures[i - 1];
            if (texture && models.interfaces.material_system) {
                engine::invoke<void>(texture, 11);
                texture = nullptr;
            }
            std::string().swap(models.texture_names[i - 1]);
        }
        for (auto* reference :
             std::array<void**, 2>{&models.shadow_material, &models.solid_material})
            if (*reference && models.interfaces.material_system) {
                engine::invoke<void>(*reference, 13);
                *reference = nullptr;
            }
    }
};
void EFFECT_MEMBER destroy(void* object) { //433279E3 ->433D6BCF ->433837D0
    auto* owner = static_cast<EffectsInstance*>(object);
    owner->bindings.active = nullptr;
    owner->release_storage();
    const auto memory = owner->memory;
    owner->~EffectsInstance();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
void EFFECT_MEMBER resources(void* object) {
    static_cast<EffectsInstance*>(object)->bindings.command.models.initialize_resources();
}
void EFFECT_MEMBER initialize(void* object) {
    auto& bindings = static_cast<EffectsInstance*>(object)->bindings;
    bindings.hooks.bind_replacements();
    bindings.lifecycle.initialize_bindings();
}
void EFFECT_MEMBER shutdown(void* object) {
    static_cast<EffectsInstance*>(object)->bindings.lifecycle.shutdown();
}
void EFFECT_MEMBER reset(void* object) {
    static_cast<EffectsInstance*>(object)->bindings.lifecycle.reset();
}
void EFFECT_MEMBER reset_forwarder(void* object) {
    static_cast<EffectsInstance*>(object)->vtable[5](object);
}
void EFFECT_MEMBER clear_marker(void* object) {
    static_cast<EffectsInstance*>(object)->bindings.lifecycle.clear_shot_marker();
}
void EFFECT_FAST hud_entry(void* object, ui::RenderQueue*, void*, std::int32_t*) {
    static_cast<HudEndpoint*>(static_cast<hud::HudFrameCallback*>(object))
        ->owner->bindings.command.world.update();
}
void EFFECT_FAST frame_entry(void* object, std::uint8_t refresh) { //433C8C2A
    if (!refresh)
        return;
    auto* owner =
        static_cast<FrameEndpoint*>(static_cast<engine::FrameUpdateEndpoint*>(object))->owner;
    owner->set_impact_listener(owner->bindings.command.world.settings.bullet_impacts.value);
}
#undef EFFECT_MEMBER
#undef EFFECT_FAST
}
std::size_t model_effects_owner_size() noexcept {
    return sizeof(EffectsInstance);
}
void construct_model_effects_owner(void* storage, const EffectsOwnerRegistries& registry,
                                   ConfigRegistries& config, EffectsOwnerBindings bindings) {
    ::new (storage) EffectsInstance(registry, config, bindings);
    bindings.active = &bindings.command;
}
} // namespace skeet::effects

// ============================================================================
// Subcomponent: owner_settings.cpp
// ============================================================================

namespace skeet::effects {
namespace {
void group(ConfigLink& registry, esp::ModelColorGroup& c, std::uint32_t enabled,
           std::uint32_t color_hash, std::uint32_t color, std::uint32_t material_hash,
           std::int32_t material, std::uint32_t secondary_hash, std::uint32_t secondary) {
    initialize_boolean_config(registry, c.enabled, enabled);
    initialize_color_config(registry, c.primary_color, color_hash, color);
    initialize_integer_config(registry, c.material, material_hash, material);
    initialize_color_config(registry, c.secondary_color, secondary_hash, secondary);
}
void occluded(ConfigLink& registry, esp::OccludedModelColorGroup& c, std::uint32_t enabled,
              std::uint32_t color_hash, std::uint32_t color, std::uint32_t behind,
              std::uint32_t behind_color_hash, std::uint32_t behind_color,
              std::uint32_t material_hash, std::int32_t material, std::uint32_t secondary_hash,
              std::uint32_t secondary) {
    initialize_boolean_config(registry, c.enabled, enabled);
    initialize_color_config(registry, c.primary_color, color_hash, color);
    initialize_boolean_config(registry, c.behind_wall, behind);
    initialize_color_config(registry, c.behind_wall_color, behind_color_hash, behind_color);
    initialize_integer_config(registry, c.material, material_hash, material);
    initialize_color_config(registry, c.secondary_color, secondary_hash, secondary);
}
}
void initialize_model_effects_owner_settings(ConfigRegistries& registries, esp::EffectsSettings& e,
                                             esp::ColoredModelsSettings& m) noexcept {
    auto& r = registries.descriptors;
    initialize_integer_config(r, e.impact_duration, 0x4C044F71u, 4);
    initialize_integer_config(r, e.wall_transparency, 0x9B4090D9u, 100);
    initialize_integer_config(r, e.prop_transparency, 0x8BED781Eu, 100);
    initialize_boolean_config(r, m.props, 0xE217C264u);
    initialize_color_config(r, m.props_color, 0x84830790u, 0xFFFFFFFFu);
    //4348E7CA registers this integer without overwriting its stored value.
    initialize_integer_config(r, e.brightness, 0x2E5DD918u, e.brightness.value);
    initialize_color_config(r, e.night_color, 0x2A0B0DF3u, 0xFF2E292Bu);
    initialize_boolean_config(r, e.remove_fog, 0x3359E29Cu);
    initialize_boolean_config(r, e.disable_post_processing, 0xA8321370u);
    occluded(r, m.player, 0xC51E0D3Du, 0xFE06341Du, 0xFF3CC896u, 0xF6B67DE5u, 0xD4699245u,
             0xB2B4783Cu, 0x044F7095u, 0, 0x9FF240E4u, 0xFF00B2FFu);
    occluded(r, m.teammate, 0x1570AF7Cu, 0x2CEFC91Cu, 0xFF323232u, 0x57BF65ECu, 0x04AD8938u,
             0xB2FFFFFFu, 0xE85380FAu, 4, 0xF3D8F0D1u, 0xFF3B2D09u);
    group(r, m.local_player, 0x6670DB4Du, 0x9201E7EDu, 0xFF323232u, 0x3B28B0E5u, 4, 0x9436ECB4u,
          0xFF3B2D09u);
    group(r, m.local_fake, 0x96DB2E1Cu, 0xA98CEAE8u, 0x78000000u, 0x3FC208C1u, 1, 0xF50A4300u,
          0xFF00B2FFu);
    initialize_integer_config(r, m.local_transparency, 0x3EA36C3Au, 0);
    group(r, m.on_shot, 0x5E0E2558u, 0xC2DBF0A4u, 0xFF323232u, 0x17C5B91Au, 5, 0x8B122392u,
          0xFFFFFFFFu);
    initialize_integer_config(r, m.on_shot_duration, 0x1A07B48Fu, 20);
    group(r, m.hands, 0xE8407B64u, 0x5A93F490u, 0xFFFFFFFFu, 0xB7360BA6u, 5, 0x2926F5DEu,
          0xFFA49E54u);
    group(r, m.weapon_viewmodel, 0xDA411D58u, 0x75A868A4u, 0xFF6E1032u, 0x2A1FA11Au, 3, 0x1C678B92u,
          0xFFA49E54u);
    group(r, m.weapons, 0x92EBAD93u, 0x83BE3FA7u, 0xFF6E1032u, 0x118A8443u, 3, 0x6D66C717u,
          0xFFA49E54u);
    initialize_boolean_config(r, m.shadow, 0x67F8885Cu);
    initialize_color_config(r, m.shadow_color, 0xC2140C28u, 0x78000000u);
    initialize_boolean_config(r, e.instant_scope, 0xEAFCEAD9u);
    initialize_boolean_config(r, e.bullet_impacts, 0xD54A1CBDu);
    initialize_boolean_config(r, e.remove_sky, 0xFBD4768Du);
}
} // namespace skeet::effects

// ============================================================================
// Subcomponent: player_auxiliary_hooks.cpp
// ============================================================================

namespace skeet::effects {
namespace {
std::uint32_t offset(client::ClientRuntimeStorage& storage, std::uint32_t salt) {
    const auto& globals = storage.offsets().globals;
    // Resolve the exact entity publications before salt lookup; the origin
    // salt is also used by an unrelated temporary bullet event.
    struct Derived {
        std::uint32_t pointer, key, salt;
    };
    constexpr std::array derived{Derived{0x43468D30, 0x43468ED4, 0x2A671B42},
                                 Derived{0x43468368, 0x43467FAC, 0x3B1F6ECC},
                                 Derived{0x43468BF8, 0x43467B8C, 0xD070684C}};
    for (const auto& recipe : derived)
        if (recipe.salt == salt)
            return std::uint32_t(std::ptrdiff_t(
                engine::DecodedOffset(globals, recipe.pointer, recipe.key, salt, 0)));
    for (const auto& recipe : engine::build_receive_offset_recipes())
        if (recipe.initial_xor_key == salt)
            return std::uint32_t(std::ptrdiff_t(
                engine::DecodedOffset(globals, recipe.pointer_identity, recipe.xor_key_identity,
                                      recipe.initial_xor_key, 0)));
    for (const auto& recipe : engine::build_data_map_offset_recipes())
        if (recipe.initial_xor_key == salt)
            return std::uint32_t(std::ptrdiff_t(
                engine::DecodedOffset(globals, recipe.pointer_identity, recipe.xor_key_identity,
                                      recipe.initial_xor_key, 0)));
    throw std::invalid_argument("Player hook field has no proven receive/datamap recipe");
}
EntityNetworkableOffsets ragdoll_fields(client::ClientRuntimeStorage& storage) {
    return {{offset(storage, 0x73DBBB49), offset(storage, 0x38CA5228), offset(storage, 0x9C14F5A7),
             offset(storage, 0x1B5F47D5), offset(storage, 0xFC2723F7), offset(storage, 0x8481EA04),
             offset(storage, 0x258BF82B), offset(storage, 0xA9EE6062), offset(storage, 0x2A671B42),
             offset(storage, 0x3B1F6ECC), offset(storage, 0xC865F10E)},
            offset(storage, 0x688E0292),
            offset(storage, 0xD070684C),
            offset(storage, 0x2F4E97C4),
            offset(storage, 0x3B704B41)};
}
client::ClientRuntimeStorage& checked_storage(client::ClientRuntimeStorage& storage,
                                              const client::EngineRuntimeBindings& engine,
                                              client::ConfigurationRuntimeOwner& configuration,
                                              client::HistoryRuntimeOwner& history,
                                              animation::RuntimeOwner& animation,
                                              engine::HookRuntimeOwner& hooks) {
    auto& data = storage.data();
    const auto core = engine.interfaces.core();
    const auto& entity = history.entity_runtime();
    auto* globals = data.native_cell<std::byte*>(0x43467B04);
    if (&configuration.storage() != &storage || &history.storage() != &storage ||
        &history.players().storage() != &storage || &animation.storage() != &storage ||
        &hooks.storage() != &storage || &animation.players() != &history.players().cache() ||
        &engine.variables.data() != &data ||
        core.engine_client != data.native_cell<void*>(0x4346A514) ||
        core.entity_list != data.native_cell<void*>(0x4346A890) ||
        core.engine_trace != data.native_cell<void*>(0x43468304) ||
        entity.engine != core.engine_client || entity.entity_list != core.entity_list ||
        history.game_type().get() != engine.variables.at(0x43467A8C).get() ||
        history.teammates_are_enemies().get() != engine.variables.at(0x4346A568).get())
        throw std::invalid_argument(
            "Player auxiliary hooks must share canonical engine, history, animation and configuration owners");
    for (const auto cell :
         {0x4346A514u, 0x4346A890u, 0x43468304u, 0x4346A894u, 0x43467014u, 0x434758B4u})
        if (!data.native_cell<void*>(cell))
            throw std::invalid_argument(
                "Player auxiliary hooks require the actual input, trace and engine interfaces");
    if (!globals || !data.native_cell<std::int32_t*>(0x434678F8) ||
        reinterpret_cast<std::byte*>(data.native_cell<float*>(0x4346A338)) != globals + 0x20 ||
        &entity.interval != data.native_cell<float*>(0x4346A338) ||
        !data.native_cell<InvalidateEntityPhysics>(0x43468C10) ||
        !data.native_cell<engine::MemberFunction<void>>(0x43468320))
        throw std::invalid_argument(
            "Player auxiliary hooks require the published engine clock and render/physics entries");
    for (const auto identity : {0x43467A8Cu, 0x4346A568u, 0x4346A67Cu, 0x43467A78u})
        if (!engine.variables.at(identity).get())
            throw std::invalid_argument(
                "Player auxiliary hooks require their genuine acquired ConVars");
    return storage;
}
}
struct PlayerAuxiliaryHooks::Data {
    client::ClientRuntimeStorage& storage;
    client::HistoryRuntimeOwner& history;
    EntityVisibilityRelationships relationships;
    EntityVisibilityBindings visibility_bindings;
    EntityVisibilityRuntime visibility;
    FreeLookCaptureBindings capture;
    EntityNetworkableBindings ragdoll_bindings;
    EntityNetworkableRuntime ragdolls;
    Data(client::ClientRuntimeStorage& source, const client::EngineRuntimeBindings& engine,
         client::ConfigurationRuntimeOwner& configuration,
         client::HistoryRuntimeOwner& history_owner, animation::RuntimeOwner& animation,
         engine::HookRuntimeOwner& hooks)
        : storage(checked_storage(source, engine, configuration, history_owner, animation, hooks)),
          history(history_owner),
          relationships{source.data().native_cell<void*>(0x4346A890), history.game_type(),
                        history.teammates_are_enemies(), history.players().adjustments(),
                        history.target_offsets()},
          visibility_bindings{source.data().native_cell<void*>(0x4346A514),
                              &source.data().native_cell<void*>(0x43475808), relationships},
          visibility{configuration.effects_settings(), visibility_bindings},
          capture{source.data().native_cell<void*>(0x4346A894),
                  source.data().native_cell<void*>(0x43468304),
                  source.data().native_cell<void*>(0x43467014),
                  source.data().native_cell<void*>(0x4346A890),
                  source.data().native_cell<void* const*>(0x434758B4),
                  offset(source, 0x4E36DFD7),
                  source.data().native_cell<FreeLookOwner*>(0x43475A64),
                  source.data().native_cell<std::int32_t*>(0x434678F8),
                  animation.trigonometry()},
          ragdoll_bindings{visibility_bindings,
                           source.data().native_cell<void*>(0x4346A890),
                           source.data().native_cell<std::uint32_t>(0x4346A67C),
                           history.entity_runtime().offsets,
                           ragdoll_fields(source),
                           source.data().native_cell<InvalidateEntityPhysics>(0x43468C10),
                           source.data().native_cell<engine::MemberFunction<void>>(0x43468320),
                           source.data().native_cell<void*>(0x43467B04),
                           source.data().native_cell<std::uint32_t>(0x43467A78),
                           hooks.registry()},
          ragdolls{configuration.effects_settings(), configuration.colored_model_settings(),
                   history.players().cache(), ragdoll_bindings} {}
};
PlayerAuxiliaryHooks::PlayerAuxiliaryHooks(client::ClientRuntimeStorage& storage,
                                           const client::EngineRuntimeBindings& engine,
                                           client::ConfigurationRuntimeOwner& configuration,
                                           client::HistoryRuntimeOwner& history,
                                           animation::RuntimeOwner& animation,
                                           engine::HookRuntimeOwner& hooks)
    : data_(std::make_unique<Data>(storage, engine, configuration, history, animation, hooks)) {}
PlayerAuxiliaryHooks::~PlayerAuxiliaryHooks() = default;
client::ClientRuntimeStorage& PlayerAuxiliaryHooks::storage() const noexcept {
    return data_->storage;
}
client::HistoryRuntimeOwner& PlayerAuxiliaryHooks::history() const noexcept {
    return data_->history;
}
EntityVisibilityRuntime& PlayerAuxiliaryHooks::visibility() noexcept {
    return data_->visibility;
}
EntityNetworkableRuntime& PlayerAuxiliaryHooks::ragdolls() noexcept {
    return data_->ragdolls;
}
const FreeLookCaptureBindings& PlayerAuxiliaryHooks::free_look_capture() const noexcept {
    return data_->capture;
}
}

// ============================================================================
// Subcomponent: player_opacity.cpp
// ============================================================================

namespace skeet::effects {
namespace {
/* [removed duplicate field] */
bool convar_nonzero(const void* object) noexcept {
    for (;;) {
        const auto parent = field<const void*>(object, 0x1C);
        if (parent == object)
            break;
        object = parent;
    }
    return field<std::uint32_t>(object, 0x30) !=
           std::uint32_t(reinterpret_cast<std::uintptr_t>(object));
}
RageVector target_eye(void* player, std::ptrdiff_t duck_offset) {
    const auto* origin = engine::invoke<const RageVector*>(player, 10);
    auto eye = *origin;
    const float duck = field<float>(player, duck_offset), square = duck * duck;
    const float smooth = 1.0f - (square * 3.0f - (square + square) * duck);
    eye[2] = (smooth * 18.0f + 46.0f) + eye[2];
    return eye;
}
}
bool player_inside_active_smoke(void* player, const PlayerOpacityBindings& b) {
    if (!b.smoke_registry)
        return false;
    const auto* target_origin = engine::invoke<const RageVector*>(player, 10);
    void* registry = b.smoke_registry;
    std::int32_t index = 0;
    while (index < field<std::int32_t>(registry, 0xC)) {
        const auto* handles = field<const std::uint32_t*>(registry, 0);
        const auto handle = handles[index];
        if (handle != UINT32_MAX) {
            void* smoke = engine::invoke<void*>(b.entities.entity_list, 4, handle);
            if (smoke && field<std::uint8_t>(smoke, b.offsets.smoke_effect_58ae2423)) {
                const auto elapsed = std::bit_cast<std::int32_t>(
                    std::uint32_t(*b.smoke_clock) -
                    field<std::uint32_t>(smoke, b.offsets.smoke_tick_f267e956));
                const float time = float(elapsed) * b.interval;
                if (time > 0.0f && 18.0f > time) {
                    const auto* smoke_origin = engine::invoke<const RageVector*>(smoke, 10);
                    const float x = (*target_origin)[0] - (*smoke_origin)[0];
                    const float y = (*target_origin)[1] - (*smoke_origin)[1];
                    const float z = (*target_origin)[2] - (*smoke_origin)[2];
                    if (22201.0f > ((y * y + x * x) + z * z))
                        return true;
                }
            }
            registry = b.smoke_registry;
        }
        ++index;
    }
    return false;
}
bool player_near_camera_segment(void* player, const PlayerOpacityBindings& b) {
    void* local = b.entities.local_player;
    if (!local || field<std::uint8_t>(local, b.entities.offsets.life_state))
        return false;
    const auto* absolute = engine::invoke<const RageVector*>(local, 10);
    const auto offset = field<RageVector>(local, b.offsets.view_offset_c246315e);
    const RageVector eye{(*absolute)[0] + offset[0], (*absolute)[1] + offset[1],
                         (*absolute)[2] + offset[2]};
    const auto camera = b.camera_position;
    const auto target = target_eye(player, b.offsets.duck_amount_3309c0cf);
    const RageVector delta{camera[0] - eye[0], camera[1] - eye[1], camera[2] - eye[2]};
    const float length =
        std::sqrt((delta[0] * delta[0] + delta[1] * delta[1]) + delta[2] * delta[2]);
    const float inverse = 1.0f / (length + std::numeric_limits<float>::epsilon());
    const RageVector direction{inverse * delta[0], inverse * delta[1], inverse * delta[2]};
    const float projection =
        ((target[1] - eye[1]) * direction[1] + (target[0] - eye[0]) * direction[0]) +
        (target[2] - eye[2]) * direction[2];
    float distance = std::numeric_limits<float>::max();
    if (!(0.0f > projection) && !(projection > length)) {
        const float x = target[0] - (direction[0] * projection + eye[0]);
        const float y = target[1] - (direction[1] * projection + eye[1]);
        const float z = target[2] - (direction[2] * projection + eye[2]);
        distance = std::sqrt((x * x + y * y) + z * z);
    }
    return 36.0f > distance;
}
std::uint8_t evaluate_player_opacity(void* player, const PlayerOpacityBindings& b) {
    std::uint8_t alpha = field<std::uint8_t>(player, b.offsets.field_af649dac) ? 128 : 255;
    if (b.suspended)
        return alpha;
    const auto index = engine::invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
    if (b.player_adjustments.records[index].adjustments.disable_visuals)
        return alpha;
    const bool local = (field<std::uint8_t>(player, b.offsets.local_player_d070684c) & 1U) != 0;
    const bool teammate = !local && model_teammate(player, b.entities);
    const bool history =
        !local && !teammate && b.settings.shadow.value && convar_nonzero(b.lag_compensation.get());
    const auto pass = player_model_colors(b.settings, local, teammate, history);
    if (!(pass.flags & 8U) && player_inside_active_smoke(player, b))
        return alpha;
    if (pass.flags & 4U) {
        const auto visible = std::uint8_t(pass.visible_color >> 24);
        if (visible < alpha)
            alpha = visible;
    }
    if (engine::invoke<void*>(b.input, 32, std::int32_t(-1)) &&
        player_near_camera_segment(player, b))
        alpha = 60;
    return alpha;
}
void apply_player_opacity(void* player, const PlayerOpacityBindings& b) {
    if (!b.active_effects)
        return;
    const auto alpha = evaluate_player_opacity(player, b);
    engine::invoke<void>(player, 396, std::int32_t(alpha == 255 ? 0 : 4), true);
    engine::invoke<void>(player, 395, alpha);
}
} // namespace skeet::effects

// ============================================================================
// Subcomponent: player_opacity_owner.cpp
// ============================================================================

namespace skeet::effects {
namespace {
client::ClientRuntimeStorage& checked(client::ClientRuntimeStorage& storage,
                                      const client::EngineRuntimeBindings& engine,
                                      client::ConfigurationRuntimeOwner& settings,
                                      client::PlayerRuntimeOwner& players) {
    auto& data = storage.data();
    const auto core = engine.interfaces.core();
    if (&engine.variables.data() != &data || &settings.storage() != &storage ||
        &players.storage() != &storage || !core.engine_client || !core.entity_list ||
        core.engine_client != data.native_cell<void*>(0x4346A514u) ||
        core.entity_list != data.native_cell<void*>(0x4346A890u) ||
        !data.native_cell<void*>(0x4346A894u) ||
        players.adjustments().settings_base() != &data.native_cell<std::uint8_t>(0x4347BE08u))
        throw std::invalid_argument(
            "Player opacity requires its canonical engine, configuration and player owners");
    const auto* globals = data.native_cell<std::byte*>(0x43467B04u);
    if (!globals || data.native_cell<void*>(0x4346A338u) != globals + 0x20 ||
        data.native_cell<void*>(0x43468D4Cu) != globals + 0x1C ||
        !data.native_cell<void*>(0x434679C0u))
        throw std::invalid_argument(
            "Player opacity requires the published smoke registry and global clock aliases");
    for (const auto address : {0x43467A8Cu, 0x4346A568u, 0x43467A78u})
        if (!engine.variables.at(address).get())
            throw std::invalid_argument("Player opacity requires discovered native ConVars");
    return storage;
}
/* [dedup offset] */
ModelRelationshipOffsets relationships(const engine::NativeOffsetGlobals& globals) {
    return {offset(globals, 0x43468E78u, 0x434684E4u, 0x9D920E35u),
            offset(globals, 0x43468BD4u, 0x4346A678u, 0x7501CB91u),
            offset(globals, 0x4346A6CCu, 0x43468EF8u, 0x2F4E97C4u),
            offset(globals, 0x43468F84u, 0x43467E64u, 0x3B704B41u),
            offset(globals, 0x434688B0u, 0x4346A518u, 0x8B5586F6u),
            offset(globals, 0x434678ACu, 0x43467B7Cu, 0x430C8993u)};
}
PlayerOpacityOffsets opacity_offsets(const engine::NativeOffsetGlobals& globals) {
    return {offset(globals, 0x43467DD0u, 0x4346A610u, 0xAF649DACu),
            offset(globals, 0x43468BF8u, 0x43467B8Cu, 0xD070684Cu),
            offset(globals, 0x43468A34u, 0x43468C00u, 0xC246315Eu),
            offset(globals, 0x434679F8u, 0x434675F8u, 0x3309C0CFu),
            offset(globals, 0x4346A530u, 0x43467564u, 0x58AE2423u),
            offset(globals, 0x43467AC0u, 0x43468E60u, 0xF267E956u)};
}
}
PlayerOpacityOwner::PlayerOpacityOwner(client::ClientRuntimeStorage& storage,
                                       const client::EngineRuntimeBindings& engine,
                                       client::ConfigurationRuntimeOwner& settings,
                                       client::PlayerRuntimeOwner& players)
    : storage_(checked(storage, engine, settings, players)),
      entities_{engine.interfaces.core().engine_client,
                engine.interfaces.core().entity_list,
                storage_.data().native_cell<void*>(0x43475808u),
                relationships(storage_.offsets().globals),
                engine.variables.at(0x43467A8Cu),
                engine.variables.at(0x4346A568u)},
      bindings_{storage_.data().native_cell<EffectsCommandOwner*>(0x43475A1Cu),
                settings.colored_model_settings(),
                entities_,
                players.adjustments(),
                storage_.data().native_cell<void*>(0x4346A894u),
                storage_.data().native_cell<void*>(0x434679C0u),
                storage_.data().native_cell<const std::int32_t*>(0x43468D4Cu),
                *storage_.data().native_cell<float*>(0x4346A338u),
                storage_.data().native_cell<RageVector>(0x43479CCCu),
                settings.hide_from_obs().enabled.value,
                engine.variables.at(0x43467A78u),
                opacity_offsets(storage_.offsets().globals)} {}
} //namespace skeet::effects

// ============================================================================
// Subcomponent: projectile_ribbons.cpp
// ============================================================================

namespace skeet::effects {
using engine::invoke;
namespace {
/* [removed duplicate field] */
std::int32_t entry_type(const ProjectileRibbonEntry& entry) noexcept {
    return entry.state;
}
std::uint32_t entry_type(const esp::GrenadeWarningRecord& entry) noexcept {
    return entry.type;
}
bool entry_suppressed(const ProjectileRibbonEntry& entry) noexcept {
    return entry.rejected != 0;
}
bool entry_suppressed(const esp::GrenadeWarningRecord& entry) noexcept {
    return entry.suppressed;
}

// Native KeyValues allocation/constructor 433F1094 and symbol setter 43328E4B.
// The final word is deliberately not initialized by the native constructor.
/* [dedup KeyValues] */

void* create_material(const ProjectileRibbonBindings& bindings, const char* shader) {
    auto* values =
        static_cast<KeyValues*>(invoke<void*>(bindings.key_values_system, 2, std::int32_t(0x2c)));
    if (values) {
        values->symbol = 0;
        values->string_value = nullptr;
        values->wide_value = nullptr;
        values->value = 0;
        values->type = values->escape = 0;
        values->symbol_high = 0;
        values->system = bindings.key_values_system;
        values->case_sensitive = false;
        values->peer = values->sub = values->chain = nullptr;
        std::uint32_t insensitive = UINT32_MAX;
        const auto sensitive =
            invoke<std::uint32_t>(values->system, 10, &insensitive, shader, true);
        values->symbol = (insensitive & 0x00ffffffu) | ((sensitive & 255u) << 24);
        values->symbol_high = std::uint16_t(sensitive >> 8);
    }
    char name[64]{};
    // Native format at 433806A0 is m%I64u; this is the same unsigned value.
    std::snprintf(name, sizeof name, "m%llu", static_cast<unsigned long long>(__rdtsc()));
    return invoke<void*>(bindings.material_system, 83, static_cast<const char*>(name), values);
}
}

ProjectileRibbonOffsets::ProjectileRibbonOffsets(const engine::NativeOffsetGlobals& globals)
    : inactive{globals, 0x43467210, 0x43467630, 0x01E80986, 0},
      effects{globals, 0x43468B50, 0x43467B48, 0x38CA5228, 0},
      thrower_handle{globals, 0x43468C18, 0x4346A888, 0xDC743513, 0},
      velocity{globals, 0x4346A8E4, 0x434688CC, 0x36375C23, 0},
      previous_velocity{globals, 0x434683E8, 0x434693C0, 0x434E9E95, 0},
      simulation_time{globals, 0x43469198, 0x4346A538, 0xD5FD2E2C, 0},
      origin{globals, 0x434670CC, 0x43468BCC, 0xA9EE6062, 0},
      collision_group{globals, 0x434683C8, 0x4346758C, 0xE4E2D1A7, 0},
      projectile_flags{globals, 0x43467488, 0x43467DEC, 0x7418868F, 0} {}

void ProjectileRibbonOwner::prepare_material(const ProjectileRibbonBindings& bindings) {
    if (!material)
        material = create_material(bindings, material_names.shader.c_str());
    if (!material)
        return; // native continues drawing with the null auto-bind material

    auto* real = invoke<void*>(material, 99);
    if (invoke<bool>(real, 71) && invoke<void*>(real, 69) &&
        invoke<bool>(real, 30, std::int32_t(0x2000)))
        return;

    const auto lock = invoke<std::uint32_t>(bindings.material_system, 118);
    invoke<void>(real, 37);
    for (const auto flag : {0x10, 0x20, 0x2000, 0x200000})
        invoke<void>(real, 29, std::int32_t(flag), true);
    if (auto* variable =
            invoke<void*>(real, 47, material_names.render_mode.c_str(), &render_mode_token))
        invoke<void>(variable, 5, std::int32_t(5));
    auto* texture = invoke<void*>(bindings.material_system, 91, material_names.texture.c_str(),
                                  "Particle textures", true, std::int32_t(0));
    if (texture) {
        if (auto* variable =
                invoke<void*>(real, 47, material_names.base_texture.c_str(), &base_texture_token))
            invoke<void>(variable, 15, texture);
    }
    invoke<void>(real, 53);
    invoke<void>(bindings.material_system, 119, lock);
}

void ProjectileRibbonOwner::release_material() {
    if (material) {
        invoke<void>(material, 13);
        material = nullptr;
    }
}

template <class Frame, class Entry, class Environment>
void ProjectileRibbonOwner::draw_impl(const Frame& frame, std::span<const Entry> entries,
                                      const ProjectileRibbonBindings& bindings,
                                      const Environment& environment,
                                      esp::GrenadeTraceProvider trace,
                                      std::vector<esp::Vector3>& world_path) {
    if (frame.suppressed || !frame.active)
        return;
    const auto& offsets = bindings.offsets;
    for (const auto& entry : entries) {
        if (entry.handle == UINT32_MAX)
            continue;
        auto* entity = invoke<void*>(bindings.entity_list, 4, entry.handle);
        if (!entity || (entry_type(entry) != 0 && entry_type(entry) != 1) ||
            entry_suppressed(entry))
            continue;
        if (field<std::int32_t>(entity, offsets.inactive) != 0 ||
            (field<std::uint32_t>(entity, offsets.effects) & 0x20u))
            continue;
        const auto thrower = field<std::uint32_t>(entity, offsets.thrower_handle);
        if (thrower == UINT32_MAX || !invoke<void*>(bindings.entity_list, 4, thrower))
            continue;

        world_path.clear();
        world_path.push_back(*invoke<const esp::Vector3*>(entity, 10));
        const auto velocity = field<esp::Vector3>(entity, offsets.velocity);
        const auto previous = field<esp::Vector3>(entity, offsets.previous_velocity);
        const auto dx = velocity.x - previous.x;
        const auto dy = velocity.y - previous.y;
        const auto dz = velocity.z - previous.z;
        const auto speed_delta_squared = (dy * dy + dx * dx) + dz * dz;
        const auto current_time = speed_delta_squared < 1.0f
                                      ? float(frame.current_tick) * environment.tick_interval
                                      : field<float>(entity, offsets.simulation_time);

        esp::GrenadeState prediction{};
        prediction.owner_entity = std::uint32_t(reinterpret_cast<std::uintptr_t>(entity));
        esp::simulate_grenade(prediction, entry.kind, field<esp::Vector3>(entity, offsets.origin),
                              velocity, current_time, entry.creation_time, environment, trace,
                              nullptr, nullptr,
                              field<std::int32_t>(entity, offsets.collision_group),
                              field<std::uint16_t>(entity, offsets.projectile_flags), &world_path);
        if (world_path.size() <= 1)
            continue;

        if (entry.kind == esp::GrenadeKind::explosive) {
            esp::finalize_grenade(prediction, trace);
            prediction.position.z += 1.0f;
            world_path.push_back(prediction.position);
        } else if (entry.kind == esp::GrenadeKind::molotov) {
            esp::finalize_grenade(prediction, trace);
            const auto endpoint = prediction.position;
            const auto last = world_path.back();
            if (last.x != endpoint.x || last.y != endpoint.y || last.z != endpoint.z)
                world_path.push_back(endpoint);
        }

        const auto color = frame.color;
        auto* context = invoke<void*>(bindings.material_system, 115);
        if (context)
            invoke<void>(context, 2);
        prepare_material(bindings);
        esp::Vector3 camera;
        invoke<void>(context, 152, &camera);
        auto* mesh = invoke<void*>(context, 62, true, static_cast<void*>(nullptr),
                                   static_cast<void*>(nullptr), material);
        std::vector<render::BeamSegment> segments;
        segments.reserve(world_path.size());
        for (const auto position : world_path)
            segments.push_back({position, 0.0f, color, 0.0f, 10.0f});
        render::BeamGeometry geometry;
        render::build_beam_geometry(segments, camera, geometry);
        render::draw_beam_geometry(mesh, geometry);
        invoke<void>(context, 3);
        invoke<void>(context, 1);
    }
}

void ProjectileRibbonOwner::draw(const ProjectileRibbonFrame& frame,
                                 std::span<const ProjectileRibbonEntry> entries,
                                 const ProjectileRibbonBindings& bindings,
                                 const esp::GrenadeEnvironment& environment,
                                 esp::GrenadeTraceProvider trace) {
    draw_impl(frame, entries, bindings, environment, trace, path_);
}
void ProjectileRibbonOwner::draw(const ProjectileRibbonLiveFrame& frame,
                                 std::span<const esp::GrenadeWarningRecord> entries,
                                 const ProjectileRibbonBindings& bindings,
                                 const esp::GrenadeEnvironmentBindings& environment,
                                 esp::GrenadeTraceProvider trace,
                                 std::vector<esp::Vector3>& world_path) {
    draw_impl(frame, entries, bindings, environment, trace, world_path);
}

} // namespace skeet::effects

// ============================================================================
// Subcomponent: receive_proxies.cpp
// ============================================================================

namespace skeet::effects {
namespace {
VisualReceiveRuntime* active = nullptr;
/* [removed duplicate read] */
/* [removed duplicate write] */
void smoke(const engine::RecvProxyData* packet, void* entity, void* output) {
    active->receive_smoke(packet, entity, output);
}
void flash(const engine::RecvProxyData* packet, void* entity, void* output) {
    active->receive_flash(packet, entity, output);
}
void observer(const engine::RecvProxyData* packet, void* entity, void* output) {
    active->receive_observer(packet, entity, output);
}
void fov(const engine::RecvProxyData* packet, void* entity, void* output) {
    active->receive_fov(packet, entity, output);
}
void sun(const engine::RecvProxyData* packet, void* entity, void* output) {
    active->receive_sun(packet, entity, output);
}
constexpr std::array<VisualReceiveDescriptor, 5> descriptors{{
    {0xA98u, 0x71D79C49u, 0xF7DBD1ADu, 0x1362ED63u, 0x4337EDF0u, 0x434769ECu, smoke},
    {0xAACu, 0x5AB40334u, 0xE0281F50u, 0x7763C80Au, 0x4338A0C3u, 0x4347693Cu, flash},
    {0xAC0u, 0xE8D6D5BFu, 0xDC554CD3u, 0x487F26C4u, 0x4340C7B4u, 0x43476A2Cu, observer},
    {0xAD4u, 0xE8D6D5BFu, 0xDC554CD3u, 0xAE5C0B76u, 0x4339EBD4u, 0x4347692Cu, fov},
    {0xAE8u, 0xB66F52E5u, 0x44C7B439u, 0xF9D86BC1u, 0x433E978Bu, 0x43476ACCu, sun},
}};
// Only real engine virtuals are adapted for the complete43359AFA source body.
struct ViewSelection {
    engine::NativeDataImage& data;
    esp::RuntimePlayer local{}, observed{};
    esp::PlayerRuntimeEngine engine{};
    explicit ViewSelection(engine::NativeDataImage& source) : data(source) {
        engine.context = this;
        engine.by_handle = [](void* context, std::int32_t handle) -> esp::RuntimePlayer* {
            auto& self = *static_cast<ViewSelection*>(context);
            auto* entity =
                skeet::engine::invoke<void*>(self.data.native_cell<void*>(0x4346A890u), 4, handle);
            if (!entity)
                return nullptr;
            self.observed.engine_entity = entity;
            return &self.observed;
        };
        engine.is_player = [](void*, const esp::RuntimePlayer& player) {
            return skeet::engine::invoke<bool>(player.engine_entity, 158);
        };
    }
    void* selected() {
        const auto* result = esp::effective_view_player(engine);
        return result ? result->engine_entity : nullptr;
    }
};
}
std::span<const VisualReceiveDescriptor, 5> visual_receive_descriptors() noexcept {
    return descriptors;
}
VisualReceivePatchFailure::VisualReceivePatchFailure(std::size_t at,
                                                     const VisualReceiveDescriptor& row,
                                                     VisualReceivePatchFailureReason why)
    : std::runtime_error("Required visual receive patch cannot be initialized"), index(at),
      native_entry(row.native_entry), patch_address(row.patch_address), reason(why) {}
VisualReceiveRuntime::VisualReceiveRuntime(engine::NativeDataImage& data,
                                           const esp::EffectsSettings& settings,
                                           const engine::NativeOffsetStorage& offsets,
                                           client::PlayerStateCache& players)
    : data_(data), settings_(settings), offsets_(offsets), players_(players) {}
std::uint32_t VisualReceiveRuntime::offset(std::uint32_t pointer, std::uint32_t key,
                                           std::uint32_t salt) const {
    return *offsets_.globals.pointers.at(pointer) ^ offsets_.globals.keys.at(key) ^ salt;
}
void VisualReceiveRuntime::original(std::uint32_t cell, const engine::RecvProxyData* packet,
                                    void* entity, void* output) const {
    data_.native_cell<engine::RecvVarProxy>(cell)(packet, entity, output);
}
void VisualReceiveRuntime::receive_smoke(const engine::RecvProxyData* packet, void* entity,
                                         void* output) {
    if (!data_.native_cell<std::uint8_t>(0x43468E58u) &&
        data_.native_cell<WorldRuntime*>(0x4347585Cu)->settings.remove_smoke.value)
        write(entity, offset(0x43468970u, 0x4346A3A8u, 0x666EC5F2u), std::uint8_t(1));
    original(0x434769F4u, packet, entity, output);
}
void VisualReceiveRuntime::receive_flash(const engine::RecvProxyData* packet, void* entity,
                                         void* output) {
    if (settings_.remove_flash.value) {
        ViewSelection view(data_);
        if (auto* local = data_.native_cell<void*>(0x43475808u)) {
            view.local.engine_entity = local;
            view.engine.local_player = &view.local;
            // Preserve the original helper's short-circuit field reads.
            view.local.life_state =
                read<std::uint8_t>(local, offset(0x43468E78u, 0x434684E4u, 0x9D920E35u));
            if (view.local.life_state) {
                view.local.move_type =
                    read<std::uint8_t>(local, offset(0x43468BD4u, 0x4346A678u, 0x7501CB91u));
                if (view.local.move_type == 10) {
                    view.local.observer_mode =
                        read<std::int32_t>(local, offset(0x4346A6CCu, 0x43468EF8u, 0x2F4E97C4u));
                    if (view.local.observer_mode == 4 || view.local.observer_mode == 5)
                        view.local.observer_handle = read<std::int32_t>(
                            local, offset(0x43468F84u, 0x43467E64u, 0x3B704B41u));
                }
            }
        }
        if (entity == view.selected())
            return;
    }
    original(0x43476944u, packet, entity, output);
}
void VisualReceiveRuntime::capture(const engine::RecvProxyData* packet, void* entity,
                                   bool observer_mode) {
    const auto index = engine::invoke<std::uint32_t>(static_cast<std::byte*>(entity) + 8, 10);
    if (index - 1u <= 63u) {
        auto* state = players_.get(index);
        if (!state)
            throw std::bad_alloc();
        const auto value = read<std::int32_t>(packet, 8);
        if (observer_mode)
            state->native.captured_observer_mode = value;
        else
            state->native.captured_camera_fov = value;
    }
}
void VisualReceiveRuntime::receive_observer(const engine::RecvProxyData* packet, void* entity,
                                            void* output) {
    capture(packet, entity, true);
    if (settings_.third_person_dead.value &&
        read<std::uint8_t>(entity, offset(0x43468BF8u, 0x43467B8Cu, 0xD070684Cu)) &&
        read<std::int32_t>(packet, 8) == 4) {
        write(const_cast<engine::RecvProxyData*>(packet), 8, std::int32_t(5));
        if (engine::invoke<void*>(data_.native_cell<void*>(0x4346A894u), 32, std::int32_t(-1)))
            engine::invoke<void>(data_.native_cell<void*>(0x4346A894u), 36);
    }
    original(0x43476A34u, packet, entity, output);
}
void VisualReceiveRuntime::receive_fov(const engine::RecvProxyData* packet, void* entity,
                                       void* output) {
    capture(packet, entity, false);
    original(0x43476934u, packet, entity, output);
}
void VisualReceiveRuntime::receive_sun(const engine::RecvProxyData* packet, void* entity,
                                       void* output) {
    if (data_.native_cell<EffectsCommandOwner*>(0x43475A1Cu)->world.settings.brightness.value == 2)
        write(const_cast<engine::RecvProxyData*>(packet), 8, std::int32_t(0));
    original(0x43476AD4u, packet, entity, output);
}
void VisualReceiveRuntime::bind_receive_patches() {
    for (const auto& row : descriptors) {
        const auto& node = data_.native_cell<client::EntityUpdatePatch>(row.patch_address);
        if (!node.target || !node.after)
            throw std::invalid_argument(
                "Visual receive patches require genuine captured property targets and originals");
    }
    active = this;
    for (const auto& row : descriptors)
        data_.native_cell<client::EntityUpdatePatch>(row.patch_address).before =
            std::bit_cast<std::int32_t>(
                std::uint32_t(reinterpret_cast<std::uintptr_t>(row.replacement)));
}
void VisualReceiveRuntime::initialize_receive_patches() {
    auto* client = data_.native_cell<void*>(0x434683E0u);
    if (!client)
        throw std::invalid_argument("Visual receive patches require canonical VClient018");
    struct Capture {
        client::EntityUpdatePatch* node;
        engine::RecvProp* property;
        engine::RecvVarProxy original;
    };
    std::array<Capture, 5> captures;
    for (std::size_t index = 0; index < descriptors.size(); ++index) {
        const auto& row = descriptors[index];
        auto& node = data_.native_cell<client::EntityUpdatePatch>(row.patch_address);
        if (node.target || node.after || node.before)
            throw VisualReceivePatchFailure(index, row,
                                            VisualReceivePatchFailureReason::already_initialized);
        auto* property = engine::find_receive_property(client, row.class_hash, row.table_hash,
                                                       row.property_hash);
        if (!property)
            throw VisualReceivePatchFailure(index, row,
                                            VisualReceivePatchFailureReason::missing_property);
        if (!property->proxy)
            throw VisualReceivePatchFailure(index, row,
                                            VisualReceivePatchFailureReason::missing_original);
        captures[index] = {&node, property, property->proxy};
    }
    for (const auto& capture : captures) {
        capture.node->target = reinterpret_cast<volatile std::int32_t*>(&capture.property->proxy);
        capture.node->after = std::bit_cast<std::int32_t>(
            std::uint32_t(reinterpret_cast<std::uintptr_t>(capture.original)));
    }
    bind_receive_patches();
}
} //namespace skeet::effects

// ============================================================================
// Subcomponent: runtime_owner.cpp
// ============================================================================

namespace skeet::effects {
namespace {
void require(bool value, const char* message) {
    if (!value)
        throw std::invalid_argument(message);
}
client::ClientRuntimeStorage& checked(client::ClientRuntimeStorage& storage,
                                      const client::EngineRuntimeBindings& engine,
                                      client::ConfigurationRuntimeOwner& configuration,
                                      const engine::BuildMaterialBindings& imports,
                                      void* key_values) {
    auto& data = storage.data();
    require(&engine.variables.data() == &data && &configuration.storage() == &storage &&
                &imports.data == &data && key_values,
            "Effects must borrow one canonical engine, configuration, visual and material owner");
    require(imports.create_variable &&
                imports.create_variable == data.native_cell<CreateMaterialVariable>(0x43467A68u) &&
                imports.localization &&
                imports.localization == data.native_cell<void*>(0x4346A31Cu),
            "Effects require genuine acquired material and localization bindings");
    require(engine.modules.at(u"materialsystem.dll") == data.native_cell<void*>(0x4346A600u),
            "Effects material interface must belong to the published module bundle");
    const auto manifest = engine::build_interface_manifest();
    for (const auto cell : {0x4346A514u, 0x4346A890u, 0x4346A4E0u, 0x43467014u, 0x43467118u,
                            0x4346A44Cu, 0x4346A894u, 0x43469264u, 0x4346712Cu}) {
        // CInput is an independently proven inline object, not a factory row.
        if (cell == 0x4346A894u) {
            require(data.native_cell<void*>(cell) != nullptr,
                    "Effects require the acquired genuine CInput object");
            continue;
        }
        bool found = false;
        for (std::size_t index = 0; index < manifest.size(); ++index) {
            if (manifest[index].destination_cell != cell)
                continue;
            auto* object = data.native_cell<void*>(cell);
            require(object && object == engine.interfaces.objects[index],
                    "Effects interface differs from its acquired canonical engine object");
            found = true;
            break;
        }
        require(found, "Effects interface is absent from the fixed-build acquisition manifest");
    }
    require(storage.offsets().words && !storage.offsets().globals.pointers.empty() &&
                data.native_cell<const std::int32_t*>(0x43469300u) &&
                data.native_cell<void*>(0x434684C4u),
            "Effects require initialized offsets, the live client limit and genuine CSun class");
    require(engine.variables.at(0x4346A534u).get() != nullptr,
            "Effects view requires the acquired viewmodel_recoil ConVar");
    return storage;
}
void* key_values(client::ClientRuntimeStorage& storage, client::VisualRuntimeOwner& visual) {
    require(&visual.storage() == &storage, "Effects require the same canonical visual owner");
    return visual.key_values_system();
}
std::uint32_t offset(const engine::NativeOffsetGlobals& globals, std::uint32_t salt) {
    // This is the entity origin, not CTEFireBullets::m_vecOrigin, which
    // happens to carry the same XOR salt.
    if (salt == 0x2A671B42u)
        return std::uint32_t(
            std::ptrdiff_t(engine::DecodedOffset(globals, 0x43468D30u, 0x43468ED4u, salt, 0)));
    for (const auto& row : engine::build_receive_offset_recipes())
        if (row.initial_xor_key == salt)
            return std::uint32_t(std::ptrdiff_t(engine::DecodedOffset(
                globals, row.pointer_identity, row.xor_key_identity, salt, 0)));
    for (const auto& row : engine::build_data_map_offset_recipes())
        if (row.initial_xor_key == salt)
            return std::uint32_t(std::ptrdiff_t(engine::DecodedOffset(
                globals, row.pointer_identity, row.xor_key_identity, salt, 0)));
    if (salt == 0x3B1F6ECCu)
        return std::uint32_t(
            std::ptrdiff_t(engine::DecodedOffset(globals, 0x43468368u, 0x43467FACu, salt, 0)));
    if (salt == 0x54783141u)
        return std::uint32_t(
            std::ptrdiff_t(engine::DecodedOffset(globals, 0x43468E7Cu, 0x434674A8u, salt, 0)));
    if (salt == 0xD070684Cu)
        return std::uint32_t(
            std::ptrdiff_t(engine::DecodedOffset(globals, 0x43468BF8u, 0x43467B8Cu, salt, 0)));
    throw std::logic_error("Effects field has no recovered native offset recipe");
}
WorldNetvars world_fields(const engine::NativeOffsetGlobals& globals) {
    return {offset(globals, 0x37482375u), offset(globals, 0xDC91E6CFu),
            offset(globals, 0x60816092u)};
}
ViewOffsets view_fields(const engine::NativeOffsetGlobals& globals) {
    return {offset(globals, 0x9D920E35u), offset(globals, 0xCACAEECCu),
            offset(globals, 0xCD812654u), offset(globals, 0xFBE8C5EAu),
            offset(globals, 0x35B5B2ECu), offset(globals, 0x5AE32976u),
            offset(globals, 0x1BAE1168u)};
}
EntityRenderOffsets entity_fields(const engine::NativeOffsetGlobals& globals) {
    return {
        offset(globals, 0x73DBBB49u), offset(globals, 0x38CA5228u), offset(globals, 0x9C14F5A7u),
        offset(globals, 0x1B5F47D5u), offset(globals, 0xFC2723F7u), offset(globals, 0x8481EA04u),
        offset(globals, 0x258BF82Bu), offset(globals, 0xA9EE6062u), offset(globals, 0x2A671B42u),
        offset(globals, 0x3B1F6ECCu), offset(globals, 0xC865F10Eu)};
}
/* [dedup registry] */
float& frame_time(engine::NativeDataImage& data) {
    auto* globals = data.native_cell<std::byte*>(0x43467B04u);
    require(globals != nullptr, "Effects require the canonical gpGlobals frame-time alias");
    return *reinterpret_cast<float*>(globals + 0x14);
}
}
struct EffectsRuntimeOwner::Data {
    client::ClientRuntimeStorage& storage;
    const client::EngineRuntimeBindings& engine;
    client::ConfigurationRuntimeOwner& configuration;
    ModelMaterialInterfaces materials;
    ModelRenderRuntime models;
    WorldInterfaces world_interfaces;
    WorldRuntime world;
    ViewBindings view_interfaces;
    ViewRuntime view;
    SmokeRuntimeBindings smoke_engine;
    SmokeOwnerBindings smoke;
    ScopeOwnerBindings scope;
    ModelShotState shot;
    rage::ShotModelBoneStorage shot_bones;
    struct Command {
        const lagcomp::EntityNetvars animation_fields;
        EntityRenderBindings entities;
        const rage::ShotgunPattern shotgun;
        BulletImpactCommandBindings bullet;
        EffectsCommandOwner owner;
        Command(Data& data, CameraRuntimeOwner& camera, client::PlayerRuntimeOwner& players,
                engine::SceneQueryRuntimeOwner& scene, client::CrtRuntimeOwner& crt,
                const engine::BuildEspBindings& functions)
            : animation_fields(
                  client::native_history_entity_fields(data.storage.offsets().globals)),
              entities{data.storage.data().native_cell<void*>(0x4346A890u),
                       data.storage.data().native_cell<void*>(0x43467DC0u),
                       data.engine.variables.at(0x4346A67Cu),
                       data.storage.data().native_cell<InvalidateEntityPhysics>(0x43468C10u),
                       entity_fields(data.storage.offsets().globals),
                       animation_fields},
              shotgun(functions.shotgun_pattern()),
              bullet{data.storage.data().native_cell<void**>(0x4346A3A4u),
                     std::ptrdiff_t(offset(data.storage.offsets().globals, 0xD720AEA6u)),
                     std::ptrdiff_t(offset(data.storage.offsets().globals, 0x5C9F0FBBu)),
                     data.storage.data().native_cell<void*>(0x43468260u),
                     data.engine.variables.at(0x43468520u),
                     scene.queries().ballistics(),
                     data.storage.data().native_cell<void* const*>(0x43475B90u),
                     crt.spread().tables(),
                     crt.random(),
                     shotgun,
                     crt.trigonometry()},
              owner{camera.runtime(), data.world, data.models, players.cache(), entities, bullet} {}
    };
    std::unique_ptr<Command> command;
    struct Primary {
        Data& data;
        ModelClassification classification;
        ModelEntityBindings entities;
        const lagcomp::RotationMath rotations;
        ModelShadowTimingSource timing;
        LocalTransparencyInterfaces transparency;
        LocalTransparencyNetvars transparency_fields;
        ModelOcclusionInterfaces occlusion;
        ModelRenderLifecycleState lifecycle_state{};
        ModelRenderLifecycleBindings lifecycle_bindings;
        ModelRenderLifecycle lifecycle;
        WorldCallbackState world_state;
        WorldCallbackBindings world_bindings;
        WorldCallbacks updates;
        ModelRenderHookInputs draw;
        ModelRenderHooks hooks;
        client::EffectsRuntimeFactoryBindings factory;

        Primary(Data& source, CameraRuntimeOwner& camera, client::HistoryRuntimeOwner& history,
                animation::RuntimeOwner& bones, client::CrtRuntimeOwner& crt)
            : data(source), classification{ModelRenderableTables(source.storage.data()),
                                           source.storage.data().native_cell<void*>(0x4346A890u),
                                           offset(source.storage.offsets().globals, 0x38CA5228u),
                                           offset(source.storage.offsets().globals, 0x4E36DFD7u)},
              entities{source.storage.data().native_cell<void*>(0x4346A514u),
                       source.storage.data().native_cell<void*>(0x4346A890u),
                       nullptr,
                       offset(source.storage.offsets().globals, 0x9D920E35u),
                       offset(source.storage.offsets().globals, 0x7501CB91u),
                       offset(source.storage.offsets().globals, 0x2F4E97C4u),
                       offset(source.storage.offsets().globals, 0x3B704B41u),
                       offset(source.storage.offsets().globals, 0x8B5586F6u),
                       offset(source.storage.offsets().globals, 0x430C8993u),
                       offset(source.storage.offsets().globals, 0x49D93E62u) - 0x18u,
                       0,
                       false},
              rotations{crt.trigonometry().sine, crt.trigonometry().cosine,
                        history.recording_bindings().publication.arccosine,
                        history.recording_bindings().publication.atan2},
              timing(source.storage.data(), source.storage.offsets().globals),
              transparency{source.storage.data().native_cell<void*>(0x4346A894u),
                           source.storage.data().native_cell<void*>(0x4346A890u),
                           source.storage.data().native_cell<void*>(0x4346A3A0u)},
              transparency_fields{offset(source.storage.offsets().globals, 0xAF649DACu),
                                  offset(source.storage.offsets().globals, 0xCACAEECCu),
                                  offset(source.storage.offsets().globals, 0xC07D1150u),
                                  offset(source.storage.offsets().globals, 0xCD812654u)},
              occlusion{source.storage.data().native_cell<void*>(0x4346A890u),
                        source.storage.data().native_cell<void*>(0x4346A614u),
                        source.storage.data().native_cell<void*>(0x43469330u), nullptr, UINT32_MAX},
              lifecycle_bindings{source.storage.data().native_cell<ModelHookDescriptor>(
                                     studio_model_hook_descriptor),
                                 source.storage.data().native_cell<ModelHookDescriptor>(
                                     model_occlusion_hook_descriptor),
                                 camera.interfaces(),
                                 source.storage.data().native_cell<ObserverProxy>(0x43476A34u),
                                 source.storage.data().native_cell<void*>(0x43468A4Cu),
                                 source.storage.data().native_cell<void*>(0x4346A3A0u),
                                 source.storage.data().native_cell<void*>(0x43467204u),
                                 source.storage.data().native_cell<std::uint32_t>(0x4346D674u)},
              lifecycle(source.models, source.configuration.colored_model_settings(), source.world,
                        lifecycle_state, lifecycle_bindings),
              world_bindings{
                  lifecycle_bindings.first_hook,
                  lifecycle_bindings.second_hook,
                  source.storage.data().native_cell<std::uint8_t>(0x43467AECu),
                  source.engine.variables.at(0x4346A39Cu),
                  source.engine.variables.at(0x43467080u),
                  source.storage.data().native_cell<void*>(0x43467204u),
                  source.storage.data().native_cell<void*>(0x4346A3A0u),
                  source.engine.variables.at(0x43468DFCu),
                  camera.interfaces(),
                  camera.runtime().offsets,
                  source.storage.data().native_cell<std::uint32_t>(0x4346D674u),
                  source.storage.data().native_cell<const std::uint32_t*>(0x43469060u),
                  source.shot,
                  source.storage.data().native_cell<engine::MemberFunction<void>>(0x43468320u),
                  {{std::ptrdiff_t(offset(source.storage.offsets().globals, 0x54783141u)),
                    std::ptrdiff_t(offset(source.storage.offsets().globals, 0x2B554AD0u))},
                   offset(source.storage.offsets().globals, 0x38CA5228u),
                   offset(source.storage.offsets().globals, 0xFC3297F7u),
                   offset(source.storage.offsets().globals, 0xCA2B5FFBu)},
                  source.storage.input_runtime().bitmap(),
                  transparency,
                  transparency_fields,
                  source.storage.data().native_cell<std::uint8_t>(0x4346A65Cu),
                  frame_time(source.storage.data())},
              updates{source.world,    source.view, source.models,
                      lifecycle_state, world_state, world_bindings},
              draw{classification,
                   entities,
                   crt.trigonometry(),
                   rotations,
                   history.players().adjustments(),
                   ModelShadowSource(history.players().cache(), timing),
                   bones.bones(),
                   offset(source.storage.offsets().globals, 0xD070684Cu),
                   offset(source.storage.offsets().globals, 0xCDE290D2u),
                   offset(source.storage.offsets().globals, 0x9C14F5A7u),
                   offset(source.storage.offsets().globals, 0x8D1BD8D0u),
                   source.storage.data().native_cell<void*>(0x43475808u),
                   source.storage.data().native_cell<std::uint8_t>(0x4346A65Cu),
                   history.game_type(),
                   history.teammates_are_enemies(),
                   source.engine.variables.at(0x43467A78u),
                   {source.storage.data().native_cell<anti_aim::CommandOwner*>(0x43475940u),
                    source.storage.data().native_cell<const std::int32_t*>(0x43475B2Cu),
                    source.storage.data().native_cell<float>(0x43475890u),
                    source.storage.data().native_cell<RageVector>(0x43476414u),
                    source.storage.data().native_cell<RageVector>(0x4347A4F0u),
                    source.storage.data().native_cell<float>(0x43472534u)},
                   source.storage.data().native_cell<std::uint32_t>(0x4346D674u),
                   source.shot,
                   occlusion},
              hooks(source.models, source.world, draw, lifecycle_bindings.first_hook,
                    lifecycle_bindings.second_hook),
              factory{{registry(source.storage, 0x43475B78u), source.storage.command_phases(),
                       registry(source.storage, 0x43475A9Cu), source.storage.entity_observers(),
                       registry(source.storage, 0x43475988u), registry(source.storage, 0x43475AD8u),
                       source.storage.empty_ticks()},
                      {source.command->owner, lifecycle, hooks, updates,
                       source.storage.event_registry(),
                       source.storage.data().native_cell<EffectsCommandOwner*>(0x43475A1Cu),
                       source.storage.data(), classification}} {}
        static void initialize(void* context) {
            auto& self = *static_cast<Primary*>(context);
            client::create_effects_runtime_owner(self.factory, self.data.storage.configuration());
        }
    };
    std::unique_ptr<Primary> primary;
    Data(client::ClientRuntimeStorage& source, const client::EngineRuntimeBindings& bindings,
         client::ConfigurationRuntimeOwner& settings, const engine::BuildMaterialBindings& imports,
         void* key_values)
        : storage(checked(source, bindings, settings, imports, key_values)), engine(bindings),
          configuration(settings), materials(storage.data(), key_values),
          models(materials, settings.colored_model_settings()), world_interfaces(storage.data()),
          world(settings.effects_settings(), world_interfaces,
                world_fields(storage.offsets().globals),
                storage.data().native_cell<std::uint8_t>(0x43468E58u)),
          view_interfaces(storage.data()), view(settings.effects_settings(), view_interfaces,
                                                view_fields(storage.offsets().globals)),
          smoke_engine{storage.data().native_cell<void*>(0x4346A514u),
                       storage.data().native_cell<void*>(0x4346A890u),
                       storage.data().native_cell<void*>(0x434679C0u)},
          smoke{world, smoke_engine, storage.data().native_cell<WorldRuntime*>(0x4347585Cu)},
          scope{view, settings.effects_settings()}, shot(storage.data()),
          shot_bones(storage.data(), storage.memory()) {}
};
EffectsRuntimeOwner::EffectsRuntimeOwner(client::ClientRuntimeStorage& storage,
                                         const client::EngineRuntimeBindings& engine,
                                         client::ConfigurationRuntimeOwner& configuration,
                                         client::VisualRuntimeOwner& visual,
                                         const engine::BuildMaterialBindings& imports)
    : EffectsRuntimeOwner(storage, engine, configuration, imports, key_values(storage, visual)) {}
EffectsRuntimeOwner::EffectsRuntimeOwner(client::ClientRuntimeStorage& storage,
                                         const client::EngineRuntimeBindings& engine,
                                         client::ConfigurationRuntimeOwner& configuration,
                                         const engine::BuildMaterialBindings& imports,
                                         void* key_values)
    : data_(std::make_unique<Data>(storage, engine, configuration, imports, key_values)) {}
EffectsRuntimeOwner::~EffectsRuntimeOwner() = default;
client::ClientRuntimeStorage& EffectsRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
ModelRenderRuntime& EffectsRuntimeOwner::models() noexcept {
    return data_->models;
}
WorldRuntime& EffectsRuntimeOwner::world() noexcept {
    return data_->world;
}
ViewRuntime& EffectsRuntimeOwner::view() noexcept {
    return data_->view;
}
SmokeOwnerBindings& EffectsRuntimeOwner::smoke_bindings() noexcept {
    return data_->smoke;
}
ScopeOwnerBindings& EffectsRuntimeOwner::scope_bindings() noexcept {
    return data_->scope;
}
ModelShotState& EffectsRuntimeOwner::shot_state() noexcept {
    return data_->shot;
}
rage::ShotModelBoneStorage& EffectsRuntimeOwner::shot_bones() noexcept {
    return data_->shot_bones;
}
void EffectsRuntimeOwner::compose_command(CameraRuntimeOwner& camera,
                                          client::PlayerRuntimeOwner& players,
                                          engine::SceneQueryRuntimeOwner& scene,
                                          client::CrtRuntimeOwner& crt,
                                          const engine::BuildEspBindings& functions) {
    if (data_->command)
        throw std::logic_error("The canonical effects command was already composed");
    auto& storage = data_->storage;
    auto& image = storage.data();
    require(&camera.storage() == &storage && &players.storage() == &storage &&
                &scene.storage() == &storage && &scene.players() == &players &&
                &crt.storage() == &storage && &functions.data == &image &&
                &crt.spread().image() == &image,
            "Effects command must share its actual camera, player, scene, CRT and engine owners");
    require(&camera.runtime().force_dead == &data_->world.settings.third_person_dead.value &&
                &camera.runtime().alive_hotkey == &data_->world.settings.third_person_key.value,
            "Effects command and camera require the same canonical third-person settings");
    require(crt.random().seed && crt.random().uniform && crt.trigonometry().sine &&
                crt.trigonometry().cosine && crt.spread().tables().standard.data() &&
                crt.spread().tables().shotgun_patterns.data(),
            "Effects command requires its genuinely initialized CRT spread and numeric services");
    require(
        image.native_cell<void*>(0x43467DC0u) && image.native_cell<void*>(0x43475B90u) &&
            image.native_cell<InvalidateEntityPhysics>(0x43468C10u) &&
            image.native_cell<engine::ShotgunPatternEntry>(0x43467DDCu) &&
            data_->engine.variables.at(0x4346A67Cu).get() &&
            data_->engine.variables.at(0x43468520u).get(),
        "Effects command requires the recovered ragdoll, trace, physics, shotgun and ConVar bindings");
    data_->command =
        std::make_unique<Data::Command>(*data_, camera, players, scene, crt, functions);
}
EffectsCommandOwner& EffectsRuntimeOwner::command() {
    if (!data_->command)
        throw std::logic_error("Actual effects command dependencies have not been composed");
    return data_->command->owner;
}
void EffectsRuntimeOwner::compose_primary(CameraRuntimeOwner& camera,
                                          client::HistoryRuntimeOwner& history,
                                          animation::RuntimeOwner& bones,
                                          client::CrtRuntimeOwner& crt) {
    if (data_->primary)
        throw std::logic_error("The canonical primary effects owner was already composed");
    if (!data_->command)
        throw std::logic_error("Primary effects require the actual command owner first");
    auto& storage = data_->storage;
    auto& image = storage.data();
    const auto& recording = history.recording_bindings();
    require(
        &camera.storage() == &storage && &history.storage() == &storage &&
            &bones.storage() == &storage && &crt.storage() == &storage &&
            &camera.runtime() == &data_->command->owner.camera &&
            &history.players().cache() == &data_->command->owner.players &&
            &recording.players == &history.players().cache() &&
            &bones.players() == &recording.players && &recording.bones == &bones.bones(),
        "Primary effects must share their actual command, camera, player cache, history, animation and CRT owners");
    require(&crt.descriptor_storage().image() == &image &&
                static_cast<void*>(&crt.descriptor_storage().at(studio_model_hook_descriptor)) ==
                    &image.native_cell<ModelHookDescriptor>(studio_model_hook_descriptor) &&
                static_cast<void*>(&crt.descriptor_storage().at(model_occlusion_hook_descriptor)) ==
                    &image.native_cell<ModelHookDescriptor>(model_occlusion_hook_descriptor),
            "Effects hooks require the canonical source descriptor storage");
    require(recording.publication.arccosine && recording.publication.atan2 &&
                crt.trigonometry().sine && crt.trigonometry().cosine,
            "Effects history interpolation requires its genuine CRT numeric imports");
    const auto manifest = engine::build_interface_manifest();
    for (const auto cell : {0x43468A4Cu, 0x4346A3A0u, 0x4346A614u, 0x43469330u}) {
        bool found = false;
        for (std::size_t index = 0; index < manifest.size(); ++index) {
            if (manifest[index].destination_cell != cell)
                continue;
            auto* object = image.native_cell<void*>(cell);
            require(object && object == data_->engine.interfaces.objects[index],
                    "Primary effects interface differs from the acquired canonical engine object");
            found = true;
            break;
        }
        require(found, "Primary effects interface is absent from the acquisition manifest");
    }
    require(image.native_cell<void*>(0x43467204u) && image.native_cell<void*>(0x43475B2Cu) &&
                image.native_cell<void*>(0x43467EB8u) && image.native_cell<void*>(0x4346A338u),
            "Primary effects require their actual allocator and live timing aliases");
    for (const auto cell : {0x4346A39Cu, 0x43467080u, 0x43468DFCu, 0x43467A78u, 0x43467A8Cu,
                            0x4346A568u, 0x4346747Cu, 0x4346A748u, 0x4346A330u, 0x4346A850u})
        require(data_->engine.variables.at(cell).get() != nullptr,
                "Primary effects require their acquired world, camera and history ConVars");
    data_->primary = std::make_unique<Data::Primary>(*data_, camera, history, bones, crt);
}
client::EffectsRuntimeFactoryBindings& EffectsRuntimeOwner::factory_bindings() {
    if (!data_->primary)
        throw std::logic_error("Primary effects dependencies have not been composed");
    return data_->primary->factory;
}
ModelRenderHooks& EffectsRuntimeOwner::model_hooks() {
    if (!data_->primary)
        throw std::logic_error("Primary effects draw hooks have not been composed");
    return data_->primary->hooks;
}
ModelRenderLifecycle& EffectsRuntimeOwner::lifecycle() {
    return factory_bindings().owner.lifecycle;
}
WorldCallbacks& EffectsRuntimeOwner::updates() {
    return factory_bindings().owner.updates;
}
void EffectsRuntimeOwner::bind_initializers(client::NativeStartupSources& sources) {
    (void)factory_bindings();
    require(&sources.data() == &data_->storage.data(),
            "Effects startup must use its canonical source data image");
    sources.bind_initializer(
        {client::effects_runtime_factory, data_->primary.get(), &Data::Primary::initialize});
}
} //namespace skeet::effects

// ============================================================================
// Subcomponent: scope_owner.cpp
// ============================================================================

namespace skeet::effects {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define SCOPE_MEMBER __thiscall
#define SCOPE_FAST __fastcall
#else
#define SCOPE_MEMBER
#define SCOPE_FAST
#endif
using OwnerEntry = void(SCOPE_MEMBER*)(void*);
struct ScopeOwner;
struct ScopeHud : hud::HudFrameCallback {
    ScopeOwner* owner;
};
void SCOPE_MEMBER scope_destroy(void*);
void SCOPE_MEMBER scope_empty(void*) {} //433CCE8E
void SCOPE_MEMBER scope_discover(void*);
void SCOPE_FAST scope_draw(void*, ui::RenderQueue*, void*, std::int32_t*);
const std::array<OwnerEntry, 9> scope_primary_table{scope_destroy, scope_empty, scope_empty,    scope_empty, scope_empty,
                                              scope_empty,   scope_empty, scope_discover, scope_empty}; //434727D8
const std::array<hud::HudFrameEntry, 1> scope_hud_table{scope_draw}; //43470428
struct ScopeOwner {
    const OwnerEntry* vtable = nullptr;
    std::uint32_t untouched_4 = 0;
    ScopeHud hud{{nullptr}, this};
    ScopeViewObserver updates;
    ScopeOwnerBindings bindings;
    engine::JobMemory memory;
    ScopeOwner(const ScopeOwnerRegistries& registry, ConfigRegistries& config,
               ScopeOwnerBindings context)
        : updates(context.runtime), bindings(context), memory(registry.lifecycle.memory) {
        registry.lifecycle.append(this);
        hud::register_hud_frame(registry.hud, &hud);
        network::register_update_observer(registry.updates, updates.endpoint);
        vtable = scope_primary_table.data();
        hud.vtable = scope_hud_table.data();
        updates.initialize_vtable();
        initialize_boolean_config(config.descriptors, bindings.settings.remove_scope, 0x171EED00u);
        bindings.runtime.scope_panel = -1;
    }
};
void SCOPE_MEMBER scope_destroy(void* object) { //whole433B1A7A ->433837D0
    if (!object)
        return;
    auto* owner = static_cast<ScopeOwner*>(object);
    const auto memory = owner->memory;
    owner->~ScopeOwner();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
void SCOPE_MEMBER scope_discover(void* object) {
    static_cast<ScopeOwner*>(object)->bindings.runtime.initialize_scope_panel();
}
void SCOPE_FAST scope_draw(void* endpoint, ui::RenderQueue* queue, void* player, std::int32_t*) {
    auto& bindings =
        static_cast<ScopeHud*>(static_cast<hud::HudFrameCallback*>(endpoint))->owner->bindings;
    bindings.runtime.draw_scope_live(*queue, player);
}
#undef SCOPE_MEMBER
#undef SCOPE_FAST
}
std::size_t scope_owner_size() noexcept {
    return sizeof(ScopeOwner);
}
void construct_scope_owner(void* storage, const ScopeOwnerRegistries& registry,
                           ConfigRegistries& config, ScopeOwnerBindings bindings) {
    ::new (storage) ScopeOwner(registry, config, bindings);
}
} // namespace skeet::effects

// ============================================================================
// Subcomponent: smoke_owner.cpp
// ============================================================================

namespace skeet::effects {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define SMOKE_MEMBER __thiscall
#define SMOKE_FAST __fastcall
#else
#define SMOKE_MEMBER
#define SMOKE_FAST
#endif
using OwnerEntry = void(SMOKE_MEMBER*)(void*);
WorldRuntime** smoke_active_slot = nullptr;
struct SmokeOwner;
struct SmokeObserver : network::UpdateObserver {
    SmokeOwner* owner;
};
void SMOKE_MEMBER smoke_destroy(void*);
void SMOKE_MEMBER smoke_empty(void*) {} //433CCE8E
void SMOKE_MEMBER smoke_before(void*);
void SMOKE_MEMBER smoke_after(void*);
void SMOKE_FAST smoke_player_view_empty(void*, void*, std::uintptr_t, std::uintptr_t, std::uintptr_t) {
} //4337D653
void SMOKE_FAST smoke_view_empty(void*, float*, float*, float*) {} //4334A269
void SMOKE_FAST smoke_post_empty(void*, void*) {}
const std::array<OwnerEntry, 9> smoke_primary_table{smoke_destroy, smoke_empty, smoke_empty, smoke_empty, smoke_empty,
                                              smoke_empty,   smoke_empty, smoke_empty, smoke_empty}; //43470980
const network::UpdateObserverVtable smoke_update_table{
    {smoke_before, smoke_empty, smoke_empty, smoke_after, smoke_empty}, smoke_player_view_empty, smoke_view_empty, smoke_post_empty}; //4347005C
struct SmokeOwner {
    const OwnerEntry* vtable = nullptr;
    std::uint32_t untouched_4 = 0;
    SmokeObserver updates{{nullptr}, this};
    SmokeOwnerBindings bindings;
    engine::JobMemory memory;
    SmokeOwner(const SmokeOwnerRegistries& registry, ConfigRegistries& config,
               SmokeOwnerBindings context)
        : bindings(context), memory(registry.lifecycle.memory) {
        registry.lifecycle.append(this);
        network::register_update_observer(registry.updates, updates);
        vtable = smoke_primary_table.data();
        updates.vtable = &smoke_update_table;
        initialize_boolean_config(config.descriptors, bindings.runtime.settings.remove_smoke,
                                  0x623BD4F2u);
    }
};
void SMOKE_MEMBER smoke_before(void* object) {
    const auto& bindings =
        static_cast<SmokeObserver*>(static_cast<network::UpdateObserver*>(object))->owner->bindings;
    bindings.runtime.smoke_before_render(bindings.engine);
}
void SMOKE_MEMBER smoke_after(void* object) {
    const auto& bindings =
        static_cast<SmokeObserver*>(static_cast<network::UpdateObserver*>(object))->owner->bindings;
    bindings.runtime.smoke_after_render(bindings.engine);
}
void SMOKE_MEMBER smoke_destroy(void* object) { //whole433F216C ->433837D0
    *molotov_active_slot = nullptr;
    if (!object)
        return;
    auto* owner = static_cast<SmokeOwner*>(object);
    const auto memory = owner->memory;
    owner->~SmokeOwner();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
#undef SMOKE_MEMBER
#undef SMOKE_FAST
}
std::size_t smoke_owner_size() noexcept {
    return sizeof(SmokeOwner);
}
void construct_smoke_owner(void* storage, const SmokeOwnerRegistries& registry,
                           ConfigRegistries& config, SmokeOwnerBindings bindings) {
    ::new (storage) SmokeOwner(registry, config, bindings);
    smoke_active_slot = &bindings.active;
    bindings.active = &bindings.runtime;
}
} // namespace skeet::effects

// ============================================================================
// Subcomponent: view.cpp
// ============================================================================

namespace skeet::effects {
ViewBindings::ViewBindings(engine::NativeDataImage& data)
    : entity_list(data.native_cell<void*>(0x4346A890u)),
      hud_panels(data.native_cell<void*>(0x4346A44Cu)), input(data.native_cell<void*>(0x4346A894u)),
      prediction(data.native_cell<void*>(0x43469264u)),
      recoil_scale(data.native_cell<std::uint32_t>(0x4346A534u), 0xB739EB42u),
      suspended(&data.native_cell<std::uint8_t>(0x43468E58u)),
      calculating_view(&data.native_cell<std::uint8_t>(0x43475BD1u)),
      panel_surface(data.native_cell<void*>(0x4346712Cu)) {}
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
/* [removed duplicate parent] */
std::int32_t find_scope_panel(void* panels, std::int32_t id) {
    const auto* name = engine::invoke<const char*>(panels, 36, id);
    std::uint32_t hash = 0x811C9DC5u;
    if (name)
        for (; *name; ++name)
            hash = (hash ^ std::uint8_t(*name)) * 0x01000193u;
    if ((hash ^ 0x92CA5DF1u) == 0x13DD15B4u)
        return id; // HudZoom
    const auto count = engine::invoke<std::int32_t>(panels, 17, id);
    for (std::int32_t child = 0; child < count; ++child) {
        const auto child_id = engine::invoke<std::int32_t>(panels, 18, id, child);
        const auto found = find_scope_panel(panels, child_id);
        if (found != -1)
            return found;
    }
    return -1;
}
}
void ViewRuntime::initialize_scope_panel() {
    // Native scope owner slot 5 is the recovered no-op 0x433CCE8E.
    const auto root = engine::invoke<std::int32_t>(interfaces.panel_surface, 1, std::int32_t(2));
    if (root != -1)
        scope_panel = find_scope_panel(interfaces.hud_panels, root);
}
void ViewRuntime::hide_scope_panel(void* player) {
    if (!settings.remove_scope.value || scope_panel == -1)
        return;
    const auto handle = read<std::int32_t>(player, offsets.active_weapon);
    if (handle == -1)
        return;
    void* weapon = engine::invoke<void*>(interfaces.entity_list, 4, handle);
    if (weapon && engine::invoke<std::int32_t>(weapon, 455) == 5)
        engine::invoke<void>(interfaces.hud_panels, 14, scope_panel, false);
}
bool ViewRuntime::scope_visible(void* player) {
    if (!settings.remove_scope.value || scope_panel == -1 || !player ||
        read<std::uint8_t>(player, offsets.life_state) ||
        !read<std::uint8_t>(player, offsets.scoped))
        return false;
    const auto handle = read<std::int32_t>(player, offsets.active_weapon);
    if (handle == -1)
        return false;
    void* weapon = engine::invoke<void*>(interfaces.entity_list, 4, handle);
    return weapon && engine::invoke<std::int32_t>(weapon, 455) == 5;
}
namespace {
void scope_rectangles(ui::RenderQueue& queue, Point viewport) {
    queue.rectangle({viewport.x / 2, 0, 1, viewport.y, 0xFF000000u});
    queue.rectangle({0, viewport.y / 2, viewport.x, 1, 0xFF000000u});
}
}
void ViewRuntime::draw_scope(ui::RenderQueue& queue, Point viewport, void* player) {
    if (scope_visible(player))
        scope_rectangles(queue, viewport);
}
void ViewRuntime::draw_scope_live(ui::RenderQueue& queue, void* player) {
    if (!scope_visible(player))
        return;
    scope_rectangles(queue, queue.screen());
}
void ViewRuntime::instant_scope(void* player) {
    if (!player || *interfaces.suspended || !settings.instant_scope.value)
        return;
    const auto fov = read<std::uint32_t>(player, offsets.fov);
    if (fov != read<std::uint32_t>(player, offsets.scope_transition_35b5b2ec))
        write(player, offsets.scope_transition_35b5b2ec, fov);
}
SavedRecoilAngles ViewRuntime::begin_recoil_angles(void* player) {
    SavedRecoilAngles state;
    if (!settings.visual_recoil.value || *interfaces.calculating_view ||
        engine::invoke<void*>(interfaces.input, 32, std::int32_t(-1)) ||
        read<std::uint8_t>(player, offsets.life_state) ||
        read<std::uint8_t>(interfaces.prediction, 8))
        return state;
    state.shake = static_cast<std::byte*>(player) + offsets.shake_angles_5ae32976;
    state.previous_shake = read<std::array<std::uint32_t, 3>>(state.shake, 0);
    write(state.shake, 0, std::array<std::uint32_t, 3>{});
    if (settings.visual_recoil.value == 2) {
        state.all = static_cast<std::byte*>(player) + offsets.recoil_angles_1bae1168;
        state.previous_all = read<std::array<std::uint32_t, 3>>(state.all, 0);
        write(state.all, 0, std::array<std::uint32_t, 3>{});
    }
    return state;
}
void ViewRuntime::restore_recoil_angles(const SavedRecoilAngles& state) {
    if (state.shake)
        write(state.shake, 0, state.previous_shake);
    if (state.all)
        write(state.all, 0, state.previous_all);
}
const float* ViewDuckTarget::get() const noexcept {
    if (!owner_)
        return target_;
    auto* const owner = *owner_;
    return owner ? &owner->duck_peek.target : nullptr;
}
SavedViewHeight ViewRuntime::begin_view_height(void* player, ViewDuckBindings& c) {
    const auto reset = [&]() {
        c.previous_duck = std::numeric_limits<float>::max();
        return SavedViewHeight{};
    };
    const auto* const duck_target = c.duck_target.get();
    if (!duck_target || !c.enabled || read<std::uint8_t>(player, offsets.life_state))
        return reset();
    const auto duck = read<float>(player, c.duck_amount);
    bool active = (c.command_flags & 0x40u) != 0;
    if (!active)
        active = hotkey_active(c.duck_key, c.input) || (c.command_flags & 0x100u) != 0;
    if (!(active && !(c.command_flags & 2u))) {
        if (c.previous_duck == std::numeric_limits<float>::max())
            return {};
        if (!(duck > 0.0f && duck < 1.0f && c.previous_duck >= duck))
            return reset();
    }
    c.previous_duck = duck;
    const auto target = *duck_target;
    const auto square = target * target;
    const auto doubled = (square + square) * target;
    const auto interpolation = 1.0f - (square * 3.0f - doubled);
    auto* address = static_cast<std::byte*>(player) + c.view_offset + 8u;
    SavedViewHeight saved{address, read<std::uint32_t>(address, 0)};
    write(address, 0, interpolation * 18.0f + 46.0f);
    return saved;
}
void ViewRuntime::restore_view_height(const SavedViewHeight& saved) {
    if (saved.address)
        write(saved.address, 0, saved.value);
}
template <bool hooked>
void ViewRuntime::calculate_view_transaction(void* player, void** original_table,
                                             ViewDuckBindings& duck,
                                             const engine::OwnerRegistry& registry,
                                             std::uintptr_t first, std::uintptr_t second,
                                             std::uintptr_t third) {
    const auto recoil = begin_recoil_angles(player);
    SavedViewHeight height;
    if (!read<std::uint8_t>(interfaces.prediction, 8) && !*interfaces.calculating_view) {
        dispatch_player_view(registry, player, first, second, third);
        height = begin_view_height(player, duck);
    }
    if constexpr (hooked) {
        const auto table = *static_cast<void***>(player);
        original_table = static_cast<engine::VtableHookMetadata*>(table[-2])->original_table;
    }
    reinterpret_cast<engine::MemberFunction<void, std::uintptr_t, std::uintptr_t, std::uintptr_t>>(
        original_table[366])(player, first, second, third);
    restore_view_height(height);
    restore_recoil_angles(recoil);
}
void ViewRuntime::calculate_view_hook(void* player, void** original_table, ViewDuckBindings& duck,
                                      const engine::OwnerRegistry& registry, std::uintptr_t first,
                                      std::uintptr_t second, std::uintptr_t third) {
    calculate_view_transaction<false>(player, original_table, duck, registry, first, second, third);
}
void ViewRuntime::calculate_hooked_view(void* player, ViewDuckBindings& duck,
                                        const engine::OwnerRegistry& registry, std::uintptr_t first,
                                        std::uintptr_t second, std::uintptr_t third) {
    calculate_view_transaction<true>(player, nullptr, duck, registry, first, second, third);
}
template <bool hooked>
void ViewRuntime::recoil_scale_transaction(void* player, void** original_table,
                                           const std::uint32_t& encoded_recoil_scale,
                                           std::uintptr_t first, std::uintptr_t second,
                                           std::uintptr_t third) {
    float previous = std::bit_cast<float>(0x7F7FFFFFu);
    if (settings.visual_recoil.value) {
        void* variable = interfaces.recoil_scale;
        if constexpr (hooked)
            variable = reinterpret_cast<void*>(std::uintptr_t(encoded_recoil_scale ^ 0xB739EB42u));
        previous = engine::invoke<float>(variable, 12);
        variable = interfaces.recoil_scale;
        if constexpr (hooked)
            variable = reinterpret_cast<void*>(std::uintptr_t(encoded_recoil_scale ^ 0xB739EB42u));
        void* p = parent(variable);
        const auto encoded = std::uint32_t(reinterpret_cast<std::uintptr_t>(p));
        write(p, 0x2c, encoded);
        write(p, 0x3c, encoded);
    }
    if constexpr (hooked) {
        const auto table = *static_cast<void***>(player);
        original_table = static_cast<engine::VtableHookMetadata*>(table[-2])->original_table;
    }
    reinterpret_cast<engine::MemberFunction<void, std::uintptr_t, std::uintptr_t, std::uintptr_t>>(
        original_table[250])(player, first, second, third);
    if (previous != std::bit_cast<float>(0x7F7FFFFFu)) {
        void* variable = interfaces.recoil_scale;
        if constexpr (hooked)
            variable = reinterpret_cast<void*>(std::uintptr_t(encoded_recoil_scale ^ 0xB739EB42u));
        void* p = parent(variable);
        const auto encoded = std::bit_cast<std::uint32_t>(previous) ^
                             std::uint32_t(reinterpret_cast<std::uintptr_t>(p));
        write(p, 0x2c, encoded);
        write(p, 0x3c, encoded);
    }
}
void ViewRuntime::recoil_scale_hook(void* player, void** original_table, std::uintptr_t first,
                                    std::uintptr_t second, std::uintptr_t third) {
    recoil_scale_transaction<false>(player, original_table, 0, first, second, third);
}
void ViewRuntime::recoil_scale_hooked(void* player, const std::uint32_t& encoded_recoil_scale,
                                      std::uintptr_t first, std::uintptr_t second,
                                      std::uintptr_t third) {
    recoil_scale_transaction<true>(player, nullptr, encoded_recoil_scale, first, second, third);
}
} // namespace skeet::effects

// ============================================================================
// Subcomponent: view_dispatch.cpp
// ============================================================================

namespace skeet::effects {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_VIEW_PHASE __thiscall
#define SKEET_VIEW_FAST __fastcall
#else
#define SKEET_VIEW_PHASE
#define SKEET_VIEW_FAST
#endif
void SKEET_VIEW_PHASE empty_phase(void*) {} //433CCE8E
void SKEET_VIEW_FAST empty_player_view(void*, void*, std::uintptr_t, std::uintptr_t,
                                       std::uintptr_t) {} //4337D653
void SKEET_VIEW_FAST empty_view(void*, float*, float*, float*) {} //4334A269
void SKEET_VIEW_FAST empty_post(void*, void*) {} //433CCE8E
void SKEET_VIEW_FAST scope_view(void* receiver, void* player, std::uintptr_t, std::uintptr_t,
                                std::uintptr_t) {
    static_cast<ScopeViewObserver*>(receiver)->runtime->hide_scope_panel(player);
}
void SKEET_VIEW_PHASE sensitivity(void* receiver) {
    static_cast<FovViewObserver*>(receiver)->sensitivity->update();
}
void SKEET_VIEW_FAST fov_view(void* receiver, float*, float*, float* fov) {
    static_cast<FovViewObserver*>(receiver)->runtime->update(*fov);
}
const network::UpdateObserverVtable scope_vtable{
    {empty_phase, empty_phase, empty_phase, empty_phase, empty_phase},
    scope_view,
    empty_view,
    empty_post};
const network::UpdateObserverVtable fov_vtable{
    {sensitivity, empty_phase, empty_phase, empty_phase, empty_phase},
    empty_player_view,
    fov_view,
    empty_post};
#undef SKEET_VIEW_PHASE
#undef SKEET_VIEW_FAST
}
ScopeViewObserver::ScopeViewObserver(ViewRuntime& runtime) : endpoint{}, runtime(&runtime) {}
void ScopeViewObserver::initialize_vtable() noexcept {
    endpoint.vtable = &scope_vtable;
}
ScopeViewObserver::ScopeViewObserver(engine::OwnerRegistryBinding registry, ViewRuntime& runtime)
    : ScopeViewObserver(runtime) {
    network::register_update_observer(registry, endpoint);
    initialize_vtable();
}
FovViewObserver::FovViewObserver(misc::ZoomSensitivityRuntime& sensitivity, ViewFovRuntime& runtime)
    : endpoint{}, sensitivity(&sensitivity), runtime(&runtime) {}
void FovViewObserver::initialize_vtable() noexcept {
    endpoint.vtable = &fov_vtable;
}
FovViewObserver::FovViewObserver(engine::OwnerRegistryBinding registry,
                                 misc::ZoomSensitivityRuntime& sensitivity, ViewFovRuntime& runtime)
    : FovViewObserver(sensitivity, runtime) {
    network::register_update_observer(registry, endpoint);
    initialize_vtable();
}
void dispatch_player_view(const engine::OwnerRegistry& registry, void* player, std::uintptr_t a,
                          std::uintptr_t b, std::uintptr_t c) {
    const auto end = registry.end;
    for (auto current = registry.begin; current != end; ++current) {
        const auto owner = static_cast<network::UpdateObserver*>(*current);
        owner->vtable->player_view(owner, player, a, b, c);
    }
}
bool dispatch_view(const engine::OwnerRegistry& registry, float* origin, float* angles,
                   float* fov) {
    const auto end = registry.end;
    for (auto current = registry.begin; current != end; ++current) {
        const auto owner = static_cast<network::UpdateObserver*>(*current);
        owner->vtable->view(owner, origin, angles, fov);
    }
    return false;
}
} // namespace skeet::effects

// ============================================================================
// Subcomponent: view_fov.cpp
// ============================================================================

namespace skeet::effects {
namespace {
/* [removed duplicate read] */
/* [removed duplicate add] */
/* [removed duplicate sub] */
// This adapter supplies only actual entity-list/player virtuals to the existing
// complete43359AFA observer resolver; it does not replace its selection logic.
struct LiveViewPlayer {
    ViewFovRuntime& runtime;
    esp::RuntimePlayer local{}, observed{};
    esp::PlayerRuntimeEngine engine{};
    explicit LiveViewPlayer(ViewFovRuntime& r) : runtime(r) {
        const auto player = *r.local_player;
        if (player) {
            local.engine_entity = player;
            local.life_state = read<std::uint8_t>(player, r.offsets.life_state);
            local.move_type = read<std::uint8_t>(player, r.offsets.movement_type);
            local.observer_mode = read<std::int32_t>(player, r.offsets.observer_mode);
            local.observer_handle = read<std::int32_t>(player, r.offsets.observer_target);
        }
        engine.context = this;
        engine.local_player = player ? &local : nullptr;
        engine.by_handle = [](void* context, std::int32_t handle) -> esp::RuntimePlayer* {
            auto& self = *static_cast<LiveViewPlayer*>(context);
            const auto player = skeet::engine::invoke<void*>(self.runtime.entity_list, 4, handle);
            if (!player)
                return nullptr;
            self.observed.engine_entity = player;
            return &self.observed;
        };
        engine.is_player = [](void*, const esp::RuntimePlayer& player) {
            return skeet::engine::invoke<bool>(player.engine_entity, 158);
        };
    }
    void* selected() {
        const auto player = esp::effective_view_player(engine);
        return player ? player->engine_entity : nullptr;
    }
};
}
void ViewFovRuntime::update(float& value) {
    LiveViewPlayer live(*this);
    const auto player = live.selected();
    if (!player || read<std::int32_t>(player, offsets.fov) == 45)
        return;
    const auto base = engine::invoke<std::int32_t>(player, 333);
    if (settings.zoom_fov.value != 100) {
        const auto difference = sub(_mm_cvttss_si32(_mm_set_ss(value)), base);
        if (difference) {
            auto scaled = _mm_cvtepi32_ps(_mm_cvtsi32_si128(settings.zoom_fov.value));
            scaled = _mm_mul_ss(scaled, _mm_set_ss(0.01f));
            scaled = _mm_mul_ss(scaled, _mm_cvtepi32_ps(_mm_cvtsi32_si128(difference)));
            scaled = _mm_add_ss(scaled, _mm_set_ss(0.5f));
            value = static_cast<float>(add(_mm_cvttss_si32(scaled), base));
        }
    }
    const auto difference = sub(settings.fov.value, base);
    if (difference) {
        auto shifted = _mm_set_ss(static_cast<float>(difference) + value);
        shifted = _mm_max_ss(shifted, _mm_set_ss(1.0f));
        shifted = _mm_min_ss(shifted, _mm_set_ss(160.0f));
        value = _mm_cvtss_f32(shifted);
    }
}
} // namespace skeet::effects

// ============================================================================
// Subcomponent: visual_receive_owner.cpp
// ============================================================================

namespace skeet::effects {
namespace {
client::ClientRuntimeStorage& checked(client::ClientRuntimeStorage& storage,
                                      client::ConfigurationRuntimeOwner& configuration,
                                      client::PlayerRuntimeOwner& players) {
    if (&configuration.storage() != &storage || &players.storage() != &storage ||
        &players.cache().addresses !=
            &storage.data().native_cell<std::array<client::NativePlayerStateLayout*, 64>>(
                0x434797BCu))
        throw std::invalid_argument(
            "Visual receivers require canonical configuration and player-cache owners");
    return storage;
}
}
VisualReceiveOwner::VisualReceiveOwner(client::ClientRuntimeStorage& storage,
                                       client::ConfigurationRuntimeOwner& configuration,
                                       client::PlayerRuntimeOwner& players)
    : storage_(checked(storage, configuration, players)),
      runtime_(storage_.data(), configuration.effects_settings(), storage_.offsets(),
               players.cache()) {}
} //namespace skeet::effects

// ============================================================================
// Subcomponent: world.cpp
// ============================================================================

namespace skeet::effects {
namespace {
using engine::invoke;
/* [removed duplicate read] */
/* [removed duplicate write] */
std::uint32_t hash_name(const char* text) {
    std::uint32_t value = 0x811C9DC5u;
    if (text)
        for (; *text; ++text)
            value = (value ^ static_cast<std::uint8_t>(*text)) * 0x01000193u;
    return value;
}
void* convar_parent(void* variable) {
    for (;;) {
        void* parent = read<void*>(variable, 0x1C);
        if (parent == variable)
            return variable;
        variable = parent;
    }
}
const esp::EffectsSettings* flash_settings = nullptr;
bool suppress_flash(const esp::EffectsSettings& settings, void* argument) {
    if (settings.remove_flash.value && argument && (read<std::uint8_t>(argument, 0x10) & 1)) {
        void* name = read<void*>(argument, 8);
        if (read<std::uint32_t>(name, 0x10) == 15) {
            const char* text = read<std::uint32_t>(name, 0x14) < 16 ? static_cast<const char*>(name)
                                                                    : read<const char*>(name, 0);
            const auto hash = hash_name(text) ^ 0x92CA5DF1u;
            if (hash == 0x51F0705Bu || hash == 0x77F6FC36u || hash == 0x69F55EF1u)
                return true;
        }
    }
    return false;
}
}

WorldInterfaces::WorldInterfaces(engine::NativeDataImage& data)
    : engine(data.native_cell<void*>(0x4346A514u)),
      material_system(data.native_cell<void*>(0x4346A4E0u)),
      static_props(data.native_cell<void*>(0x43467014u)),
      entity_list(data.native_cell<void*>(0x4346A890u)),
      max_clients(data.native_cell<const std::int32_t*>(0x43469300u)),
      tonemap_client_class(data.native_cell<void*>(0x434684C4u)),
      smoke_registry(data.native_cell<void*>(0x434679C0u)) {}
std::uint8_t classify_world_material(void* material, std::uint32_t group, bool dust2) {
    auto shader = invoke<const char*>(material, 49);
    if (shader) {
        const auto hash = hash_name(shader);
        if (hash == 0x288259D2u || hash == 0x57FE4F24u || hash == 0xC6D3B449u ||
            hash == 0xEC7340B0u || hash == 0xFAE67322u)
            return 0;
    }
    std::uint8_t flags = group == 0xBB2DA711u;
    if (flags || group == 0x2D3D8226u || group == 0xD402D032u) {
        if (invoke<std::int32_t>(material, 69) && !invoke<bool>(material, 17) &&
            !invoke<bool>(material, 30, std::uint32_t(0x200100))) {
            auto parameters = invoke<void**>(material, 41);
            bool first_empty =
                !parameters || !parameters[0] || !read<std::uint32_t>(parameters[0], 8);
            if (first_empty) {
                bool eligible = invoke<bool>(material, 19);
                if (!eligible) {
                    shader = invoke<const char*>(material, 49);
                    if (shader) {
                        const auto hash = hash_name(shader) ^ 0x92CA5DF1u;
                        eligible = hash == 0x9C79434Fu || hash == 0xFC16AAE8u;
                    }
                }
                if (eligible) {
                    const auto count = invoke<std::int32_t>(material, 40);
                    parameters = invoke<void**>(material, 41);
                    if (parameters && count > 6 && parameters[6] &&
                        invoke<bool>(parameters[6], 18)) {
                        bool phong = false;
                        if (dust2)
                            for (std::int32_t i = 0; i < count; ++i) {
                                void* parameter = parameters[i];
                                if (parameter &&
                                    (hash_name(invoke<const char*>(parameter, 2)) ^ 0x92CA5DF1u) ==
                                        0x5B95D50Au &&
                                    invoke<bool>(parameter, 18) &&
                                    invoke<std::int32_t>(parameter, 27)) {
                                    phong = true;
                                    break;
                                }
                            }
                        if (!phong)
                            flags |= 2;
                    }
                }
            }
        }
        if (invoke<bool>(material, 19))
            flags |= 4;
        if (group == 0x2D3D8226u || group == 0xD402D032u)
            return flags;
    }
    return flags | 8;
}

void apply_world_material(void* material, const WorldMaterialChange& change,
                          const std::array<void*, 9>& excluded) {
    const auto group = hash_name(invoke<const char*>(material, 1));
    if (group == 0x5CE5C520u || group == 0x74C16AE4u || group == 0x7E3CE5C1u ||
        group == 0xA1BEA871u || group == 0xF0813BA7u)
        return;
    for (void* owned : excluded)
        if (owned == material)
            return;
    void* actual = invoke<void*>(material, 99);
    const auto flags = classify_world_material(actual, group, change.dust2);
    if (!flags)
        return;
    if (change.refresh)
        invoke<void>(actual, 37);
    if (change.alpha && (flags & 1))
        invoke<void>(actual, 27, change.wall_alpha);
    if (change.fullbright) {
        if (flags & 2)
            invoke<void>(actual, 29, std::uint32_t(0x40), true);
        if (flags & 4)
            invoke<void>(actual, 29, std::uint32_t(0x08000000), true);
    }
    if (change.night && (flags & 8))
        invoke<void>(actual, 28, change.color[0], change.color[1], change.color[2]);
}

void WorldRuntime::update() {
    if (suspended)
        return;
    const auto prop = settings.prop_transparency.value;
    if (prop != previous_prop_transparency) {
        const auto multiplied = prop * std::uint32_t(255);
        const auto alpha = std::uint8_t(std::bit_cast<std::int32_t>(multiplied) / 100);
        for (std::int32_t index = 0;; ++index) {
            void* object = invoke<void*>(interfaces.static_props, 4, index);
            if (!object)
                break;
            object = invoke<void*>(object, 13);
            if (!object)
                continue;
            object = invoke<void*>(object, 9);
            if (!object)
                continue;
            if (prop < 100) {
                invoke<void>(object, 5, true);
                invoke<void>(object, 1, alpha);
            } else {
                invoke<void>(object, 1, std::uint8_t(255));
                invoke<void>(object, 5, false);
            }
        }
        previous_prop_transparency = settings.prop_transparency.value;
    }
    if (prop != 100 || previous_prop_transparency_2 != 100)
        previous_prop_transparency_2 = prop;
    const auto brightness = settings.brightness.value;
    const auto wall = settings.wall_transparency.value;
    const float wall_alpha = static_cast<float>(wall) * 0.01f;
    std::uint8_t changes = 0;
    if (bool(material_flags & 1) != (brightness == 1))
        changes = brightness == 1 ? 1 : 2;
    if (bool(material_flags & 4) != (wall != 100) || wall_alpha != previous_wall_alpha)
        changes |= wall == 100 ? 0x20 : 0x10;
    if (bool(material_flags & 2) != (brightness == 2))
        changes |= brightness == 2 ? 4 : 8;
    if (brightness == 2 && previous_night_rgb != (settings.night_color.value & 0xFFFFFFu))
        changes |= 4;
    if (!changes)
        return;
    WorldMaterialChange change;
    change.refresh = (changes & 0x2A) != 0;
    change.alpha = (change.refresh && wall != 100) || (changes & 0x10);
    change.wall_alpha = wall_alpha;
    change.fullbright = (change.refresh && brightness == 1) || (changes & 1);
    change.night = (change.refresh && brightness == 2) || (changes & 4);
    if (change.fullbright) {
        const char* level = invoke<const char*>(interfaces.engine, 53);
        change.dust2 = level && (hash_name(level) ^ 0x92CA5DF1u) == 0x1D7A7522u;
    }
    if (change.night)
        for (unsigned i = 0; i != 3; ++i)
            change.color[i] =
                float(std::uint8_t(settings.night_color.value >> (i * 8))) * 0.003921568859368563f;
    const auto context = invoke<void*>(interfaces.material_system, 118);
    auto handle = invoke<std::uint16_t>(interfaces.material_system, 86);
    while (handle != invoke<std::uint16_t>(interfaces.material_system, 88)) {
        void* material = invoke<void*>(interfaces.material_system, 89, handle);
        if (material && invoke<std::int32_t>(material, 56) > 0 && !invoke<bool>(material, 42) &&
            invoke<std::int32_t>(material, 69))
            apply_world_material(material, change, excluded_materials);
        handle = invoke<std::uint16_t>(interfaces.material_system, 87, handle);
    }
    invoke<void>(interfaces.material_system, 119, context);
    if (changes & 0xC) {
        const auto limit = read<std::int32_t>(interfaces.entity_list, 0x24);
        for (std::int32_t index = *interfaces.max_clients + 1; index < limit; ++index) {
            if (std::uint32_t(index) & 0xFFFF0000u)
                continue;
            void* entity = invoke<void*>(interfaces.entity_list, 3, index);
            if (entity && invoke<void*>(static_cast<std::byte*>(entity) + 8, 2) ==
                              interfaces.tonemap_client_class) {
                invoke<void>(static_cast<std::byte*>(entity) + 8, 6, std::int32_t(1));
                write(entity, offsets.tonemap_disabled,
                      std::uint8_t(settings.brightness.value != 2));
                break;
            }
        }
    }
    if (changes & 0x10)
        previous_wall_alpha = wall_alpha;
    else if (changes >= 0x20)
        previous_wall_alpha = 1.0f;
    if (changes & 4)
        previous_night_rgb = settings.night_color.value & 0xFFFFFFu;
    material_flags = (material_flags & 0xF8) | (settings.brightness.value == 1) |
                     (settings.brightness.value == 2 ? 2 : 0) |
                     (settings.wall_transparency.value == 100 ? 0 : 4);
}

void WorldConvarOverride::set(std::uint32_t value) {
    if (!variable)
        return;
    auto* parent = convar_parent(variable);
    saved =
        read<std::uint32_t>(parent, 0x30) ^ std::uint32_t(reinterpret_cast<std::uintptr_t>(parent));
    active = true;
    parent = convar_parent(variable);
    const auto encoded = std::uint32_t(reinterpret_cast<std::uintptr_t>(parent)) ^ value;
    write(parent, 0x30, encoded);
    write(parent, 0x40, encoded);
}
void WorldConvarOverride::restore() {
    if (!variable || !active)
        return;
    auto* parent = convar_parent(variable);
    const auto encoded = std::uint32_t(reinterpret_cast<std::uintptr_t>(parent)) ^ saved;
    write(parent, 0x30, encoded);
    write(parent, 0x40, encoded);
    active = false;
}
void WorldRuntime::begin_render() {
    if (suspended)
        return;
    const auto index = invoke<std::int32_t>(interfaces.engine, 12);
    void* local = invoke<void*>(interfaces.entity_list, 3, index);
    if (!local)
        return;
    begin_render_for_player(local);
}
void WorldRuntime::begin_render_for_player(void* local) {
    if (settings.disable_post_processing.value)
        post_processing.set(0);
    if (settings.remove_fog.value) {
        const auto handle = read<std::uint32_t>(local, offsets.fog_controller);
        if (handle != UINT32_MAX)
            if (void* fog = invoke<void*>(interfaces.entity_list, 4, handle)) {
                saved_fog_enabled = read<std::uint8_t>(fog, offsets.fog_enabled);
                write(fog, offsets.fog_enabled, std::uint8_t(0));
                material_flags |= 8;
            }
    } else if (fog_overridden()) {
        const auto handle = read<std::uint32_t>(local, offsets.fog_controller);
        if (handle != UINT32_MAX)
            if (void* fog = invoke<void*>(interfaces.entity_list, 4, handle))
                write(fog, offsets.fog_enabled, saved_fog_enabled);
        material_flags &= 0xF7;
    }
    if (settings.remove_sky.value)
        for (unsigned i = 0; i != sky.size(); ++i)
            sky[i].set(i == 3);
}
void WorldRuntime::end_render() {
    post_processing.restore();
    for (auto& value : sky)
        value.restore();
}
void WorldRuntime::smoke_before_render() {
    smoke_before_render({interfaces.engine, interfaces.entity_list, interfaces.smoke_registry});
}
void WorldRuntime::smoke_before_render(const SmokeRuntimeBindings& bindings) {
    if (suspended || !settings.remove_smoke.value)
        return;
    const auto index = invoke<std::int32_t>(bindings.engine, 12);
    if (invoke<void*>(bindings.entity_list, 3, index) && bindings.smoke_registry)
        write(bindings.smoke_registry, 0xC, std::uint32_t(0));
}
void WorldRuntime::smoke_after_render() {
    smoke_after_render({interfaces.engine, interfaces.entity_list, interfaces.smoke_registry});
}
void WorldRuntime::smoke_after_render(const SmokeRuntimeBindings& bindings) {
    if (!suspended && settings.remove_smoke.value && bindings.smoke_registry)
        write(bindings.smoke_registry, 0xC, std::uint32_t(0));
}
bool filter_flash_effect(const esp::EffectsSettings& settings, void* receiver,
                         void** original_vtable, void* argument) {
    if (suppress_flash(settings, argument))
        return true;
    return reinterpret_cast<engine::MemberFunction<bool, void*>>(original_vtable[13])(receiver,
                                                                                      argument);
}
void bind_flash_effect_runtime(const esp::EffectsSettings& settings) noexcept {
    flash_settings = &settings;
}
#if defined(_M_IX86) || defined(__i386__)
bool __fastcall flash_effect_hook(void* receiver, void*, void* argument) {
#else
bool flash_effect_hook(void* receiver, void*, void* argument) {
#endif
    if (suppress_flash(*flash_settings, argument))
        return true;
    const auto table = *static_cast<void***>(receiver);
    const auto metadata = static_cast<engine::VtableHookMetadata*>(table[-2]);
    return reinterpret_cast<engine::MemberFunction<bool, void*>>(metadata->original_table[13])(
        receiver, argument);
}

} // namespace skeet::effects

// ============================================================================
// Subcomponent: world_callbacks.cpp
// ============================================================================

namespace skeet::effects {
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
void exchange(ModelHookDescriptor& hook, void** table) {
    std::atomic_ref<void**>(*hook.object).exchange(table, std::memory_order_seq_cst);
}
bool camera_player(void* player, const CameraNetvars& offsets) {
    if (!read<std::uint8_t>(player, offsets.life_state))
        return true;
    const auto mode = read<std::int32_t>(player, offsets.observer_mode);
    return mode == 1 || mode == 2;
}
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_WORLD_PHASE __thiscall
#define SKEET_WORLD_FAST __fastcall
#else
#define SKEET_WORLD_PHASE
#define SKEET_WORLD_FAST
#endif
void SKEET_WORLD_PHASE begin(void* receiver) {
    static_cast<WorldUpdateObserver*>(receiver)->runtime->begin();
}
void SKEET_WORLD_PHASE end(void* receiver) {
    static_cast<WorldUpdateObserver*>(receiver)->runtime->end();
}
void SKEET_WORLD_PHASE clear(void* receiver) {
    static_cast<WorldUpdateObserver*>(receiver)->runtime->clear();
}
void SKEET_WORLD_PHASE world_empty(void*) {}
void SKEET_WORLD_PHASE prepare(void* receiver) {
    static_cast<WorldUpdateObserver*>(receiver)->runtime->prepare();
}
void SKEET_WORLD_FAST world_empty_player_view(void*, void*, std::uintptr_t, std::uintptr_t,
                                        std::uintptr_t) {} //4337D653
void SKEET_WORLD_FAST world_view(void* receiver, float*, float*, float*) {
    auto& r = *static_cast<WorldUpdateObserver*>(receiver)->runtime;
    update_shot_entity_bones(r.world.interfaces.entity_list, r.bindings.shot_entity_handle,
                             r.bindings.offsets.bones, r.bindings.bone_counter);
}
void SKEET_WORLD_FAST post_view(void* receiver, void* player) {
    auto& r = *static_cast<WorldUpdateObserver*>(receiver)->runtime;
    r.models.update_local_transparency(
        player, player ? static_cast<std::byte*>(player) + 4 : nullptr,
        r.bindings.transparency_interfaces, r.bindings.transparency_offsets, r.world.suspended != 0,
        r.bindings.player_effects != 0, r.bindings.frame_time);
}
const network::UpdateObserverVtable vtable{
    {begin, end, clear, world_empty, prepare}, world_empty_player_view, world_view, post_view};
#undef SKEET_WORLD_PHASE
#undef SKEET_WORLD_FAST
}
WorldUpdateObserver::WorldUpdateObserver(WorldCallbacks& runtime) : endpoint{}, runtime(&runtime) {}
void WorldUpdateObserver::initialize_vtable() noexcept {
    endpoint.vtable = &vtable;
}
WorldUpdateObserver::WorldUpdateObserver(engine::OwnerRegistryBinding registry,
                                         WorldCallbacks& runtime)
    : WorldUpdateObserver(runtime) {
    network::register_update_observer(registry, endpoint);
    initialize_vtable();
}
void WorldCallbacks::begin() {
    if (bindings.install_second_hook) {
        if (!state.second_hook_installed) {
            state.second_hook_installed = 1;
            exchange(bindings.second_hook, bindings.second_hook.replacement_table);
        }
    } else if (state.second_hook_installed) {
        state.second_hook_installed = 0;
        if (bindings.second_hook.object)
            exchange(bindings.second_hook, bindings.second_hook.original_table);
    }
    exchange(bindings.first_hook, bindings.first_hook.replacement_table);
    if (world.suspended)
        return;
    const auto index = engine::invoke<std::int32_t>(world.interfaces.engine, 12);
    void* local = engine::invoke<void*>(world.interfaces.entity_list, 3, index);
    if (!local)
        return;
    if (state.camera_pending) {
        engine::invoke<void>(local, 312, true);
        state.camera_pending = 0;
    }
    world.begin_render_for_player(local);
}
void WorldCallbacks::end() {
    if (state.saved_sky_name && bindings.sky_name_variable) {
        if (read<void*>(bindings.sky_name_variable, 0x24) == lifecycle.allocation_760)
            write(bindings.sky_name_variable, 0x24, state.saved_sky_name);
        else {
            lifecycle.allocation_760 = nullptr;
            engine::invoke<void>(bindings.allocator, 5, state.saved_sky_name);
        }
        state.saved_sky_name = nullptr;
    }
    if (bindings.first_hook.object)
        exchange(bindings.first_hook, bindings.first_hook.original_table);
    world.end_render();
}
void WorldCallbacks::clear() {
    auto* variable = bindings.clear_convar.get();
    for (;;) {
        auto* parent = read<void*>(variable, 0x1c);
        if (parent == variable)
            break;
        variable = parent;
    }
    const auto encoded = std::uint32_t(reinterpret_cast<std::uintptr_t>(variable));
    write(variable, 0x30, encoded);
    write(variable, 0x40, encoded);
}
void WorldCallbacks::prepare() {
    if (world.settings.brightness.value == 2 && bindings.sky_name_variable) {
        if (!lifecycle.allocation_760) {
            lifecycle.allocation_760 =
                engine::invoke<void*>(bindings.allocator, 1, std::uint32_t(17));
            if (lifecycle.allocation_760) {
                // Constructor4348EFEB..4348F03C decrypts these16 bytes and
                // appends the seventeenth null byte through433B2213.
                std::memcpy(lifecycle.allocation_760, state.sky_name.data(), 17);
            }
        }
        if (lifecycle.allocation_760) {
            state.saved_sky_name = read<void*>(bindings.sky_name_variable, 0x24);
            write(bindings.sky_name_variable, 0x24, lifecycle.allocation_760);
        }
    }
    if (bindings.shot_entity_handle != UINT32_MAX) {
        if (void* marker = engine::invoke<void*>(world.interfaces.entity_list, 4,
                                                 bindings.shot_entity_handle)) {
            void* renderable = static_cast<std::byte*>(marker) + 4;
            if (*engine::invoke<const std::uint16_t*>(renderable, 7) != UINT16_MAX) {
                auto* manager = bindings.model_manager;
                const auto handle = *engine::invoke<const std::uint16_t*>(renderable, 7);
                engine::invoke<void>(manager, 3, std::uint32_t(handle), true);
            }
            update_shot_entity_bones_for_entity(marker, bindings.offsets.bones,
                                                bindings.bone_counter);
            const auto duration = double(float(models.settings.on_shot_duration.value)) * 0.1;
            const auto now = engine::invoke<double>(models.interfaces.clock, 5);
            const auto remaining = bindings.shot.expires - now;
            if (!models.settings.on_shot.enabled.value || remaining <= 0.0 ||
                remaining > duration || bindings.shot.bones_begin == bindings.shot.bones_end) {
                if (bindings.shot_entity_handle != UINT32_MAX) {
                    if (void* current = engine::invoke<void*>(world.interfaces.entity_list, 4,
                                                              bindings.shot_entity_handle)) {
                        auto flags = read<std::uint32_t>(current, bindings.offsets.render_flags);
                        if (!(flags & 0x20u)) {
                            write(current, bindings.offsets.render_flags, flags | 0x20u);
                            if (bindings.invalidate_render_flags)
                                bindings.invalidate_render_flags(current);
                        }
                    }
                }
            }
        }
    }
    auto* local = bindings.camera.local_player;
    if (!local)
        return;
    if (!world.suspended) {
        if (models.settings.weapon_viewmodel.enabled.value) {
            auto* handles = static_cast<std::byte*>(local) + bindings.offsets.inventory;
            for (std::uint32_t i = 0; i < 48; ++i) {
                const auto handle = read<std::uint32_t>(handles, i * 4);
                if (handle == UINT32_MAX)
                    continue;
                auto* weapon = engine::invoke<void*>(world.interfaces.entity_list, 4, handle);
                if (!weapon)
                    continue;
                const auto attached =
                    read<std::uint32_t>(weapon, bindings.offsets.weapon_world_model);
                if (attached != UINT32_MAX)
                    if (auto* model =
                            engine::invoke<void*>(world.interfaces.entity_list, 4, attached))
                        engine::invoke<void>(model, 238, false);
                engine::invoke<void>(weapon, 238, false);
            }
            if (world.suspended)
                goto restore_camera;
        }
        view.instant_scope(local);
        if (world.suspended)
            goto restore_camera;
        if (world.settings.third_person_alive.value &&
            hotkey_active(world.settings.third_person_key.value.binding, bindings.input) &&
            read<std::int32_t>(local, bindings.camera_offsets.fov) != 45 &&
            camera_player(local, bindings.camera_offsets)) {
            write(bindings.camera.input, 0xa9, std::uint8_t(0));
            write(bindings.camera.input, 0x100, std::int32_t(1));
            world.material_flags |= 0x10;
            state.camera_pending = 1;
            engine::invoke<void>(bindings.camera_snap_to.get(), 16, true);
            return;
        }
    }
restore_camera:
    if (world.material_flags & 0x10) {
        engine::invoke<void>(bindings.camera.input, 36);
        engine::invoke<void>(local, 312, false);
        world.material_flags &= 0xef;
    }
}
} // namespace skeet::effects
