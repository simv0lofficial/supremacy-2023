// Bone Setup & Animation Fix Subsystem Implementation
#include "skeet/animation/bone_runtime.hpp"
#include "skeet/engine/opaque_native_word.hpp"
#include <algorithm>
#include <bit>
#include <cstring>
#include "skeet/animation/bones.hpp"
#include "skeet/animation/entity_update.hpp"
#include "skeet/animation/fix_animation_backup.hpp"
#include "skeet/animation/state_copy.hpp"
#include "skeet/engine/hash_growth.hpp"
#include <new>
#include <xmmintrin.h>
#include "skeet/animation/fix_animation_backup_cleanup.hpp"
#include "skeet/client/native_startup.hpp"
#include "skeet/client/shutdown.hpp"
#include "skeet/client/runtime_storage.hpp"
#include <stdexcept>
#include "skeet/animation/fix_animations.hpp"
#include "skeet/client/entity_update_dispatch.hpp"
#include "skeet/effects/player_opacity.hpp"
#include "skeet/prediction/history.hpp"
#include <atomic>
#include <cmath>
#include "skeet/animation/fix_runtime_bindings.hpp"
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/animation/runtime_owner.hpp"
#include "skeet/client/runtime_composition.hpp"
#include "skeet/movement/global_config.hpp"
#include "skeet/rage/settings.hpp"
#include "skeet/anti_aim/config.hpp"
#include "skeet/esp/effects_config.hpp"
#include <windows.h>
#include "skeet/animation/fix_runtime_owner.hpp"
#include "skeet/engine/hook_runtime_owner.hpp"
#include "skeet/animation/layer_receive.hpp"
#include "skeet/engine/interface_call.hpp"
#include <array>
#include "skeet/animation/layer_state.hpp"
#include <cstddef>
#include "skeet/animation/local_helpers.hpp"
#include "skeet/client/prediction_history_owner.hpp"
#include "skeet/resolver.hpp"
#include "skeet/animation/local_prediction_pose.hpp"
#include "skeet/lagcomp/native_history_boundaries.hpp"
#include "skeet/animation/native_trigonometry.hpp"
#include <emmintrin.h>
#include "skeet/client/player_physics_hooks.hpp"
#include "player_physics_hooks_detail.hpp"
#include "skeet/animation/remote_animation_preparation.hpp"
#include "skeet/animation/remote_setup_bones.hpp"
#include <intrin.h>
#include "skeet/animation/runtime_bindings.hpp"
#include <cstdint>


// ============================================================================
// Subcomponent: bone_runtime.cpp
// ============================================================================

namespace skeet::animation {
namespace {
template <class T> T read(const void* object, std::ptrdiff_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
template <class T> void write(void* object, std::ptrdiff_t offset, const T& value) noexcept {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof value);
}
template <class T> T* field_ptr(void* object, std::ptrdiff_t offset) noexcept {
    return reinterpret_cast<T*>(static_cast<std::byte*>(object) + offset);
}
const void* studio_wrapper(const void* player, std::ptrdiff_t offset) noexcept {
    const auto* wrapper = read<const void*>(player, offset);
    return wrapper && read<const void*>(wrapper, 0) ? wrapper : nullptr;
}
const std::byte* hitbox_set(const void* player, const void* wrapper,
                            std::ptrdiff_t offset) noexcept {
    const auto* header = read<const std::byte*>(wrapper, 0);
    const auto relative = std::uint32_t(read<std::int32_t>(player, offset)) * 12u +
                          std::uint32_t(read<std::int32_t>(header, 0xB0));
    return header + std::bit_cast<std::int32_t>(relative);
}
BoneModel hierarchy_model(const void* wrapper) noexcept {
    const auto* header = read<const void*>(wrapper, 0);
    const auto count = std::size_t(read<std::int32_t>(header, 0x9C));
    return {true,
            0,
            true,
            false,
            false,
            {read<const std::uint32_t*>(wrapper, 0x30), count},
            {read<const std::int32_t*>(wrapper, 0x44), count},
            {}};
}
Matrix3x4 root_matrix(const RageVector& angles, const RageVector& origin,
                      const RageTrigonometry& trig) {
    constexpr float radians = 0.01745329238474369f;
    const float sy = trig.sine(angles[1] * radians), cy = trig.cosine(angles[1] * radians);
    const float sp = trig.sine(angles[0] * radians), cp = trig.cosine(angles[0] * radians);
    const float sr = trig.sine(angles[2] * radians), cr = trig.cosine(angles[2] * radians);
    const float crcy = cr * cy, crsy = cr * sy, srcy = sr * cy, srsy = sr * sy;
    return {{{cp * cy, srcy * sp - crsy, crcy * sp + srsy, origin[0]},
             {cp * sy, srsy * sp + crcy, crsy * sp - srcy, origin[1]},
             {-sp, sr * cp, cr * cp, origin[2]}}};
}
}

BoneModel BoneModelContext::model() const noexcept {
    return {true,
            0,
            header != nullptr,
            virtual_model_required,
            virtual_model,
            flags,
            parents,
            {hitbox_bones.data(), std::size_t(std::max(0, hitbox_count))}};
}

BoneModelContext BoneRuntime::load_model(const void* player) const {
    BoneModelContext result;
    result.wrapper = studio_wrapper(player, fields.studio_wrapper_32e4daf1);
    if (!result.wrapper)
        return result;
    result.header = read<const void*>(result.wrapper, 0);
    const auto count = std::size_t(read<std::int32_t>(result.header, 0x9C));
    result.virtual_model_required = read<std::int32_t>(result.header, 0x150) != 0;
    result.virtual_model = read<const void*>(result.wrapper, 4) != nullptr;
    result.flags = {read<const std::uint32_t*>(result.wrapper, 0x30), count};
    result.parents = {read<const std::int32_t*>(result.wrapper, 0x44), count};
    const auto* set = hitbox_set(player, result.wrapper, fields.hitbox_set_c2b6fb10);
    if (!set)
        return result;
    result.hitbox_count = std::min(19, read<std::int32_t>(set, 4));
    if (result.hitbox_count <= 0)
        return result;
    const auto* descriptors = set + read<std::int32_t>(set, 8);
    for (std::int32_t i = 0; i < result.hitbox_count; ++i)
        result.hitbox_bones[std::size_t(i)] =
            read<std::uint32_t>(descriptors, std::ptrdiff_t(i) * 0x44);
    return result;
}

std::span<float, 24> BoneRuntime::pose_parameters(void* player) const noexcept {
    return std::span<float, 24>(field_ptr<float>(player, fields.pose_parameters), 24);
}

namespace {
std::span<Matrix3x4> matrix_output(std::span<Matrix3x4> output, std::size_t) noexcept {
    return output;
}
std::span<Matrix3x4> matrix_output(Matrix3x4* output, std::size_t count) noexcept {
    return {output, count};
}
template <class Output>
bool build_live_bones(const BoneRuntime& runtime, void* player, const BoneRequest& request,
                      Output output, std::int32_t* count) {
    const auto& fields = runtime.fields;
    const auto entity_list = runtime.entity_list;
    const auto& trigonometry = runtime.trigonometry;
    if (count)
        *count = 0;
    if (!engine::invoke<const void*>(static_cast<std::byte*>(player) + 4, 8))
        return false;
    if (read<std::int32_t>(player, fields.model_index_a25d8271) == -1)
        return false;
    // The wrapper is retained through the transaction, but its live header is
    // read again for the count and hierarchy after genuine engine boundaries.
    const auto* wrapper = read<const void*>(player, fields.studio_wrapper_32e4daf1);
    if (!wrapper)
        return false;
    const auto* header = read<const void*>(wrapper, 0);
    if (!header)
        return false;
    if (read<std::int32_t>(header, 0x150) && !read<const void*>(wrapper, 4))
        return false;

    auto* interpolation = field_ptr<std::uint8_t>(player, fields.interpolation_flags_57b2d535);
    const auto saved_interpolation = *interpolation;
    *interpolation = saved_interpolation | 2u;
    auto* animation_frame = field_ptr<std::uint32_t>(player, fields.animation_frame_ae5b982e);
    const auto saved_animation_frame = *animation_frame;
    *animation_frame = 0;
    auto* effects = field_ptr<std::uint32_t>(player, fields.effects_38ca5228);
    const auto saved_effects = *effects;
    *effects = saved_effects | 8u;
    auto* setup_flags = field_ptr<std::uint32_t>(player, fields.setup_flags_bb78dc43);
    const auto saved_setup_flags = *setup_flags;
    *setup_flags = request.options | 4u;

    std::uint32_t* related_setup_flags = nullptr;
    std::uint32_t saved_related_setup_flags = 0;
    void** ik_owner_slot = nullptr;
    void* saved_ik_owner = nullptr;
    const auto owner_handle = read<std::uint32_t>(player, fields.owner_handle_cd812654);
    if (owner_handle != UINT32_MAX) {
        auto* owner = engine::invoke<void*>(entity_list, 4, owner_handle);
        if (owner) {
            const auto related_handle = read<std::uint32_t>(owner, fields.related_handle_ca2b5ffb);
            if (related_handle != UINT32_MAX) {
                if (auto* related = engine::invoke<void*>(entity_list, 4, related_handle)) {
                    related_setup_flags =
                        field_ptr<std::uint32_t>(related, fields.setup_flags_bb78dc43);
                    saved_related_setup_flags = *related_setup_flags;
                    *related_setup_flags = read<std::uint32_t>(player, fields.setup_flags_bb78dc43);
                }
            }
            if (auto* animation = read<void*>(player, fields.animation_state_3d52a1a7)) {
                ik_owner_slot = field_ptr<void*>(animation, 0x5C);
                // The optional state may be a native caller's still-opaque
                // automatic copy. Save/restore the actual pointer bits.
                saved_ik_owner = engine::read_opaque_native_pointer(ik_owner_slot);
                *ik_owner_slot = owner;
            }
        }
    }

    if (count)
        *count = read<std::int32_t>(read<const void*>(wrapper, 0), 0x9C);
    auto* ik_mask = field_ptr<std::uint32_t>(player, fields.ik_mask_01d0bacc);
    const auto saved_ik_mask = *ik_mask;
    *ik_mask = (request.options & 2u) ? 0x300u : UINT32_MAX;
    std::uint32_t* layer_weight = nullptr;
    std::uint32_t saved_layer_weight = 0;
    if (read<std::int32_t>(player, fields.animation_layers_b538bba5 + 0xC) > 12) {
        layer_weight =
            field_ptr<std::uint32_t>(read<void*>(player, fields.animation_layers_b538bba5), 0x2C0);
        saved_layer_weight = *layer_weight;
        *layer_weight = 0;
    }
    auto* angles_first = field_ptr<RageVector>(player, fields.angles_3b1f6ecc);
    const auto saved_angles_first = *angles_first;
    *angles_first = request.angles;
    auto* angles_second = field_ptr<RageVector>(player, fields.angles_f8205eba);
    const auto saved_angles_second = *angles_second;
    *angles_second = request.angles;
    auto* origin_first = field_ptr<RageVector>(player, fields.origin_2a671b42);
    const auto saved_origin_first = *origin_first;
    *origin_first = request.origin;
    auto* origin_second = field_ptr<RageVector>(player, fields.origin_4e66bb34);
    const auto saved_origin_second = *origin_second;
    *origin_second = request.origin;
    auto* entity_flags = field_ptr<std::uint32_t>(player, fields.entity_flags_83a6cf2b);
    const auto saved_entity_flags = *entity_flags;
    *entity_flags = saved_entity_flags & 0xFFFF07FFu;
    *entity_flags |= 8u;

    std::array<RageVector, maximum_bones> positions;
    std::array<BoneQuaternion, maximum_bones> rotations;
    engine::invoke<void>(player, 0x338 / 4, wrapper, positions.data(), rotations.data(),
                         request.time, request.mask);
    auto* current_flags = field_ptr<std::uint32_t>(player, fields.entity_flags_83a6cf2b);
    *current_flags &= ~8u;
    const auto root = root_matrix(request.angles, request.origin, trigonometry);
    const auto model = hierarchy_model(wrapper);
    const auto matrices = matrix_output(output, model.parents.size());
    compose_bone_hierarchy(model, root, positions, rotations, request.starting_bone, request.mask,
                           matrices);
    if (request.attachments)
        runtime.update_bone_attachments(player, matrices, request.mask);

    // Restore the exact saved destinations in native order; genuine engine
    // changes to other fields, including layer count and bone accessors, remain.
    *entity_flags = saved_entity_flags;
    *origin_second = saved_origin_second;
    *origin_first = saved_origin_first;
    *angles_second = saved_angles_second;
    *angles_first = saved_angles_first;
    if (layer_weight)
        *layer_weight = saved_layer_weight;
    *ik_mask = saved_ik_mask;
    if (ik_owner_slot)
        *ik_owner_slot = saved_ik_owner;
    if (related_setup_flags)
        *related_setup_flags = saved_related_setup_flags;
    *setup_flags = saved_setup_flags;
    *effects = saved_effects;
    *animation_frame = saved_animation_frame;
    *interpolation = saved_interpolation;
    return true;
}
}
bool BoneRuntime::build_bone_matrices(void* player, const BoneRequest& request,
                                      std::span<Matrix3x4> output, std::int32_t* count) const {
    return build_live_bones(*this, player, request, output, count);
}
bool BoneRuntime::build_bone_matrices(void* player, const BoneRequest& request, Matrix3x4* output,
                                      std::int32_t* count) const {
    return build_live_bones(*this, player, request, output, count);
}

std::int32_t BoneRuntime::select_hitbox_bone_matrices(const void* player,
                                                      std::span<const Matrix3x4> bones,
                                                      std::span<Matrix3x4> hitboxes) const {
    const auto* wrapper = studio_wrapper(player, fields.studio_wrapper_32e4daf1);
    if (!wrapper)
        return 0;
    const auto* set = hitbox_set(player, wrapper, fields.hitbox_set_c2b6fb10);
    if (!set)
        return 0;
    const auto count = std::min(19, read<std::int32_t>(set, 4));
    if (count <= 0)
        return count;
    for (std::int32_t i = 0; i < count; ++i) {
        const auto relative = read<std::int32_t>(set, 8);
        const auto bone =
            read<std::uint32_t>(set, std::ptrdiff_t(relative) + std::ptrdiff_t(i) * 0x44);
        std::memcpy(&hitboxes[std::size_t(i)], &bones[bone], sizeof(Matrix3x4));
    }
    return count;
}

void BoneRuntime::scatter_hitbox_bone_matrices(const void* player,
                                               std::span<const Matrix3x4> hitboxes,
                                               std::span<Matrix3x4> bones) const {
    scatter_hitbox_bone_matrices(player, hitboxes, bones.data());
}
void BoneRuntime::scatter_hitbox_bone_matrices(const void* player,
                                               std::span<const Matrix3x4> hitboxes,
                                               Matrix3x4* bones) const {
    const auto* wrapper = studio_wrapper(player, fields.studio_wrapper_32e4daf1);
    if (!wrapper)
        return;
    const auto* set = hitbox_set(player, wrapper, fields.hitbox_set_c2b6fb10);
    if (!set)
        return;
    const auto count = std::min(19, read<std::int32_t>(set, 4));
    for (std::int32_t i = 0; i < count; ++i) {
        const auto relative = read<std::int32_t>(set, 8);
        const auto bone =
            read<std::uint32_t>(set, std::ptrdiff_t(relative) + std::ptrdiff_t(i) * 0x44);
        std::memcpy(&bones[bone], &hitboxes[std::size_t(i)], sizeof(Matrix3x4));
    }
}

void BoneRuntime::update_bone_attachments(void* player, std::span<Matrix3x4> bones,
                                          std::uint32_t mask) const {
    if (!update_attachments || !(mask & 0xFFF00u))
        return;
    auto* accessor = static_cast<std::byte*>(player) + fields.bone_accessor_54783141;
    const auto readable = read<std::uint32_t>(accessor, 8);
    write(accessor, 8, 0xFFF00u);
    const auto writable = read<std::uint32_t>(accessor, 0xC);
    write(accessor, 0xC, 0xFFF00u);
    update_attachments(player, bones.data(), mask);
    write(accessor, 0xC, writable);
    write(accessor, 8, readable);
}

std::int32_t BoneRuntime::refresh_hitbox_attachments(void* player,
                                                     std::span<Matrix3x4, 19> hitboxes,
                                                     std::span<Matrix3x4, 256> scratch) const {
    scatter_hitbox_bone_matrices(player, hitboxes, scratch);
    update_bone_attachments(player, scratch, 0x300u);
    return select_hitbox_bone_matrices(player, scratch, hitboxes);
}

std::int32_t BoneRuntime::build_resolver_hitbox_matrices(void* player, float yaw,
                                                         std::span<Matrix3x4> hitboxes) const {
    const auto origin = read<RageVector>(player, fields.bone_origin_a9ee6062);
    std::array<Matrix3x4, maximum_bones> bones;
    const BoneRequest request{{0.0f, yaw, 0.0f}, origin, 0.0f, 0x80300u, -1, 0, false};
    if (!build_bone_matrices(player, request, bones))
        return 0;
    return select_hitbox_bone_matrices(player, bones, hitboxes);
}
} // namespace skeet::animation

// ============================================================================
// Subcomponent: bones.cpp
// ============================================================================

namespace skeet::animation {
Matrix3x4 bone_local_matrix(const RageVector& position, const BoneQuaternion& q) noexcept {
    const float x = q[0], y = q[1], z = q[2], w = q[3];
    const double dx = x, dy = y, dz = z, dw = w;
    return {{{float((1.0 - (dy + dy) * dy) - (dz + dz) * dz), (x + x) * y - (w + w) * z,
              (x + x) * z + (w + w) * y, position[0]},
             {float((dx + dx) * dy + (dw + dw) * dz), (1.0f - (x + x) * x) - (z + z) * z,
              (y + y) * z - (w + w) * x, position[1]},
             {float((dx + dx) * dz - (dw + dw) * dy), (w + w) * x + (y + y) * z,
              (1.0f - (x + x) * x) - (y + y) * y, position[2]}}};
}
namespace {
Matrix3x4 concatenate(const Matrix3x4& parent, const Matrix3x4& local) noexcept {
    Matrix3x4 result;
    for (unsigned row = 0; row < 3; ++row)
        for (unsigned column = 0; column < 4; ++column)
            // Native skips unmasked output bones. If a descendant references
            // such a parent, observe the actual prior output bytes opaquely.
            result.rows[row][column] =
                ((engine::read_opaque_native_float(&parent.rows[row][2]) * local.rows[2][column] +
                  engine::read_opaque_native_float(&parent.rows[row][1]) * local.rows[1][column]) +
                 engine::read_opaque_native_float(&parent.rows[row][0]) * local.rows[0][column]) +
                (column == 3 ? engine::read_opaque_native_float(&parent.rows[row][3]) : 0.0f);
    return result;
}
}
void compose_bone_hierarchy(const BoneModel& model, const Matrix3x4& root,
                            std::span<const RageVector> positions,
                            std::span<const BoneQuaternion> rotations, std::int32_t starting_bone,
                            std::uint32_t mask, std::span<Matrix3x4> output) noexcept {
    const auto count = std::int32_t(model.parents.size());
    std::array<std::int32_t, maximum_bones> traversal;
    unsigned length = 0;
    if (starting_bone < -1 || starting_bone >= count)
        starting_bone = 0;
    if (starting_bone == -1) {
        length = unsigned(count);
        for (std::int32_t i = 0; i < count; ++i)
            traversal[unsigned(count - i - 1)] = i;
    } else {
        do {
            traversal[length++] = starting_bone;
            starting_bone = model.parents[unsigned(starting_bone)];
        } while (starting_bone != -1);
    }
    while (length) {
        const auto bone = unsigned(traversal[--length]);
        if (!(model.flags[bone] & mask))
            continue;
        const auto local = bone_local_matrix(positions[bone], rotations[bone]);
        const auto parent = model.parents[bone];
        output[bone] = concatenate(parent == -1 ? root : output[unsigned(parent)], local);
    }
}

bool build_bone_matrices(BoneEntityState& state, const BoneModel& model, const BoneRequest& request,
                         std::span<Matrix3x4> output, const BoneEngine& engine,
                         const RageTrigonometry& trig) {
    if (!model.renderable_model || model.model_index == -1 || !model.studio_header ||
        (model.virtual_model_required && !model.virtual_model))
        return false;
    const auto saved = state;
    state.interpolation_flags |= 2;
    state.animation_frame = 0;
    state.effects |= 8;
    state.setup_flags = request.options | 4;
    std::uint32_t owner_flags = 0;
    std::uintptr_t ik_owner = 0;
    if (state.owner_setup_flags) {
        owner_flags = *state.owner_setup_flags;
        *state.owner_setup_flags = state.setup_flags;
    }
    if (state.ik_owner_slot) {
        ik_owner = *state.ik_owner_slot;
        *state.ik_owner_slot = state.ik_owner;
    }
    state.ik_mask = request.options & 2 ? 0x300u : 0xffffffffu;
    if (state.layer_count > 12)
        state.last_layer_weight = 0.0f;
    state.angles[0] = state.angles[1] = request.angles;
    state.origins[0] = state.origins[1] = request.origin;
    state.entity_flags = (state.entity_flags & 0xffff07ffu) | 8;
    std::array<RageVector, maximum_bones> positions;
    std::array<BoneQuaternion, maximum_bones> rotations;
    engine.accumulate_pose(engine.context, state, model, positions, rotations, request.time,
                           request.mask);
    state.entity_flags &= ~8u;
    compose_bone_hierarchy(model, root_matrix(request.angles, request.origin, trig), positions,
                           rotations, request.starting_bone, request.mask, output);
    if (request.attachments && engine.update_attachments && (request.mask & 0xfff00u)) {
        const auto readable = state.readable_bones, writable = state.writable_bones;
        state.readable_bones = state.writable_bones = 0xfff00;
        engine.update_attachments(engine.context, state, output, request.mask);
        state.writable_bones = writable;
        state.readable_bones = readable;
    }
    if (saved.ik_owner_slot)
        *saved.ik_owner_slot = ik_owner;
    if (saved.owner_setup_flags)
        *saved.owner_setup_flags = owner_flags;
    state.entity_flags = saved.entity_flags;
    state.angles = saved.angles;
    state.origins = saved.origins;
    if (saved.layer_count > 12)
        state.last_layer_weight = saved.last_layer_weight;
    state.ik_mask = saved.ik_mask;
    state.setup_flags = saved.setup_flags;
    state.effects = saved.effects;
    state.animation_frame = saved.animation_frame;
    state.interpolation_flags = saved.interpolation_flags;
    return true;
}
std::int32_t select_hitbox_bone_matrices(const BoneModel& model, std::span<const Matrix3x4> bones,
                                         std::span<Matrix3x4> hitboxes) noexcept {
    if (!model.studio_header)
        return 0;
    const auto count = std::min(std::size_t(19), model.hitbox_bones.size());
    for (std::size_t i = 0; i < count; ++i)
        hitboxes[i] = bones[model.hitbox_bones[i]];
    return std::int32_t(count);
}
std::int32_t build_resolver_hitbox_matrices(BoneEntityState& state, const BoneModel& model,
                                            const RageVector& origin, float yaw,
                                            std::span<Matrix3x4> hitboxes, const BoneEngine& engine,
                                            const RageTrigonometry& trig) {
    std::array<Matrix3x4, maximum_bones> bones;
    if (!build_bone_matrices(state, model, {{0.0f, yaw, 0.0f}, origin, 0.0f, 0x80300, -1, 0, false},
                             bones, engine, trig))
        return 0;
    return select_hitbox_bone_matrices(model, bones, hitboxes);
}
} // namespace skeet::animation

// ============================================================================
// Subcomponent: entity_update.cpp
// ============================================================================

namespace skeet::animation {
namespace {
/* deduplicated helper */
/* deduplicated helper */
}
void initialize_animation_state(void* state) noexcept {
    std::memset(static_cast<std::byte*>(state) + 0x13C, 0, 20);
}
bool select_animation_angles(const AnimationAngleHistory& history, std::int32_t requested_frame,
                             RageVector& angles) noexcept {
    return select_animation_angles(AnimationAngleHistoryView(history), requested_frame, angles);
}
bool AnimationAngleHistoryView::active() const noexcept {
    std::uint8_t value;
    std::memcpy(&value, active_, sizeof value);
    return value != 0;
}
bool select_animation_angles(AnimationAngleHistoryView history, std::int32_t requested_frame,
                             RageVector& angles) noexcept {
    if (!history.active())
        return false;
    const auto current_frame = history.frame();
    for (const auto& sample : history.samples()) {
        if (current_frame < sample.frame && sample.frame <= requested_frame) {
            std::memcpy(angles.data(), sample.angles.data(), sizeof sample.angles);
            return true;
        }
    }
    return false;
}
void update_animation_state(void* state, const RageVector& angles,
                            const AnimationUpdateTiming& timing,
                            const AnimationUpdateEngine& engine) {
    if (!engine.update_state)
        return;
    // DEC wraps at INT_MIN rather than invoking signed-overflow behavior.
    write(state, 0x70,
          std::bit_cast<std::int32_t>(std::bit_cast<std::uint32_t>(timing.frame) - 1u));
    //4331EE04/4333DB01 can install their own partially unwritten automatic
    // state when the entity has no animation state. Keep that native stack
    // observation opaque; +70 above is already initialized on this path.
    if (engine::read_opaque_native_float(static_cast<const std::byte*>(state) + 0x6C) ==
        timing.current_time)
        write(state, 0x6C, timing.current_time - timing.interval);
    engine.update_state(engine.context, state, 0.0f, angles[1], angles[0], 1);
    timing.clear_reset_byte();
}
void capture_entity_animation(lagcomp::AnimationState& snapshot, const void* entity,
                              const lagcomp::EntityNetvars& offsets) noexcept {
    const auto* layers = static_cast<const std::byte*>(entity) + offsets.animation_layers;
    lagcomp::capture_animation(
        snapshot, read<const void*>(layers, 0), read<std::int32_t>(layers, 0xC),
        reinterpret_cast<const float*>(static_cast<const std::byte*>(entity) +
                                       offsets.pose_parameters));
}
void restore_entity_animation(const lagcomp::AnimationState& snapshot, void* entity,
                              const lagcomp::EntityNetvars& offsets) noexcept {
    const auto* layers = static_cast<std::byte*>(entity) + offsets.animation_layers;
    lagcomp::restore_animation(
        snapshot, read<void*>(layers, 0), read<std::int32_t>(layers, 0xC),
        reinterpret_cast<float*>(static_cast<std::byte*>(entity) + offsets.pose_parameters));
}
} // namespace skeet::animation

// ============================================================================
// Subcomponent: fix_animation_backup.cpp
// ============================================================================

namespace skeet::animation {
namespace {
FixAnimationBackupRuntime* active_backup;
/* deduplicated helper */
/* deduplicated helper */
// The immutable native256-word lower-bound table43473248..43473648. This is
// shared native hash policy data, not another animation-backup owner.
constexpr std::array<std::uint32_t, 256> bucket_primes{
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
}

void* FixAnimationBackupRuntime::allocate(std::size_t size) {
    if (!size)
        return nullptr;
    auto* raw = bindings.memory.allocate(bindings.memory.process_heap, 8, size + 0x20);
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
void FixAnimationBackupRuntime::free(void* allocation) {
    if (!allocation)
        return;
    auto* bytes = static_cast<std::byte*>(allocation);
    const auto displacement = read<std::uint32_t>(bytes, -8);
    bindings.memory.free(bindings.memory.process_heap, 0, bytes - 16 - displacement);
}

FixAnimationBackupNode* FixAnimationBackupRuntime::create_node(const std::uint32_t* key) {
    auto* node = ::new (allocate(sizeof(FixAnimationBackupNode))) FixAnimationBackupNode;
    // The key is intentionally read after the actual heap call, as43329CDF.
    node->entity_index = *key;
    std::memset(reinterpret_cast<std::byte*>(node) + 4, 0, 0x3C0);
    initialize_animation_state_vector(node->animation.data());
    node->next = nullptr;
    return node;
}

FixAnimationBackupRuntime::GrowthDecision
FixAnimationBackupRuntime::plan_growth(std::uint32_t buckets, std::uint32_t size) {
    const auto& b = bindings;
    const auto growth =
        engine::plan_native_hash_growth(buckets, size, b.growth.maximum_load, b.growth.multiplier,
                                        b.growth.next_resize, bucket_primes, b.ceiling);
    return {growth.rehash, growth.bucket_count};
}

void FixAnimationBackupRuntime::rehash(std::uint32_t count) {
    const auto& b = bindings;
    auto** next =
        static_cast<FixAnimationBackupNode**>(allocate((std::size_t(count) + 1) * sizeof(void*)));
    std::memset(next, 0, std::size_t(count) * sizeof(void*));
    next[count] = reinterpret_cast<FixAnimationBackupNode*>(~std::uintptr_t(0));
    // Allocation can reenter the source owner. Capture the current bucket
    // count/storage only after that genuine boundary, as4332C319 does.
    std::uint32_t old_bucket = 0;
    while (old_bucket < b.bucket_count) {
        auto** previous = b.buckets;
        while (auto* node = previous[old_bucket]) {
            const auto bucket = node->entity_index % count;
            previous[old_bucket] = node->next;
            node->next = next[bucket];
            next[bucket] = node;
            previous = b.buckets;
        }
        ++old_bucket;
    }
    if (b.bucket_count > 1)
        free(b.buckets);
    b.bucket_count = count;
    b.buckets = next;
}

FixAnimationBackupInsert FixAnimationBackupRuntime::find_or_insert(const std::uint32_t* key) {
    const auto& b = bindings;
    const auto identity = *key;
    auto bucket = identity % b.bucket_count;
    auto** slot = b.buckets + bucket;
    for (auto* node = *slot; node; node = node->next)
        if (node->entity_index == identity)
            return {node, slot, false};
    const auto decision = plan_growth(b.bucket_count, b.size);
    auto* node = create_node(key);
    if (decision.rehash) {
        bucket = identity % decision.bucket_count;
        rehash(decision.bucket_count);
    }
    node->next = b.buckets[bucket];
    b.buckets[bucket] = node;
    ++b.size;
    return {node, b.buckets + bucket, true};
}

void FixAnimationBackupRuntime::capture(void* player) {
    const auto& b = bindings;
    if (!b.mode || read<std::uint8_t>(player, b.life_state_9d920e35))
        return;
    const auto index = engine::invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
    auto* node = find_or_insert(&index).node;
    node->acknowledged_command = engine::invoke<std::int32_t>(b.entity.engine, 205);
    node->frame = b.prediction_frame;
    const auto* state = read<const void*>(player, b.bones.fields.animation_state_3d52a1a7);
    std::memcpy(node->animation.data(), state, 0x348);
    initialize_animation_state_vector(node->animation.data());
    std::memcpy(node->poses.data(), b.bones.pose_parameters(player).data(), sizeof node->poses);
    const std::array<std::uint32_t, 3> angles{0, read<std::uint32_t>(node->animation.data(), 0x80),
                                              0};
    std::memcpy(node->angles.data(), angles.data(), sizeof angles);
    node->collision_time = read<float>(player, b.entity.offsets.field_0d560fac);
}

void FixAnimationBackupRuntime::detach_entity(void* player) {
    // Complete433CB965. Each getter follows restoration/free of the previous
    // interface, so later genuine engine calls see the original vtables.
    for (const auto slot : {4u, 5u, 8u}) {
        auto* object = engine::invoke<void*>(player, slot);
        engine::uninstall_vtable_hook(detach.hooks, object);
    }
    const auto extra = detach.extra_interface_offset;
    if (extra)
        engine::uninstall_vtable_hook(detach.hooks, static_cast<std::byte*>(player) + extra);
    engine::uninstall_vtable_hook(detach.hooks, player);
    const auto index = engine::invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
    capture(player);
    write(detach.player_settings, std::size_t(index) * 0x20 + 4, UINT32_MAX);
    const auto selected =
        detach.target.handle == -1 ? 0x1FFFu : std::uint32_t(detach.target.handle) & 0xFFFFu;
    if (index == selected)
        client::reset_target_state(detach.target);
}

void bind_fix_animation_backup(FixAnimationBackupRuntime& runtime) noexcept {
    active_backup = &runtime;
}
#if defined(_M_IX86) || defined(__i386__)
void __fastcall capture_fix_animation_backup(void* player, void*) {
#else
void capture_fix_animation_backup(void* player, void*) {
#endif
    active_backup->capture(player);
}
#if defined(_M_IX86) || defined(__i386__)
void __fastcall detach_fix_animation_entity(void* player, void*) {
#else
void detach_fix_animation_entity(void* player, void*) {
#endif
    active_backup->detach_entity(player);
}
} // namespace skeet::animation

// ============================================================================
// Subcomponent: fix_animation_backup_cleanup.cpp
// ============================================================================

namespace skeet::animation {
namespace {
FixAnimationBackupCleanupContext* active_cleanup = nullptr;

void free_native_allocation(void* allocation, const engine::JobMemory& memory) {
    if (!allocation)
        return; //complete433837D0 null gate
    auto* bytes = static_cast<std::byte*>(allocation);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
void finalize_backup_map() {
    active_cleanup->finalize();
}
void append_backup_map_finalizer(void* owner) {
    auto& context = *static_cast<FixAnimationBackupCleanupContext*>(owner);
    //433B86B4 MOVECX43359C18 -> JMP433837F5. Only registration occurs here.
    (void)context.callbacks.append(context.source_finalizer(0x43359C18u));
}
}

void cleanup_fix_animation_backups(FixAnimationBackupRuntime& runtime, void* const& allocator) {
    const auto& bindings = runtime.bindings;
    cleanup_fix_animation_backups(
        {bindings.buckets, bindings.bucket_count, bindings.size, bindings.memory}, allocator);
}
void cleanup_fix_animation_backups(const FixAnimationBackupMapBindings& map,
                                   void* const& allocator) {
    auto** const initial_buckets = map.buckets;
    const auto initial_count = map.bucket_count;
    for (std::uint32_t index = 0; index < initial_count; ++index) {
        auto* node = initial_buckets[index];
        while (node) {
            //4339D4A0 saves next before either genuine free boundary. The
            // native vector header is node+148, i.e. animation+13C.
            auto* const next = node->next;
            release_animation_state_vector(node->animation.data(), allocator);
            free_native_allocation(node, map.memory);
            node = next;
        }
        // The original captured array is cleared even if an engine/free
        // boundary changed the current global bucket pointer while releasing.
        initial_buckets[index] = nullptr;
    }
    //4339D4CE reloads the live count after a nonempty traversal. The size write
    // follows that read; the tail's allocation pointer is then read afresh.
    const auto final_count = initial_count ? map.bucket_count : initial_count;
    map.size = 0;
    if (final_count > 1)
        free_native_allocation(map.buckets, map.memory);
}
FixAnimationBackupCleanupContext::FixAnimationBackupCleanupContext(
    client::ClientRuntimeStorage& storage)
    : callbacks(storage.exit_callbacks()),
      allocator(storage.data().native_cell<void*>(0x43467204u)),
      map_{storage.data().native_cell<FixAnimationBackupNode**>(0x4346D6E4u),
           storage.data().native_cell<std::uint32_t>(0x4346D6E8u),
           storage.data().native_cell<std::uint32_t>(0x4346D6ECu), storage.memory()} {
    if (callbacks.entries.size() != 512 || !map_.buckets || !map_.bucket_count ||
        !map_.memory.allocate || !map_.memory.free || !map_.memory.process_heap)
        throw std::invalid_argument(
            "Early animation cleanup requires the canonical initialized source map and process heap");
}
void FixAnimationBackupCleanupContext::require_producer(
    const FixAnimationBackupRuntime& runtime) const {
    const auto& actual = runtime.bindings;
    if (&map_.buckets != &actual.buckets || &map_.bucket_count != &actual.bucket_count ||
        &map_.size != &actual.size || map_.memory.allocate != actual.memory.allocate ||
        map_.memory.free != actual.memory.free ||
        map_.memory.process_heap != actual.memory.process_heap)
        throw std::invalid_argument(
            "Animation capture and early terminal cleanup must share the actual map and heap");
}
void FixAnimationBackupCleanupContext::finalize() {
    cleanup_fix_animation_backups(map_, allocator);
}
void (*FixAnimationBackupCleanupContext::source_finalizer(std::uint32_t address))() {
    if (address != 0x43359C18u)
        throw std::out_of_range("Finalizer is not the recovered animation-backup cleanup entry");
    if (active_cleanup && active_cleanup != this)
        throw std::logic_error("Animation-backup exit callback requires one stable source binding");
    active_cleanup = this;
    return finalize_backup_map;
}

void bind_fix_animation_backup_exit_initializer(client::NativeStartupSources& sources,
                                                FixAnimationBackupCleanupContext& context) {
    sources.bind_initializer({0x433B86B4u, &context, append_backup_map_finalizer});
}
} // namespace skeet::animation

// ============================================================================
// Subcomponent: fix_animations.cpp
// ============================================================================

namespace skeet::animation {
namespace {
FixAnimationsRuntime* active_runtime;
/* deduplicated helper */
/* deduplicated helper */
template <class Result, class... Arguments>
Result original(void* object, std::size_t slot, Arguments... arguments) {
    const auto* metadata =
        static_cast<const engine::VtableHookMetadata*>((*static_cast<void***>(object))[-2]);
    return reinterpret_cast<engine::MemberFunction<Result, Arguments...>>(
        metadata->original_table[slot])(object, arguments...);
}
void original_six(void* object, std::size_t slot, const std::array<std::uintptr_t, 6>& a) {
    original<void>(object, slot, a[0], a[1], a[2], a[3], a[4], a[5]);
}
void* entity_from(void* receiver, std::size_t bias) noexcept {
    return receiver ? static_cast<std::byte*>(receiver) - bias : nullptr;
}
bool command_mode(const FixAnimationGlobals& globals) noexcept {
    return globals.command_mode || globals.alternate_command_mode;
}
bool interpolation_enabled(const void* variable) noexcept {
    for (;;) {
        const auto* parent = read<const void*>(variable, 0x1C);
        if (parent == variable)
            break;
        variable = parent;
    }
    const auto word = read<std::uint32_t>(variable, 0x2C) ^
                      std::uint32_t(reinterpret_cast<std::uintptr_t>(variable));
    return std::bit_cast<float>(word) != 0.0f;
}
std::int32_t subtract(std::int32_t a, std::int32_t b) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(a) - std::uint32_t(b));
}
std::int32_t add(std::int32_t a, std::int32_t b) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(a) + std::uint32_t(b));
}
std::int32_t truncate(float value) noexcept {
    return _mm_cvttss_si32(_mm_set_ss(value));
}
std::int32_t receive_tick(const FixAnimationsBindings& b) {
    return lagcomp::receive_time_to_tick(engine::invoke<float>(b.entity.engine, 14),
                                         b.globals.receive_interval);
}
// Complete4332FC35: max with explicit NaN and opposite-sign handling. This
// preserves the selected operand's signed zero and NaN payload.
float native_maximum(float first, float second) noexcept {
    if (std::isnan(first))
        return second;
    if (std::isnan(second))
        return first;
    if (std::signbit(first) != std::signbit(second))
        return std::signbit(first) ? second : first;
    return _mm_cvtss_f32(_mm_max_ss(_mm_set_ss(second), _mm_set_ss(first)));
}
void write_animation_frame(void* player, const FixAnimationsBindings& b) {
    write(player, b.fields.animation_frame_74019d87, b.globals.animation_frame);
}
void copy_absolute_angles_to_engine(void* player, const FixAnimationsBindings& b) {
    auto* result = engine::invoke<std::byte*>(player, 0x194 / 4);
    write(result, 0xC, read<RageVector>(player, b.bones.fields.angles_3b1f6ecc));
}
}

bool FixAnimationsRuntime::animation_owner_changed(void* player) const {
    const auto index = engine::invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
    const auto* owner = bindings.players.get(index);
    return read<std::uint8_t>(&owner->native, 0x17C) !=
           read<std::uint8_t>(player, bindings.fields.client_animation_8481ea04);
}

void FixAnimationsRuntime::build_transformations(void* player,
                                                 std::array<std::uintptr_t, 6> arguments) {
    if (bindings.globals.enabled) {
        const auto offset = bindings.bones.fields.setup_flags_bb78dc43;
        write(player, offset, read<std::uint32_t>(player, offset) | 4u);
    }
    original_six(player, 190, arguments);
}
void FixAnimationsRuntime::standard_blending_rules(void* player, void* studio,
                                                   RageVector* positions, BoneQuaternion* rotations,
                                                   float time) {
    if (bindings.globals.enabled) {
        const auto offset = bindings.bones.fields.setup_flags_bb78dc43;
        write(player, offset, read<std::uint32_t>(player, offset) | 4u);
    }
    original<void>(player, 207, studio, positions, rotations, time);
}
void FixAnimationsRuntime::extra_bone_processing(void* player,
                                                 std::array<std::uintptr_t, 6> arguments) {
    if (!bindings.globals.enabled)
        original_six(player, 198, arguments);
}

void FixAnimationsRuntime::calculate_view(void* player, RageVector* origin, RageVector* angles,
                                          float* near_plane, float* far_plane,
                                          float* field_of_view) {
    const auto& b = bindings;
    void** animation_slot = nullptr;
    void* saved_animation = nullptr;
    if (b.globals.enabled && (command_mode(b.globals) || b.globals.clear_animation_state)) {
        animation_slot = reinterpret_cast<void**>(static_cast<std::byte*>(player) +
                                                  b.bones.fields.animation_state_3d52a1a7);
        saved_animation = *animation_slot;
        *animation_slot = nullptr;
    }
    void* height_slot = nullptr;
    std::uint32_t saved_height = 0;
    const auto mode = read<std::int32_t>(player, b.fields.observer_mode_2f4e97c4);
    if (mode == 1 || mode == 2) {
        write(player, b.fields.observer_target_3b704b41 + 4, 76.0f);
        height_slot = static_cast<std::byte*>(player) + b.fields.view_offset_c246315e + 8;
        saved_height = read<std::uint32_t>(height_slot, 0);
        write(height_slot, 0, std::uint32_t(0));
    }
    original<void>(player, 277, origin, angles, near_plane, far_plane, field_of_view);
    const auto current_mode = read<std::int32_t>(player, b.fields.observer_mode_2f4e97c4);
    if ((current_mode == 1 || current_mode == 2) && engine::invoke<bool>(b.input, 32, -1)) {
        write(b.input, 0xAC, (*angles)[0]);
        write(b.input, 0xB0, (*angles)[1]);
    }
    if (height_slot)
        write(height_slot, 0, saved_height);
    if (animation_slot)
        *animation_slot = saved_animation;
}

bool FixAnimationsRuntime::setup_bones(void* renderable, Matrix3x4* output, std::int32_t maximum,
                                       std::uint32_t mask, float time) {
    const auto& b = bindings;
    auto* player = entity_from(renderable, 4);
    if ((b.globals.command_flags & 0x20u) && !output && maximum == -1 && mask == 0x200u)
        return true;
    if (b.globals.enabled) {
        write_animation_frame(player, b);
        write(player, b.bones.fields.setup_flags_bb78dc43, 4u);
    }
    if (b.globals.invalidate_bones && (mask & 0x100u)) {
        const auto* state = read<const void*>(player, b.bones.fields.animation_state_3d52a1a7);
        if (state) {
            const auto offset = b.entity.offsets.field_0471e13c;
            const auto counter = read<std::int32_t>(player, offset);
            if (counter && read<std::int32_t>(state, 0x70) > counter)
                write(player, offset, std::uint32_t(0));
        }
    }
    struct SavedField {
        void* destination = nullptr;
        std::uint32_t word;
    };
    std::array<SavedField, 5> fields;
    auto save = [&](std::size_t index, void* destination, std::uint32_t value) {
        fields[index] = {destination, read<std::uint32_t>(destination, 0)};
        write(destination, 0, value);
    };
    void* transient_slot = nullptr;
    std::uint32_t saved_transient = 0;
    const auto number = b.globals.last_outgoing;
    const auto history =
        prediction::CommandHistoryView::from_owner(b.globals.command_history_owner);
    if (const auto* state = history.find(number)) {
        auto* collision = engine::invoke<void*>(player, 3);
        auto* maximum_bounds = engine::invoke<RageVector*>(collision, 2);
        save(0, &(*maximum_bounds)[2], read<std::uint32_t>(state, 0x84));
        save(1, static_cast<std::byte*>(player) + b.entity.offsets.field_432b1f5b,
             read<std::uint32_t>(state, 0x7C));
        save(2, static_cast<std::byte*>(player) + b.entity.offsets.field_0d560fac,
             read<std::uint32_t>(state, 0x80));
        save(3, static_cast<std::byte*>(player) + b.fields.view_offset_c246315e + 8,
             read<std::uint32_t>(state, 0x28));
        const float clock = float(subtract(read<std::int32_t>(state, 0), 1)) * b.globals.interval;
        save(4, &b.globals.current_time, std::bit_cast<std::uint32_t>(clock));
        transient_slot = static_cast<std::byte*>(player) + b.fields.transient_6e9852d6;
        saved_transient = read<std::uint32_t>(transient_slot, 0);
        write(transient_slot, 0, read<std::uint32_t>(state, 0x78));
    }
    const auto replacement = b.globals.setup_angles;
    auto* eye_slot = static_cast<std::byte*>(player) + b.fields.eye_angles_c1b66ee8 + 4;
    const auto saved_eye = read<RageVector>(eye_slot, 0);
    write(eye_slot, 0, replacement);
    const auto result = original<bool>(renderable, 13, output, maximum, mask, time);
    write(eye_slot, 0, saved_eye);
    if (transient_slot)
        write(transient_slot, 0, saved_transient);
    for (auto cursor = fields.rbegin(); cursor != fields.rend(); ++cursor)
        if (cursor->destination)
            write(cursor->destination, 0, cursor->word);
    return result;
}

void FixAnimationsRuntime::restore_animation_backup(void* player) {
    const auto& b = bindings;
    const auto index = engine::invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
    auto** buckets = b.globals.backup_buckets;
    auto** bucket = buckets + index % b.globals.backup_bucket_count;
    auto* node = *bucket;
    while (node && node->entity_index != index)
        node = node->next;
    if (!node) {
        bucket = buckets + b.globals.backup_bucket_count;
        node = *bucket;
    }
    if (node == buckets[b.globals.backup_bucket_count])
        return;
    bool restore = false;
    if (b.globals.backup_mode == 2 && !read<std::uint8_t>(player, b.fields.life_state_9d920e35)) {
        const auto acknowledged = engine::invoke<std::int32_t>(b.entity.engine, 205);
        restore =
            node->acknowledged_command == acknowledged && node->frame == b.globals.prediction_frame;
    }
    if (restore) {
        if (auto* state = read<void*>(player, b.bones.fields.animation_state_3d52a1a7)) {
            const auto pointers = read<std::array<std::uint32_t, 4>>(state, 0x5C);
            std::memcpy(state, node->animation.data(), 0x348);
            write(state, 0x5C, pointers[0]);
            write(state, 0x68, pointers[3]);
            write(state, 0x64, pointers[2]);
            write(state, 0x60, pointers[1]);
        }
        std::memcpy(b.bones.pose_parameters(player).data(), node->poses.data(), sizeof node->poses);
        const auto current = read<RageVector>(player, b.bones.fields.angles_3b1f6ecc);
        if (node->angles[0] != current[0] || node->angles[1] != current[1] ||
            node->angles[2] != current[2]) {
            if (b.invalidate_physics)
                b.invalidate_physics(player, 2);
            write(player, b.bones.fields.angles_3b1f6ecc, node->angles);
        }
        engine::invoke<void>(player, 112);
        write(player, b.entity.offsets.field_0d560fac, node->collision_time);
    }
    if (*bucket == node)
        *bucket = node->next;
    else {
        auto* previous = *bucket;
        while (previous->next != node)
            previous = previous->next;
        previous->next = node->next;
    }
    release_animation_state_vector(node->animation.data(), b.allocator);
    std::uint32_t displacement;
    std::memcpy(&displacement, reinterpret_cast<std::byte*>(node) - 8, 4);
    b.memory.free(b.memory.process_heap, 0, reinterpret_cast<std::byte*>(node) - 16 - displacement);
    --b.globals.backup_count;
}

void FixAnimationsRuntime::networkable_pre_update(void* networkable, std::int32_t update) {
    const auto& b = bindings;
    auto* player = entity_from(networkable, 8);
    const auto index = engine::invoke<std::uint32_t>(networkable, 10);
    auto& owner = b.players.get(index)->native;
    client::capture_serialized_player_state(owner, player, b.data_update_fields);
    if (index == engine::invoke<std::uint32_t>(b.entity.engine, 12) &&
        read<std::uint8_t>(player, b.data_update_fields.local_serialized_d070684c))
        b.data_update_observers.capture_forward(player, update);
    // Original433E129A wraps the full433B7613 capture before engine slot6.
    original<void>(networkable, 6, update);
}

void FixAnimationsRuntime::networkable_view(void* networkable, std::int32_t update) {
    const auto& b = bindings;
    auto* player = entity_from(networkable, 8);
    const auto index = engine::invoke<std::uint32_t>(networkable, 10);
    auto& owner = b.players.get(index)->native;
    if (!update)
        restore_animation_backup(player);
    write(&owner, 0x118, read<std::uint32_t>(player, b.fields.body_yaw_a2a8247f));
    bool changed_view = false;
    RageVector saved_view;
    if (!engine::invoke<bool>(b.entity.engine, 82) && !engine::invoke<bool>(b.entity.engine, 93) &&
        interpolation_enabled(b.interpolation_cvar)) {
        b.data_update_observers.finish_reverse(player, update);
        const auto current = read<RageVector>(player, b.bones.fields.angles_3b1f6ecc);
        const auto& desired = b.globals.view_angles;
        if (current[0] != desired[0] || current[1] != desired[1] || current[2] != desired[2]) {
            if (b.invalidate_physics)
                b.invalidate_physics(player, 2);
            write(player, b.bones.fields.angles_3b1f6ecc, b.globals.view_angles);
        }
        engine::invoke<void>(b.entity.engine, 18, &saved_view);
        engine::invoke<void>(b.entity.engine, 19, &b.globals.view_angles);
        changed_view = true;
        if (b.globals.enabled) {
            write(player, b.bones.fields.setup_flags_bb78dc43, 4u);
            copy_absolute_angles_to_engine(player, b);
        }
    }
    original<void>(networkable, 7, update);
    if (changed_view)
        engine::invoke<void>(b.entity.engine, 19, &saved_view);
}

void FixAnimationsRuntime::thinkable_observers(void* thinkable) {
    const auto& b = bindings;
    auto* player = entity_from(thinkable, 0xC);
    b.globals.thinking = 1;
    if (b.globals.engine_timestamp) {
        const auto stamp = b.globals.timestamp();
        *b.globals.engine_timestamp = stamp;
    }
    if (b.globals.enabled) {
        write_animation_frame(player, b);
        write(player, b.bones.fields.setup_flags_bb78dc43, 4u);
    }
    original<void>(thinkable, 1);
    const auto observers = b.update_observers.entries();
    for (auto* item : observers) {
        auto* observer = static_cast<network::UpdateObserver*>(item);
        observer->vtable->post_view(observer, player);
    }
    b.globals.thinking = 0;
}
void FixAnimationsRuntime::thinkable_opacity(void* thinkable) {
    auto* player = entity_from(thinkable, 0xC);
    if (bindings.globals.enabled && animation_owner_changed(player))
        write_animation_frame(player, bindings);
    original<void>(thinkable, 1);
    effects::apply_player_opacity(player, bindings.opacity);
}

void FixAnimationsRuntime::receive_layer(const void* data, void* layer, void* output) {
    const auto& b = bindings;
    auto* player = read<void*>(layer, 0x30);
    if (player && engine::invoke<bool>(player, 158)) {
        const auto* begin =
            read<const std::byte*>(player, b.bones.fields.animation_layers_b538bba5);
        const auto layer_index = (static_cast<std::byte*>(layer) - begin) / 0x38;
        const auto index = engine::invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
        auto& owner = b.players.get(index)->native;
        write(&owner, 0x46C + layer_index * 0x1C, read<std::uint32_t>(data, 8));
        if (b.globals.local_player == player && command_mode(b.globals) &&
            !read<std::uint8_t>(b.globals.local_player, b.fields.life_state_9d920e35))
            return;
        auto* state = read<void*>(player, b.bones.fields.animation_state_3d52a1a7);
        std::uint8_t* slot = nullptr;
        std::uint8_t saved = 0;
        if (b.globals.enabled && state) {
            slot = static_cast<std::uint8_t*>(state) + 4;
            saved = *slot;
            *slot = 1;
        }
        b.original_layer_proxy(data, layer, output);
        if (slot)
            *slot = saved;
    } else
        b.original_layer_proxy(data, layer, output);
}

void FixAnimationsRuntime::networkable_history(void* networkable, std::int32_t update) {
    const auto& b = bindings;
    auto* player = entity_from(networkable, 8);
    const auto index = engine::invoke<std::uint32_t>(networkable, 10);
    auto& owner = b.players.get(index)->native;
    write(&owner, 0x118, read<std::uint32_t>(player, b.fields.body_yaw_a2a8247f));
    bool changed = false;
    for (std::size_t word = 0; word != 91; ++word) {
        if (read<std::uint32_t>(&owner, 0x2FC + word * 4) !=
            read<std::uint32_t>(&owner, 0x468 + word * 4)) {
            changed = true;
            break;
        }
    }
    const auto simulation_offset = b.fields.simulation_time_d5fd2e2c;
    write(player, simulation_offset, read<std::uint32_t>(&owner, 0x10C));
    const bool received_clock = b.history_rebuild && b.globals.local_player &&
                                player != b.globals.local_player;
    if (!read<std::uint8_t>(player, b.fields.life_state_9d920e35)) {
        const auto current = read<float>(player, simulation_offset);
        const auto previous = read<float>(player, b.fields.previous_time_1599541f);
        bool reconcile = !received_clock && current == read<float>(&owner, 0x198);
        if (!received_clock && !reconcile) {
            const auto again = engine::invoke<std::int32_t>(networkable, 10);
            if (lagcomp::time_to_tick(current, b.globals.interval) % 100 == 0) {
                const auto now = receive_tick(b);
                const auto offset = again % b.globals.clock_offset;
                auto old_bucket = subtract(read<std::int32_t>(&owner, 0x168), offset);
                auto new_bucket = subtract(now, offset);
                old_bucket = subtract(old_bucket, old_bucket % b.globals.clock_window);
                new_bucket = subtract(new_bucket, new_bucket % b.globals.clock_window);
                reconcile = old_bucket != new_bucket;
            }
        }
        if (reconcile) {
            // Complete43393C16, including each live comparison before reading
            // the interval. The animation-cycle alternative below is the
            // native correction when the grounded replicated-time test fails.
            bool replicated = false;
            if (read<float>(&owner, 0x524) != 0.0f &&
                (read<std::uint32_t>(player, b.entity.offsets.player_flags) & 1u)) {
                const auto value = read<float>(player, b.fields.replicated_time_d3d4230d);
                if (value != read<float>(&owner, 0x154) &&
                    std::fabs(value - previous) <= b.globals.interval * 16.0f)
                    replicated = true;
            }
            if (replicated) {
                write(&owner, 0x198, read<std::uint32_t>(player, simulation_offset));
                write(player, simulation_offset,
                      read<std::uint32_t>(player, b.fields.replicated_time_d3d4230d));
            } else {
                const auto new_cycle = read<float>(&owner, 0x5B4);
                auto old_cycle = read<float>(&owner, 0x448);
                const auto difference = new_cycle - old_cycle;
                const auto live_current = read<float>(player, simulation_offset);
                const auto span = truncate((live_current - previous) / b.globals.interval + 0.5f);
                std::int32_t ticks = span;
                if (difference != 0.0f && span != 1 && span != 2) {
                    auto rate = read<float>(&owner, 0x444);
                    if (difference < 0.0f) {
                        ticks = 0;
                        std::int32_t attempts = 0;
                        for (;;) {
                            ++ticks;
                            const float next = b.globals.interval * rate + old_cycle;
                            if (next >= 0.999969482421875f)
                                rate = read<float>(&owner, 0x5B0);
                            old_cycle = b.globals.interval * rate + old_cycle;
                            if (truncate(old_cycle) > 0)
                                break;
                            ++attempts;
                            if (attempts >= 16)
                                break;
                        }
                        if (attempts < 16) {
                            old_cycle -= float(truncate(old_cycle));
                            const float remaining =
                                (new_cycle - old_cycle) / read<float>(&owner, 0x5B0);
                            ticks = add(ticks, truncate(remaining / b.globals.interval + 0.5f));
                        }
                    } else
                        ticks = truncate((difference / rate) / b.globals.interval + 0.5f);
                    if (ticks < span) {
                        write(&owner, 0x198, live_current);
                        write(player, simulation_offset,
                              float(ticks) * b.globals.interval + previous);
                    }
                } else {
                    write(&owner, 0x198, live_current);
                    write(player, simulation_offset, previous);
                }
            }
        }
        if (changed) {
            write(&owner, 0x1D4, read<std::uint32_t>(&owner, 0x1D4) + 1u);
            const auto tick =
                lagcomp::time_to_tick(read<float>(player, simulation_offset), b.globals.interval);
            const auto stale = tick < subtract(receive_tick(b), 1);
            const auto old = read<std::uint32_t>(&owner, 0);
            write(&owner, 0, (old & ~8u) | (stale ? 8u : 0u));
            if (!received_clock) {
                const float minimum =
                    read<float>(player, simulation_offset) - b.globals.interval * 16.0f;
                write(player, b.fields.previous_time_1599541f,
                      native_maximum(read<float>(player, b.fields.previous_time_1599541f), minimum));
            }
        }
    } else {
        write(&owner, 0x1D4, std::uint32_t(0));
        write(&owner, 0, read<std::uint32_t>(&owner, 0) & ~8u);
    }
    write(&owner, 0x110, read<std::uint32_t>(player, simulation_offset));
    if (changed)
        write(&owner, 0x168, receive_tick(b));

    if (b.globals.command_mode && !read<std::uint8_t>(player, b.fields.life_state_9d920e35) &&
        !read<std::uint8_t>(player, b.fields.suppressed_cde290d2)) {
        write(player, b.fields.client_animation_8481ea04, std::uint8_t(0));
        if (b.globals.enabled && animation_owner_changed(player))
            write(player, b.bones.fields.setup_flags_bb78dc43, 4u);
    } else {
        write(player, b.fields.client_animation_8481ea04, read<std::uint8_t>(&owner, 0x17C));
        if (read<std::uint8_t>(player, b.fields.life_state_9d920e35) == 2 &&
            read<std::uint8_t>(player, b.fields.move_type_7501cb91) == 10) {
            const auto mode = read<std::int32_t>(player, b.fields.observer_mode_2f4e97c4);
            if (mode == 4 || mode == 5) {
                const auto handle = read<std::uint32_t>(player, b.fields.observer_target_3b704b41);
                const auto target_index = handle & 0xFFFFu;
                if (handle != UINT32_MAX && target_index - 1u <= 63u) {
                    auto* target = engine::invoke<void*>(b.entity.entity_list, 4, handle);
                    if (target && engine::invoke<bool>(target, 158) &&
                        engine::invoke<bool>(static_cast<std::byte*>(target) + 8, 9) &&
                        b.resource && *b.resource &&
                        read<std::uint8_t>(*b.resource,
                                           b.fields.resource_alive_d266b537 + target_index)) {
                        const auto source =
                            read<RageVector>(player, b.bones.fields.bone_origin_a9ee6062);
                        auto& geometry = b.players.get(target_index)->geometry;
                        geometry.origin = {source[0], source[1], source[2]};
                        geometry.center = geometry.origin;
                        const auto* minimum = engine::invoke<const RageVector*>(target, 76);
                        const auto* maximum = engine::invoke<const RageVector*>(target, 77);
                        geometry.center.z += ((*maximum)[2] + (*minimum)[2]) * 0.5f;
                        geometry.eye_height =
                            read<float>(target, b.entity.offsets.pose_parameters + 4) * 18.0f +
                            58.0f;
                        geometry.pose_adjustment =
                            read<float>(target, b.entity.offsets.pose_parameters + 0x30) * 6.0f;
                        const auto second = *engine::invoke<const RageVector*>(target, 77);
                        geometry.lower = {geometry.center.x, geometry.center.y,
                                          geometry.center.z - second[2] * 0.6f};
                        geometry.upper = {geometry.center.x, geometry.center.y,
                                          (geometry.origin.z + geometry.eye_height) -
                                              geometry.pose_adjustment};
                        geometry.alpha = 1.0f;
                        geometry.last_live_tick = receive_tick(b);
                    }
                }
            }
        }
    }
    std::array<std::byte, 0x158> information;
    if (engine::invoke<bool>(b.entity.engine, 8, index, information.data())) {
        auto* settings = b.player_settings + std::size_t(index) * 0x20;
        const auto identity = read<std::uint32_t>(information.data(), 8);
        if (read<std::uint32_t>(settings, 0) != identity) {
            write(settings, 0, identity);
            std::memset(settings + 8, 0, 0x18);
        }
    }
    if (b.globals.enabled && animation_owner_changed(player)) {
        copy_absolute_angles_to_engine(player, b);
        const auto offset = b.fields.simulation_time_d5fd2e2c;
        write(player, offset - 4, read<std::uint32_t>(player, offset - 8));
    }
    const auto current = read<float>(player, b.fields.simulation_time_d5fd2e2c);
    float temporary = current;
    if (current - read<float>(player, b.fields.previous_time_1599541f) != 0.0f) {
        const auto interval = b.globals.interval;
        const float minimum = engine::invoke<float>(b.entity.engine, 14) - interval;
        if (minimum > read<float>(player, b.fields.simulation_time_d5fd2e2c)) {
            const auto clock = engine::invoke<float>(b.entity.engine, 14);
            temporary = clock + (interval + interval);
        }
    }
    auto* simulation = static_cast<std::byte*>(player) + b.fields.simulation_time_d5fd2e2c;
    const auto saved = read<std::uint32_t>(simulation, 0);
    write(simulation, 0, temporary);
    original<void>(networkable, 7, update);
    if (!update)
        restore_animation_backup(player);
    (void)read<std::uint32_t>(player, b.bones.fields.effects_38ca5228);
    write(simulation, 0, saved);
}

#if defined(_M_IX86) || defined(__i386__)
#define SKEET_FIX_ENTRY __fastcall
#else
#define SKEET_FIX_ENTRY
#endif
namespace {
void SKEET_FIX_ENTRY transformations_entry(void* player, void*, std::uintptr_t a, std::uintptr_t b,
                                           std::uintptr_t c, std::uintptr_t d, std::uintptr_t e,
                                           std::uintptr_t f) {
    active_runtime->build_transformations(player, {a, b, c, d, e, f});
}
void SKEET_FIX_ENTRY blending_entry(void* player, void*, void* studio, RageVector* positions,
                                    BoneQuaternion* rotations, float time) {
    active_runtime->standard_blending_rules(player, studio, positions, rotations, time);
}
void SKEET_FIX_ENTRY extra_bones_entry(void* player, void*, std::uintptr_t a, std::uintptr_t b,
                                       std::uintptr_t c, std::uintptr_t d, std::uintptr_t e,
                                       std::uintptr_t f) {
    active_runtime->extra_bone_processing(player, {a, b, c, d, e, f});
}
void SKEET_FIX_ENTRY view_entry(void* player, void*, RageVector* origin, RageVector* angles,
                                float* near_plane, float* far_plane, float* field_of_view) {
    active_runtime->calculate_view(player, origin, angles, near_plane, far_plane, field_of_view);
}
bool SKEET_FIX_ENTRY bones_entry(void* renderable, void*, Matrix3x4* output, std::int32_t maximum,
                                 std::uint32_t mask, float time) {
    return active_runtime->setup_bones(renderable, output, maximum, mask, time);
}
void SKEET_FIX_ENTRY history_entry(void* networkable, void*, std::int32_t update) {
    active_runtime->networkable_history(networkable, update);
}
void SKEET_FIX_ENTRY network_view_entry(void* networkable, void*, std::int32_t update) {
    active_runtime->networkable_view(networkable, update);
}
void SKEET_FIX_ENTRY network_pre_update_entry(void* networkable, void*, std::int32_t update) {
    active_runtime->networkable_pre_update(networkable, update);
}
void SKEET_FIX_ENTRY think_observers_entry(void* thinkable, void*) {
    active_runtime->thinkable_observers(thinkable);
}
void SKEET_FIX_ENTRY think_opacity_entry(void* thinkable, void*) {
    active_runtime->thinkable_opacity(thinkable);
}
void layer_proxy_entry(const void* data, void* layer, void* output) {
    active_runtime->receive_layer(data, layer, output);
}
}
void FixAnimationsRuntime::prepare(FixAnimationHook kind,
                                   engine::VtableHookMetadata& metadata) noexcept {
    active_runtime = this;
    switch (kind) {
    case FixAnimationHook::player:
        metadata.replacement_table[190] = reinterpret_cast<void*>(&transformations_entry);
        metadata.replacement_table[207] = reinterpret_cast<void*>(&blending_entry);
        metadata.replacement_table[198] = reinterpret_cast<void*>(&extra_bones_entry);
        metadata.replacement_table[277] = reinterpret_cast<void*>(&view_entry);
        break;
    case FixAnimationHook::renderable:
        metadata.replacement_table[13] = reinterpret_cast<void*>(&bones_entry);
        break;
    case FixAnimationHook::networkable_history:
        metadata.replacement_table[7] = reinterpret_cast<void*>(&history_entry);
        break;
    case FixAnimationHook::networkable_view:
        metadata.replacement_table[6] = reinterpret_cast<void*>(&network_pre_update_entry);
        metadata.replacement_table[7] = reinterpret_cast<void*>(&network_view_entry);
        break;
    case FixAnimationHook::thinkable_observers:
        metadata.replacement_table[1] = reinterpret_cast<void*>(&think_observers_entry);
        break;
    case FixAnimationHook::thinkable_opacity:
        metadata.replacement_table[1] = reinterpret_cast<void*>(&think_opacity_entry);
        break;
    }
}
engine::VtableHookNode* FixAnimationsRuntime::install(FixAnimationHook kind,
                                                      engine::VtableHookRegistry& hooks,
                                                      void* receiver,
                                                      engine::VtableHookMetadata& metadata) {
    prepare(kind, metadata);
    return engine::install_vtable_hook(hooks, receiver, metadata);
}
void* FixAnimationsRuntime::native_replacement_entry(std::size_t, std::uint32_t entry) {
    void* replacement;
    switch (entry) {
    case 0x43330F73u:
        replacement = reinterpret_cast<void*>(&transformations_entry);
        break;
    case 0x4335C639u:
        replacement = reinterpret_cast<void*>(&blending_entry);
        break;
    case 0x43388201u:
        replacement = reinterpret_cast<void*>(&extra_bones_entry);
        break;
    case 0x4335B798u:
        replacement = reinterpret_cast<void*>(&view_entry);
        break;
    case 0x433E80F7u:
        replacement = reinterpret_cast<void*>(&bones_entry);
        break;
    case 0x4337DE0Bu:
        replacement = reinterpret_cast<void*>(&history_entry);
        break;
    case 0x433E129Au:
        replacement = reinterpret_cast<void*>(&network_pre_update_entry);
        break;
    case 0x433B7093u:
        replacement = reinterpret_cast<void*>(&network_view_entry);
        break;
    case 0x433FD3BFu:
        replacement = reinterpret_cast<void*>(&think_observers_entry);
        break;
    case 0x43383CA2u:
        replacement = reinterpret_cast<void*>(&think_opacity_entry);
        break;
    default:
        throw std::invalid_argument("Unknown captured FixAnimations entry");
    }
    return replacement;
}
void FixAnimationsRuntime::bind_native_replacement(engine::VtableHookMetadata& metadata,
                                                   std::size_t slot, std::uint32_t entry) {
    auto* replacement = native_replacement_entry(slot, entry);
    active_runtime = this;
    metadata.replacement_table[slot] = replacement;
}
void FixAnimationsRuntime::uninstall(engine::VtableHookRegistry& hooks, void* receiver) {
    engine::uninstall_vtable_hook(hooks, receiver);
}
void FixAnimationsRuntime::install_layer_proxy(void** slot) {
    active_runtime = this;
    proxy_slot_ = slot;
    saved_proxy_ = *slot;
    bindings.original_layer_proxy = reinterpret_cast<AnimationLayerProxy>(saved_proxy_);
    std::atomic_ref<void*>(*slot).exchange(reinterpret_cast<void*>(&layer_proxy_entry),
                                           std::memory_order_seq_cst);
}
void FixAnimationsRuntime::restore_layer_proxy() {
    if (proxy_slot_)
        std::atomic_ref<void*>(*proxy_slot_).exchange(saved_proxy_, std::memory_order_seq_cst);
    proxy_slot_ = nullptr;
}
#undef SKEET_FIX_ENTRY
} // namespace skeet::animation

// ============================================================================
// Subcomponent: fix_runtime_bindings.cpp
// ============================================================================
#if defined(_WIN32) && (defined(_M_IX86) || defined(__i386__))
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#endif

namespace skeet::animation {
namespace {
std::ptrdiff_t offset(const engine::NativeOffsetGlobals& globals, std::uint32_t pointer,
                     std::uint32_t key, std::uint32_t salt) {
    return engine::DecodedOffset(globals, pointer, key, salt, 0);
}
}
FixAnimationFields native_fix_animation_fields(const engine::NativeOffsetGlobals& globals) {
    // Exact operands in4337DE0B,433E80F7,4335B798,433C7FC5 and433CE396.
    // Previous-time1599541F is the derived simulation+4 slot, distinct from
    // the replicatedD3D4230D slot. The eye-angle consumer applies its own+4.
    return {offset(globals, 0x43468E78u, 0x434684E4u, 0x9D920E35u),
            offset(globals, 0x43469198u, 0x4346A538u, 0xD5FD2E2Cu),
            offset(globals, 0x43467004u, 0x4346A6B4u, 0x1599541Fu),
            offset(globals, 0x43467230u, 0x43467100u, 0xD3D4230Du),
            offset(globals, 0x43468FECu, 0x4346A3C4u, 0xA2A8247Fu),
            offset(globals, 0x43468DBCu, 0x434689C8u, 0x8481EA04u),
            offset(globals, 0x4346A5DCu, 0x43468430u, 0xCDE290D2u),
            offset(globals, 0x43468BD4u, 0x4346A678u, 0x7501CB91u),
            offset(globals, 0x4346A6CCu, 0x43468EF8u, 0x2F4E97C4u),
            offset(globals, 0x43468F84u, 0x43467E64u, 0x3B704B41u),
            offset(globals, 0x43468A34u, 0x43468C00u, 0xC246315Eu),
            offset(globals, 0x43467BA4u, 0x43468E1Cu, 0x74019D87u),
            offset(globals, 0x43468F88u, 0x4346A6D8u, 0x6E9852D6u),
            offset(globals, 0x43467A60u, 0x43468BA4u, 0xC1B66EE8u),
            offset(globals, 0x4346A724u, 0x4346857Cu, 0xD266B537u)};
}
FixAnimationEntityFields
native_fix_animation_entity_fields(const engine::NativeOffsetGlobals& globals) {
    return {offset(globals, 0x434688B4u, 0x4346A7D0u, 0x0471E13Cu),
            offset(globals, 0x43467EBCu, 0x43467038u, 0x432B1F5Bu),
            offset(globals, 0x43467A88u, 0x43467160u, 0x0D560FACu),
            offset(globals, 0x434688D0u, 0x43468C24u, 0xE2FBE695u),
            offset(globals, 0x43467D78u, 0x43467B80u, 0x1CAAF0A8u)};
}
AnimationCeiling native_animation_ceiling() {
#if defined(_WIN32) && (defined(_M_IX86) || defined(__i386__))
    static const auto entry = [] {
        const auto module = GetModuleHandleW(L"msvcrt.dll");
        if (!module)
            throw std::runtime_error("Animation backup growth requires the loaded msvcrt.dll");
        const auto function = reinterpret_cast<AnimationCeiling>(GetProcAddress(module, "ceil"));
        if (!function)
            throw std::runtime_error("Loaded msvcrt.dll lacks the native ceil export");
        return function;
    }();
    return entry;
#else
    throw std::logic_error("Native animation ceiling requires Windows x86");
#endif
}
namespace {
client::ClientRuntimeStorage& checked(RuntimeOwner& bones,
                                      client::ConfigurationRuntimeOwner& settings,
                                      const effects::PlayerOpacityBindings& opacity) {
    auto& storage = bones.storage();
    auto& data = storage.data();
    if (&settings.storage() != &storage ||
        &settings.settings_menu() != &storage.settings_menu_storage())
        throw std::invalid_argument(
            "Fix animation owners must share the canonical storage, settings and hook list");
    auto* const globals = data.native_cell<std::byte*>(0x43467B04u);
    if (!globals || data.native_cell<void*>(0x434678F8u) != globals + 4 ||
        data.native_cell<void*>(0x43467EB8u) != globals + 0x10 ||
        data.native_cell<void*>(0x4346A338u) != globals + 0x20 ||
        !data.native_cell<std::int32_t*>(0x43475B2Cu) || !data.native_cell<void*>(0x43467204u) ||
        !data.native_cell<double (*)()>(0x43468C84u) || !data.native_cell<double*>(0x4346A7E4u) ||
        !data.native_cell<void**>(0x43468FF0u) ||
        !data.native_cell<engine::MemberFunction<void, std::int32_t>>(0x43468C10u))
        throw std::invalid_argument(
            "Fix animation requires published globals, client-state, tier0 and entity animation bindings");
    if (!data.native_cell<FixAnimationBackupNode**>(0x4346D6E4u) ||
        !data.native_cell<std::uint32_t>(0x4346D6E8u))
        throw std::invalid_argument("Fix animation requires its initialized canonical backup map");
    if (opacity.input != data.native_cell<void*>(0x4346A894u) ||
        &opacity.settings != &settings.colored_model_settings() ||
        opacity.entities.engine != data.native_cell<void*>(0x4346A514u) ||
        opacity.entities.entity_list != data.native_cell<void*>(0x4346A890u) ||
        &opacity.entities.local_player != &data.native_cell<void*>(0x43475808u) ||
        &opacity.active_effects != &data.native_cell<effects::EffectsCommandOwner*>(0x43475A1Cu) ||
        &opacity.smoke_registry != &data.native_cell<void*>(0x434679C0u) ||
        &opacity.smoke_clock != &data.native_cell<const std::int32_t*>(0x43468D4Cu) ||
        &opacity.interval != reinterpret_cast<float*>(globals + 0x20) ||
        &opacity.camera_position != &data.native_cell<RageVector>(0x43479CCCu) ||
        &opacity.suspended != &data.native_cell<std::uint8_t>(0x43468E58u) ||
        opacity.player_adjustments.settings_base() != &data.native_cell<std::uint8_t>(0x4347BE08u))
        throw std::invalid_argument(
            "Fix animation opacity must borrow its actual canonical renderer and player storage");
    return storage;
}
FixAnimationGlobals globals(client::ClientRuntimeStorage& storage,
                            client::ConfigurationRuntimeOwner& settings) {
    auto& data = storage.data();
    auto* const timing = data.native_cell<std::byte*>(0x43467B04u);
    return {settings.settings_menu().fix_animations,
            settings.rage_settings().global.enabled.value,
            settings.anti_aim_global().enabled.value,
            settings.movement().infinite_duck.value,
            settings.effects_settings().third_person_alive.value,
            data.native_cell<std::uint32_t>(0x434759C0u),
            *data.native_cell<std::int32_t*>(0x434678F8u),
            *reinterpret_cast<std::int32_t*>(timing + 4),
            *reinterpret_cast<std::int32_t*>(timing + 0x38),
            *reinterpret_cast<std::int32_t*>(timing + 0x3C),
            *data.native_cell<float*>(0x4346A338u),
            *reinterpret_cast<float*>(timing + 0x20),
            *data.native_cell<float*>(0x43467EB8u),
            *data.native_cell<std::int32_t*>(0x43475B2Cu),
            data.native_cell<client::EntityCommandOwner*>(0x43475898u),
            data.native_cell<void*>(0x43475808u),
            data.native_cell<RageVector>(0x43476414u),
            data.native_cell<RageVector>(0x4347BD30u),
            data.native_cell<std::uint8_t>(0x43475BD1u),
            data.native_cell<double*>(0x4346A7E4u),
            data.native_cell<double (*)()>(0x43468C84u),
            data.native_cell<FixAnimationBackupNode**>(0x4346D6E4u),
            data.native_cell<std::uint32_t>(0x4346D6E8u),
            data.native_cell<std::uint32_t>(0x4346D6ECu),
            data.native_cell<std::int32_t>(0x4347C9ECu)};
}
const void* interpolation(const engine::NativeDataImage& data) {
    const auto address = data.native_cell<std::uint32_t>(0x4346A850u) ^ 0x478720C2u;
    if (!address)
        throw std::invalid_argument("Fix animation requires the discovered cl_interpolate ConVar");
    return reinterpret_cast<const void*>(std::uintptr_t(address));
}
}
FixAnimationsBindings
native_fix_animations_bindings(RuntimeOwner& bones, client::ConfigurationRuntimeOwner& settings,
                               const client::PlayerDataUpdateOffsets& updates,
                               const effects::PlayerOpacityBindings& opacity) {
    auto& storage = checked(bones, settings, opacity);
    auto& data = storage.data();
    return {bones.bones(),
            {data.native_cell<void*>(0x4346A514u), data.native_cell<void*>(0x4346A890u),
             native_fix_animation_entity_fields(storage.offsets().globals)},
            bones.players(),
            native_fix_animation_fields(storage.offsets().globals),
            globals(storage, settings),
            data.native_cell<void*>(0x4346A894u),
            interpolation(data),
            data.native_cell<void**>(0x43468FF0u),
            &data.native_cell<std::byte>(0x4347BE00u),
            storage.data_update_observers(),
            updates,
            storage.owner_registries().at(0x43475988u),
            data.native_cell<engine::MemberFunction<void, std::int32_t>>(0x43468C10u),
            data.native_cell<void*>(0x43467204u),
            storage.memory(),
            data.native_cell<AnimationLayerProxy>(0x43476974u),
            opacity,
            settings.rage_settings().global.experimental(rage::ExperimentalRageOption::animation)};
}
FixAnimationBackupBindings
native_fix_animation_backup_bindings(RuntimeOwner& bones, const FixAnimationsBindings& bindings) {
    auto& data = bones.storage().data();
    if (&bindings.bones != &bones.bones() || &bindings.players != &bones.players() ||
        &bindings.globals.backup_buckets !=
            &data.native_cell<FixAnimationBackupNode**>(0x4346D6E4u))
        throw std::invalid_argument(
            "Fix backup runtime must share its actual source bone owner and backup map");
    return {data.native_cell<FixAnimationBackupNode**>(0x4346D6E4u),
            data.native_cell<std::uint32_t>(0x4346D6E8u),
            data.native_cell<std::uint32_t>(0x4346D6ECu),
            data.native_cell<FixAnimationBackupGrowth>(0x4346D6F0u),
            bindings.globals.backup_mode,
            bindings.globals.prediction_frame,
            bones.bones(),
            bindings.entity,
            bindings.fields.life_state_9d920e35,
            bones.storage().memory(),
            native_animation_ceiling()};
}
} //namespace skeet::animation

// ============================================================================
// Subcomponent: fix_runtime_owner.cpp
// ============================================================================

namespace skeet::animation {
namespace {
client::ClientRuntimeStorage& checked_hooks(RuntimeOwner& bones, engine::HookRuntimeOwner& hooks) {
    auto& storage = bones.storage();
    const auto memory = storage.memory();
    if (&hooks.storage() != &storage || &hooks.registry().sentinel != &hooks.unload_state().hooks ||
        hooks.registry().memory.allocate != memory.allocate ||
        hooks.registry().memory.free != memory.free ||
        hooks.registry().memory.process_heap != memory.process_heap)
        throw std::invalid_argument(
            "Fix animation hooks must share the actual canonical hook list and process heap");
    return storage;
}
}
struct FixRuntimeOwner::Data {
    RuntimeOwner& bones;
    client::ClientRuntimeStorage& storage;
    client::PlayerDataUpdateOffsets data_update_fields;
    FixAnimationsBindings animation_bindings;
    FixAnimationsRuntime animations;
    FixAnimationBackupBindings backup_bindings;
    FixAnimationBackupDetachBindings detach_bindings;
    FixAnimationBackupRuntime backup;
    FixAnimationBackupCleanupContext& cleanup;

    Data(RuntimeOwner& bone_owner, client::ConfigurationRuntimeOwner& settings,
         engine::HookRuntimeOwner& hooks, const effects::PlayerOpacityBindings& opacity,
         FixAnimationBackupCleanupContext& early_cleanup)
        : bones(bone_owner), storage(checked_hooks(bones, hooks)),
          data_update_fields(client::native_player_data_update_offsets(storage.offsets().globals)),
          animation_bindings(
              native_fix_animations_bindings(bones, settings, data_update_fields, opacity)),
          animations(animation_bindings),
          backup_bindings(native_fix_animation_backup_bindings(bones, animation_bindings)),
          detach_bindings{hooks.registry(),
                          storage.data().native_cell<std::ptrdiff_t>(0x43479CE4u),
                          animation_bindings.player_settings,
                          {storage.data().native_cell<std::int32_t>(0x4346DB1Cu),
                           storage.data().native_cell<std::uint32_t>(0x4346DB2Cu),
                           storage.data().native_cell<std::uint32_t>(0x4346DB30u),
                           storage.data().native_cell<std::uint32_t>(0x4346DB38u)}},
          backup(backup_bindings, detach_bindings), cleanup(early_cleanup) {
        if (&cleanup.callbacks != &storage.exit_callbacks() ||
            &cleanup.allocator != &storage.data().native_cell<void*>(0x43467204u))
            throw std::invalid_argument(
                "Fix runtime requires the canonical early animation cleanup context");
        cleanup.require_producer(backup);
    }
};
FixRuntimeOwner::FixRuntimeOwner(RuntimeOwner& bones, client::ConfigurationRuntimeOwner& settings,
                                 engine::HookRuntimeOwner& hooks,
                                 const effects::PlayerOpacityBindings& opacity,
                                 FixAnimationBackupCleanupContext& cleanup)
    : data_(std::make_unique<Data>(bones, settings, hooks, opacity, cleanup)) {}
FixRuntimeOwner::~FixRuntimeOwner() = default;
client::ClientRuntimeStorage& FixRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
RuntimeOwner& FixRuntimeOwner::bones_owner() const noexcept {
    return data_->bones;
}
FixAnimationsRuntime& FixRuntimeOwner::animations() noexcept {
    return data_->animations;
}
FixAnimationBackupRuntime& FixRuntimeOwner::backup() noexcept {
    return data_->backup;
}
FixAnimationBackupCleanupContext& FixRuntimeOwner::cleanup() noexcept {
    return data_->cleanup;
}
void FixRuntimeOwner::bind_initializers(client::NativeStartupSources& sources) {
    bind_fix_animation_backup_exit_initializer(sources, data_->cleanup);
}
} //namespace skeet::animation

// ============================================================================
// Subcomponent: layer_receive.cpp
// ============================================================================

namespace skeet::animation {
namespace {
template <class T> T read(const void* object, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof(value));
    return value;
}
template <class T> void write(void* object, std::size_t offset, T value) {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(value));
}
struct Field {
    std::uint32_t original;
    std::size_t cache;
    bool capture_early;
};
constexpr std::array<Field, 7> fields{{{0x43476AB4u, 0x480, true},
                                       {0x43476A84u, 0x470, true},
                                       {0x43476974u, 0x46C, false},
                                       {0x43476B04u, 0x474, false},
                                       {0x43476A24u, 0x47C, false},
                                       {0x43476984u, 0x478, false},
                                       {0x434769A4u, 0x468, false}}};
}
std::uint32_t LayerReceiveRuntime::offset(std::uint32_t pointer, std::uint32_t key,
                                          std::uint32_t salt) const {
    return *offsets_.globals.pointers.at(pointer) ^ offsets_.globals.keys.at(key) ^ salt;
}
void LayerReceiveRuntime::receive(ReceivedLayerField field, const engine::RecvProxyData* packet,
                                  void* layer, void* output) {
    if (field == ReceivedLayerField::sequence) {
        animations_.receive_layer(packet, layer, output);
        return;
    }
    const auto& description = fields.at(static_cast<unsigned>(field));
    auto* player = read<void*>(layer, 0x30);
    if (player && engine::invoke<bool>(player, 158)) {
        const auto begin =
            read<std::uintptr_t>(player, offset(0x4346A660u, 0x434679E0u, 0xB538BBA5u));
        const auto difference =
            std::uint32_t(reinterpret_cast<std::uintptr_t>(layer)) - std::uint32_t(begin);
        const auto layer_index = std::bit_cast<std::int32_t>(difference) / 0x38;
        auto value = description.capture_early ? read<std::uint32_t>(packet, 8) : 0u;
        const auto id = engine::invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
        auto* owner = animations_.bindings.players.get(id);
        if (!owner)
            throw std::bad_alloc();
        if (!description.capture_early)
            value = read<std::uint32_t>(packet, 8);
        write(&owner->native, description.cache + layer_index * 0x1C, value);
        const auto& globals = animations_.bindings.globals;
        if (globals.local_player == player &&
            (globals.command_mode || globals.alternate_command_mode) &&
            !read<std::uint8_t>(globals.local_player,
                                offset(0x43468E78u, 0x434684E4u, 0x9D920E35u)))
            return;
        if (field == ReceivedLayerField::weight && layer_index == 6 &&
            read<std::uint8_t>(player, offset(0x43468DBCu, 0x434689C8u, 0x8481EA04u)) !=
                read<std::uint8_t>(&owner->native, 0x17C))
            return;
    }
    data_.native_cell<engine::RecvVarProxy>(description.original)(packet, layer, output);
}
} //namespace skeet::animation

// ============================================================================
// Subcomponent: layer_state.cpp
// ============================================================================

namespace skeet::animation {
namespace {
template <class T> T field(const void* layer, std::ptrdiff_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(layer) + offset, sizeof value);
    return value;
}
}

std::uint8_t layer_is_active(const void* layer) noexcept {
    if (field<std::int32_t>(layer, 0x04) <= 0)
        return 0;

    // ANDPS at 433CC624 clears the sign bit without converting the float.
    const auto rate_bits = field<std::uint32_t>(layer, 0x14) & 0x7fffffffu;
    float absolute_rate;
    std::memcpy(&absolute_rate, &rate_bits, sizeof absolute_rate);
    // COMISS/JBE rejects zero and unordered operands, including either NaN.
    if (!(absolute_rate > 0.0f))
        return 0;

    // 4347089C contains 3F7FFE00: 1 - 2^-15, exactly representable in float.
    // COMISS/JAE rejects ordered >=; unordered cycle values reach MOV AL,1.
    constexpr float completed_cycle = 0x1.fffcp-1f;
    return static_cast<std::uint8_t>(!(field<float>(layer, 0x18) >= completed_cycle));
}

} // namespace skeet::animation

// ============================================================================
// Subcomponent: local_layers.cpp
// ============================================================================

namespace skeet::animation {
namespace {
/* deduplicated helper */
template <class T> void store(void* object, std::ptrdiff_t offset, const T& value) noexcept {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(value));
}
void* layer_five(void* player, const LocalAnimationBindings& bindings) noexcept {
    const auto begin = field<std::uintptr_t>(player, bindings.offsets.layers);
    return reinterpret_cast<void*>(begin + 0x118U);
}
void reset_layer(void* player, void* layer, std::int32_t sequence) {
    store(layer, 0x18, sequence);
    store(layer, 0x2C, std::uint32_t(0));
    store(layer, 0x20, std::uint32_t(0));
    const auto rate = engine::invoke<float>(player, 223, layer, sequence);
    store(layer, 0x28, rate);
}
}
void mark_animation_modifiers(void* state) noexcept {
    const bool moving = field<float>(state, 0xF8) > 0.25f;
    std::uint8_t flags = std::uint8_t(moving);
    if (field<float>(state, 0xA4) > 0.550000011920928955078125f)
        flags = std::uint8_t(moving) | 2U;
    store(state, 0x32B, flags);
}
float read_local_layer_weight(void* state, std::int32_t layer,
                              const LocalAnimationBindings& bindings) {
    return bindings.layer_weight.read(bindings.layer_weight.context, state, layer);
}
void update_landing_layer(void* player, void* state, const LocalAnimationBindings& bindings) {
    const auto* descriptor = static_cast<const std::byte*>(player) + bindings.offsets.layers;
    if (field<std::int32_t>(descriptor, 0xC) <= 5)
        return;
    const auto previous = field<std::uint8_t>(state, 0x108);
    const bool ground = (field<std::uint8_t>(player, bindings.offsets.player_flags) & 1U) ||
                        field<std::uint8_t>(player, bindings.offsets.additional_ground);
    const bool transitioned =
        !field<std::uint8_t>(state, 4) && previous != std::uint8_t(ground) && ground;
    if (!ground || field<std::uint8_t>(state, 0x109) || !transitioned)
        return;
    const bool long_air = field<float>(state, 0x110) > 1.0f;
    auto sequence = select_symbol_weighted_sequence(player, 5, long_air ? 0x3DD : 0x3DC, bindings);
    void* layer;
    if (sequence > 1) {
        layer = layer_five(player, bindings);
        if (!layer)
            return;
    } else {
        std::array<char, 64> name;
        ::strcpy_s(name.data(), name.size(),
                   client::prediction_history_sequence_string(*bindings.sequence_owner, 5));
        ::strcat_s(
            name.data(), name.size(),
            client::prediction_history_sequence_string(*bindings.sequence_owner, long_air ? 6 : 7));
        const auto modifiers = field<std::uint8_t>(state, 0x32B);
        if (modifiers & 2U)
            ::strcat_s(name.data(), name.size(),
                       client::prediction_history_sequence_string(*bindings.sequence_owner, 1));
        if (!long_air && (modifiers & 1U))
            ::strcat_s(name.data(), name.size(),
                       client::prediction_history_sequence_string(*bindings.sequence_owner, 0));
        sequence = map_animation_activity(player, name.data(), bindings);
        if (sequence <= 1)
            return;
        layer = layer_five(player, bindings);
    }
    reset_layer(player, layer, sequence);
}
void update_post_animation_layer(void* player, void* state, bool previous_in_air,
                                 float unused_previous_time,
                                 const LocalAnimationBindings& bindings) {
    (void)unused_previous_time;
    const auto* descriptor = static_cast<const std::byte*>(player) + bindings.offsets.layers;
    if (field<std::int32_t>(descriptor, 0xC) <= 5)
        return;
    if (field<std::uint8_t>(state, 0x108)) {
        if (previous_in_air) {
            const auto* layer = layer_five(player, bindings);
            bool clear = field<float>(layer, 0x20) == 0.0f;
            if (!clear) {
                const auto sequence = field<std::int32_t>(layer, 0x18);
                clear = (bindings.sequence_activity ? bindings.sequence_activity(player, sequence)
                                                    : -1) != 0x3DB;
            }
            if (clear)
                store(state, 0x10B, std::uint8_t(0));
        }
        if (!field<std::uint8_t>(state, 0x109) && !field<std::uint8_t>(state, 0x10B) &&
            field<float>(state, 0x12C) <= 0.0f)
            store(field<void*>(descriptor, 0), 0x138, std::uint32_t(0));
        return;
    }
    if (field<std::uint8_t>(state, 0x128) || !field<std::uint8_t>(state, 0x121) ||
        field<std::uint8_t>(state, 0x10B))
        return;
    const auto sequence = select_symbol_weighted_sequence(player, 5, 0x3DA, bindings);
    if (sequence <= 1)
        return;
    auto* layer = layer_five(player, bindings);
    if (layer)
        reset_layer(player, layer, sequence);
}
} // namespace skeet::animation

// ============================================================================
// Subcomponent: local_movement.cpp
// ============================================================================

namespace skeet::animation {
namespace {
float absolute(float value) noexcept {
    return std::bit_cast<float>(std::bit_cast<std::uint32_t>(value) & 0x7FFFFFFFU);
}
float normalize(float value) noexcept {
    if (std::isnan(value))
        return 0.0f;
    if (absolute(value) > 180.0f) {
        const float correction = round_half_away_from_zero(absolute(value / 360.0f)) * 360.0f;
        value = value < 0.0f ? value + correction : value - correction;
    }
    return value;
}
}
std::uint32_t movement_button_state(std::uint32_t buttons) noexcept {
    const bool forward = (buttons & 8U) != 0, backward = (buttons & 16U) != 0;
    const bool left = (buttons & 0x400U) != 0, right = (buttons & 0x200U) != 0;
    if (!(buttons & 0x1618U))
        return 0;
    if (forward && backward) {
        if ((left && right) || (!left && !right))
            return 0;
    } else if (left && right) {
        if (!forward && !backward)
            return 0;
    }
    return buttons & 0x20000U ? 1U : 2U;
}
void apply_movement_button_state(void* player, const client::Command& command,
                                 const LocalAnimationBindings& bindings) noexcept {
    auto* output = static_cast<std::byte*>(player) + bindings.offsets.movement_state;
    const std::uint32_t zero = 0;
    std::memcpy(output, &zero, sizeof(zero));
    const auto result = movement_button_state(command.movement.buttons);
    if (result)
        std::memcpy(output, &result, sizeof(result));
}
void update_lower_body_yaw_timer(float yaw, float speed, float vertical, float now, float target,
                                 float& next_update, float& previous_yaw) noexcept {
    const float normalized = normalize(yaw);
    float delay;
    if (!(speed <= 0.1f) || !(absolute(vertical) <= 100.0f)) {
        delay = std::bit_cast<float>(0x3E6147AEU);
    } else {
        if (next_update - now >= 0.0f)
            return;
        float delta = target - normalized;
        if (std::isnan(delta))
            return;
        if (absolute(delta) > 180.0f) {
            const float correction = round_half_away_from_zero(absolute(delta / 360.0f)) * 360.0f;
            delta = delta < 0.0f ? delta + correction : delta - correction;
        }
        if (delta < 0.0f)
            delta = std::bit_cast<float>(std::bit_cast<std::uint32_t>(delta) ^ 0x80000000U);
        if (delta <= 35.0f)
            return;
        delay = std::bit_cast<float>(0x3F8CCCCDU);
    }
    next_update = delay + now;
    if (previous_yaw != normalized)
        previous_yaw = normalized;
}
} // namespace skeet::animation

// ============================================================================
// Subcomponent: local_prediction_pose.cpp
// ============================================================================

namespace skeet::animation {
namespace {
/* deduplicated helper */
MovementVector movement_vector(const RageVector& vector) noexcept {
    return {vector[0], vector[1], vector[2]};
}
const client::Command& prediction_command(const client::Command& current, std::uint32_t ordinal,
                                          const LocalPredictionCommandRing& commands) noexcept {
    if (!(ordinal & 1U)) {
        const auto previous = std::uint32_t(current.number) - 1U;
        const auto& command = commands[previous % 150U];
        if (std::uint32_t(command.number) == previous && command.tick != 0x7F7FFFFF)
            return command;
    }
    return current;
}
}
void publish_pose(client::CommandFrame& frame, void* player, const client::Command& command,
                  const LocalPredictionPoseBindings& bindings,
                  LocalPredictionPoseInputs& workspace) {
    auto& scratch = workspace.animation;
    initialize_animation_state_vector(scratch.data());
    lagcomp::AnimationState saved;
    capture_entity_animation(saved, player, bindings.entity.offsets);
    const float saved_yaw = engine::invoke<const RageVector*>(player, 11)->at(1);
    auto* const original_state = field<void*>(player, bindings.entity.offsets.animation_state);
    if (frame.field_1d4 == 1U) {
        if (original_state)
            std::memcpy(scratch.data(), original_state, scratch.size());
        store(player, bindings.entity.offsets.animation_state, static_cast<void*>(scratch.data()));
        const float forward = command.movement.forward_move, side = command.movement.side_move;
        const float square = forward * forward + side * side;
        // Immutable434724A8=10.0f and4346DD40=4.0f.
        if ((command.tick != 0x7F7FFFFF && square > 10.0f) || frame.scalar_c0 > 4.0f) {
            const auto mean = lagcomp::mean_history_timing_ticks(bindings.latency);
            const auto corrected = lagcomp::corrected_command_tick(bindings.entity.command_tick,
                                                                   bindings.entity.previous_command,
                                                                   bindings.entity.current_command);
            const auto mode = std::uint32_t(add(corrected, add(mean, 1)));
            const auto flags = field<std::uint32_t>(player, bindings.offsets.player_flags_e2fbe695);
            const auto velocity = field<RageVector>(player, bindings.offsets.velocity_36375c23);
            const auto origin = *engine::invoke<const RageVector*>(player, 10);
            MovementSimulation simulation;
            auto environment = bindings.movement_environment;
            environment.interval = bindings.interval;
            const anti_aim::MovementCaptureInput movement{bindings.movement_capture.offsets,
                                                          bindings.movement_capture.convars,
                                                          *bindings.interval, &environment};
            initialize_live_movement_simulation(simulation, player, movement_vector(origin),
                                                movement_vector(velocity), flags, mode, movement);
            for (std::uint32_t ordinal = 0; ordinal < 3; ++ordinal) {
                const auto& selected = prediction_command(command, ordinal, bindings.commands);
                auto* const clock = bindings.current_time;
                const float saved_clock = *clock;
                *clock = float(std::bit_cast<std::int32_t>(simulation.mode)) * *bindings.interval;
                environment.interval = bindings.interval;
                simulate_live_movement_tick(simulation, bindings.collision, environment, player,
                                            selected);
                const auto saved_velocity =
                    field<RageVector>(player, bindings.offsets.velocity_36375c23);
                store(player, bindings.offsets.velocity_36375c23, simulation.velocity);
                const auto saved_duck =
                    field<std::uint32_t>(player, bindings.offsets.duck_3309c0cf);
                store(player, bindings.offsets.duck_3309c0cf, simulation.duck_amount);
                const auto saved_flags =
                    field<std::uint32_t>(player, bindings.offsets.player_flags_e2fbe695);
                store(player, bindings.offsets.player_flags_e2fbe695, simulation.player_flags);
                const auto saved_command =
                    field<std::uint32_t>(player, bindings.offsets.command_f58e6118);
                store(player, bindings.offsets.command_f58e6118,
                      std::uint32_t(reinterpret_cast<std::uintptr_t>(&selected)));
                const auto saved_origin =
                    field<RageVector>(player, bindings.offsets.origin_4e66bb34);
                bindings.animation_update_flag = 1;
                store(player, bindings.offsets.origin_4e66bb34, simulation.position);
                const auto saved_absolute =
                    field<RageVector>(player, bindings.offsets.origin_2a671b42);
                store(player, bindings.offsets.origin_2a671b42, simulation.position);
                const auto saved_body =
                    field<std::uint32_t>(player, bindings.offsets.body_yaw_a2a8247f);
                store(player, bindings.offsets.body_yaw_a2a8247f, selected.angles[1]);
                const AnimationUpdateTiming timing{*bindings.animation_frame,
                                                   *bindings.current_time, *bindings.interval,
                                                   nullptr, &bindings.animation_reset};
                const auto animation_engine =
                    lagcomp::native_history_animation_engine(bindings.animation_update);
                update_animation_state(scratch.data(), selected.angles, timing, animation_engine);
                store(player, bindings.offsets.body_yaw_a2a8247f, saved_body);
                store(player, bindings.offsets.origin_2a671b42, saved_absolute[0]);
                bindings.animation_update_flag = 0;
                store(player, bindings.offsets.origin_2a671b42 + 4U, saved_absolute[1]);
                store(player, bindings.offsets.origin_2a671b42 + 8U, saved_absolute[2]);
                store(player, bindings.offsets.origin_4e66bb34, saved_origin);
                store(player, bindings.offsets.command_f58e6118, saved_command);
                store(player, bindings.offsets.player_flags_e2fbe695, saved_flags);
                store(player, bindings.offsets.duck_3309c0cf, saved_duck);
                store(player, bindings.offsets.velocity_36375c23, saved_velocity);
                *clock = saved_clock; //4331F1E3 reloads its captured pointer, not43467EB8
            }
        }
    }
    const auto angles = *engine::invoke<const RageVector*>(player, 11);
    const BoneRequest request{angles, frame.origin_10c, 0.0f, 0x300U, -1, 1, false};
    std::int32_t bone_count = 256;
    if (bindings.bones.build_bone_matrices(player, request, workspace.bones, &bone_count)) {
        bindings.output.timestamp = *bindings.publication_clock;
        bindings.bones.select_hitbox_bone_matrices(player, workspace.bones,
                                                   bindings.output.hitboxes);
        bindings.output.origin = frame.origin_10c;
    }
    restore_entity_animation(saved, player, bindings.entity.offsets);
    if (bindings.set_absolute_angles) {
        const RageVector restored{0.0f, saved_yaw, 0.0f};
        bindings.set_absolute_angles(player, restored);
    }
    store(player, bindings.entity.offsets.animation_state, original_state);
    release_animation_state_vector(scratch.data(), bindings.allocator);
}
void publish_local_prediction_pose(client::CommandFrame& frame, void* player,
                                   const client::Command& command, std::uint8_t send_packet,
                                   const LocalPredictionPoseBindings& bindings) {
    if (!send_packet && frame.choked_15c < frame.choke_limit_fc)
        return;
    //4331EE04 reserves these locals itself. Only the five vector words are
    // initialized unconditionally; mode1 copies348 bytes when state exists.
    // The native null-state branch retains its own unwritten stack bytes.
    // Bone matrices are output of43388720, never bootstrap input.
    LocalPredictionPoseInputs workspace;
    publish_pose(frame, player, command, bindings, workspace);
}
void publish_local_prediction_pose_with_inputs(client::CommandFrame& frame, void* player,
                                               const client::Command& command,
                                               std::uint8_t send_packet,
                                               const LocalPredictionPoseBindings& bindings,
                                               const LocalPredictionPoseInputs& incoming) {
    if (!send_packet && frame.choked_15c < frame.choke_limit_fc)
        return;
    LocalPredictionPoseInputs workspace;
    std::memcpy(&workspace, &incoming, sizeof workspace);
    publish_pose(frame, player, command, bindings, workspace);
}
} // namespace skeet::animation

// ============================================================================
// Subcomponent: local_sequence.cpp
// ============================================================================

namespace skeet::animation {
namespace {
/* deduplicated helper */
std::uint16_t symbol(const char* text, const LocalAnimationBindings& bindings) {
    std::uint16_t value = 0xFFFFU;
    if (bindings.find_symbol)
        bindings.find_symbol(bindings.symbol_table, &value, text);
    if (value == 0xFFFFU) {
        value = 0xFFFFU;
        if (bindings.add_symbol)
            bindings.add_symbol(bindings.symbol_table, &value, text);
    }
    return value;
}
std::array<std::uint32_t, 8> activity_context(void* player,
                                              const LocalAnimationBindings& bindings) {
    std::array<std::uint32_t, 8> words;
    words[0] = std::uint32_t(reinterpret_cast<std::uintptr_t>(bindings.activity_interface));
    std::memcpy(words.data() + 1, bindings.activity_scratch.data(),
                sizeof(bindings.activity_scratch));
    words[7] = std::uint32_t(reinterpret_cast<std::uintptr_t>(player));
    return words;
}
}
const char* weapon_sequence_prefix(void* weapon, const LocalAnimationBindings& bindings) {
    std::int32_t index = 0;
    if (weapon) {
        const auto kind = engine::invoke<std::int32_t>(weapon, 455);
        const auto identifier = field<std::uint16_t>(weapon, bindings.offsets.weapon_identifier);
        if (identifier == 37)
            return client::prediction_history_sequence_string(*bindings.sequence_owner, 4);
        if (identifier == 27)
            index = 3;
        else if (identifier == 31)
            index = 1;
        else if (kind == 11 || identifier == 70 || identifier == 85)
            index = 9;
        else if (identifier == 72 || identifier == 20)
            index = 7;
        else if (kind != 16 && kind >= 0)
            index = kind;
    }
    if (index > 10)
        index = 10;
    return client::prediction_history_sequence_string(*bindings.sequence_owner,
                                                      11 + std::size_t(index));
}
std::int32_t select_symbol_weighted_sequence(void* player, std::int32_t layer,
                                             std::int32_t activity,
                                             const LocalAnimationBindings& bindings) {
    const auto* descriptor = static_cast<const std::byte*>(player) + bindings.offsets.layers;
    const auto begin = field<std::uintptr_t>(descriptor, 0);
    if (!(begin + std::uintptr_t(std::uint32_t(layer) * 0x38U)))
        return -1;
    auto* studio = field<void*>(player, bindings.offsets.studio_header);
    if (!studio || !field<void*>(studio, 0) || !bindings.initialize_mapping ||
        !bindings.select_symbol_sequence || !bindings.add_symbol || !bindings.find_symbol ||
        !bindings.symbol_table)
        return -1;
    auto* mapping = field<void*>(studio, 0x58);
    if (!mapping) {
        mapping = bindings.initialize_mapping(studio);
        store(studio, 0x58, mapping);
        if (!mapping)
            return -1;
    }
    auto* animation = field<void*>(player, bindings.offsets.animation_state);
    std::array<std::uint16_t, 4> symbols;
    std::int32_t count = 0;
    const auto append = [&](const char* text) {
        const auto value = symbol(text, bindings);
        if (value != 0xFFFFU)
            symbols[std::size_t(count++)] = value;
    };
    const auto handle = field<std::uint32_t>(player, bindings.offsets.active_weapon);
    if (handle != 0xFFFFFFFFU) {
        if (auto* weapon = engine::invoke<void*>(bindings.entity_list, 4, handle))
            append(weapon_sequence_prefix(weapon, bindings));
    }
    const auto modifiers = field<std::uint8_t>(animation, 0x32B);
    if (modifiers & 1U)
        append("moving");
    if (modifiers & 2U)
        append("crouch");
    if (!count)
        return -1;
    return bindings.select_symbol_sequence(mapping, studio, activity, symbols.data(), count);
}
std::int32_t map_animation_activity(void* player, const char* name,
                                    const LocalAnimationBindings& bindings) {
    if (!bindings.activity_interface)
        return -1;
    const auto context = activity_context(player, bindings);
    using Map = std::int32_t (*)(const std::uint32_t*, const char*);
    const auto function = field<Map>(bindings.activity_interface, 0x64);
    return function(context.data(), name);
}
std::int32_t select_prediction_sequence(void* player, std::int32_t activity, std::int32_t seed,
                                        const LocalAnimationBindings& bindings) {
    if (!bindings.activity_interface)
        return -1;
    std::int32_t previous_seed = 0;
    if (bindings.prediction_seed) {
        previous_seed = *bindings.prediction_seed;
        *bindings.prediction_seed = seed;
    }
    void* previous_player = nullptr;
    const bool published = bindings.prediction_player && bindings.local_player;
    if (published) {
        previous_player = *bindings.prediction_player;
        *bindings.prediction_player = bindings.local_player;
    }
    auto context = activity_context(player, bindings);
    const auto function = field<engine::MemberFunction<std::int32_t, std::int32_t>>(
        bindings.activity_interface, 0x5C);
    const auto result = function(context.data(), activity);
    if (published)
        *bindings.prediction_player = previous_player;
    if (bindings.prediction_seed)
        *bindings.prediction_seed = previous_seed;
    return result;
}
} // namespace skeet::animation

// ============================================================================
// Subcomponent: native_trigonometry.cpp
// ============================================================================
#if defined(_WIN32) && (defined(_M_IX86) || defined(__i386__))
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::animation {
namespace {
void* named_entry(const char* name) {
    const auto module = GetModuleHandleW(L"msvcrt.dll");
    if (!module)
        throw std::runtime_error("Native SSE math requires the loaded msvcrt.dll");
    auto* const entry = reinterpret_cast<void*>(GetProcAddress(module, name));
    if (!entry)
        throw std::runtime_error("Loaded msvcrt.dll lacks a required native SSE math export");
    return entry;
}
struct Entries {
    void* sine;
    void* cosine;
};
const Entries& entries() {
    static const Entries result = [] {
        const auto module = GetModuleHandleW(L"msvcrt.dll");
        if (!module)
            throw std::runtime_error("Native trigonometry requires the loaded msvcrt.dll");
        auto* const sine = reinterpret_cast<void*>(GetProcAddress(module, "__libm_sse2_sinf"));
        auto* const cosine = reinterpret_cast<void*>(GetProcAddress(module, "__libm_sse2_cosf"));
        if (!sine || !cosine)
            throw std::runtime_error("Loaded msvcrt.dll lacks the native SSE trigonometry exports");
        return Entries{sine, cosine};
    }();
    return result;
}
float invoke(void* entry, float value) {
    register __m128 lanes __asm__("xmm0") = _mm_set_ss(value);
    __asm__ __volatile__("call *%1"
                         : "+x"(lanes)
                         : "r"(entry)
                         : "eax", "ecx", "edx", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6",
                           "xmm7", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)",
                           "st(7)", "memory", "cc");
    return _mm_cvtss_f32(lanes);
}
float sine(float value) {
    return invoke(entries().sine, value);
}
float cosine(float value) {
    return invoke(entries().cosine, value);
}
}
const RageTrigonometry& native_sse_trigonometry() {
    (void)entries();
    static const RageTrigonometry result{sine, cosine};
    return result;
}
double native_sse_atan2(double y, double x) {
    static auto* const entry = named_entry("__libm_sse2_atan2");
    register __m128d first __asm__("xmm0") = _mm_set_sd(y);
    register __m128d second __asm__("xmm1") = _mm_set_sd(x);
    __asm__ __volatile__("call *%2"
                         : "+x"(first), "+x"(second)
                         : "r"(entry)
                         : "eax", "ecx", "edx", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                           "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)",
                           "memory", "cc");
    return _mm_cvtsd_f64(first);
}
float native_sse_exponential(float value) {
    static auto* const entry = named_entry("__libm_sse2_expf");
    return invoke(entry, value);
}
float native_sse_tangent(float value) {
    static auto* const entry = named_entry("__libm_sse2_tanf");
    return invoke(entry, value);
}
float native_sse_arctangent(float value) {
    static auto* const entry = named_entry("__libm_sse2_atanf");
    return invoke(entry, value);
}
} //namespace skeet::animation
#else
namespace skeet::animation {
const RageTrigonometry& native_sse_trigonometry() {
    throw std::logic_error("Native SSE trigonometry requires Windows x86");
}
double native_sse_atan2(double, double) {
    throw std::logic_error("Native SSE atan2 requires Windows x86");
}
float native_sse_exponential(float) {
    throw std::logic_error("Native SSE exponential requires Windows x86");
}
float native_sse_tangent(float) {
    throw std::logic_error("Native SSE tangent requires Windows x86");
}
float native_sse_arctangent(float) {
    throw std::logic_error("Native SSE arctangent requires Windows x86");
}
}
#endif

// ============================================================================
// Subcomponent: player_interpolation.cpp
// ============================================================================

namespace skeet::client {
void PlayerPhysicsHooks::update_interpolated_state(void* player,
                                                   std::array<std::uint32_t, 4> arguments) {
    const auto index = engine::invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
    const auto& cache = bindings.players.get(index)->native;
    if (player_physics_detail::field<std::uint32_t>(&cache, 0x1D4) == 0)
        return;
    player_physics_detail::original<void>(player, 78, arguments[0], arguments[1], arguments[2],
                                          arguments[3]);
}
std::uint8_t PlayerPhysicsHooks::interpolate(void* player, std::uint32_t time_bits) {
    if (auto* guard = bindings.interpolation_guard_43468fa4)
        *guard = 0;
    // On x86 this is the native float argument's unchanged four-byte stack
    // representation. No x87 load/store may quiet a signaling NaN here.
    return player_physics_detail::original<std::uint8_t>(player, 113, time_bits);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: remote_animation_preparation.cpp
// ============================================================================

namespace skeet::animation {
namespace {
/* deduplicated helper */
/* deduplicated helper */
void* from_handle(void* entities, std::uint32_t handle) {
    return handle == 0xffffffffu ? nullptr : engine::invoke<void*>(entities, 4, handle);
}
}
RemoteAnimationPreparationFields::RemoteAnimationPreparationFields(
    const engine::NativeOffsetGlobals& data)
    : frame(data, 0x43467BA4, 0x43468E1C, 0x74019D87, 0),
      ik_mask(data, 0x43467CD8, 0x43468DD8, 0x01D0BACC, 0),
      flags(data, 0x43467634, 0x43468F2C, 0xBB78DC43, 0),
      previous_flags(data, 0x43468ED0, 0x4346A394, 0xABD16CE2, 0),
      active_weapon(data, 0x43467A74, 0x434678C0, 0xCD812654, 0),
      animation_state(data, 0x43467F4C, 0x4346A8B0, 0x3D52A1A7, 0),
      view_gate(data, 0x43468BF8, 0x43467B8C, 0xD070684C, 0),
      observer_mode(data, 0x4346A6CC, 0x43468EF8, 0x2F4E97C4, 0),
      observer_target(data, 0x43468F84, 0x43467E64, 0x3B704B41, 0),
      world_model(data, 0x43467180, 0x43467FA4, 0xCA2B5FFB, 0),
      eye_origin(data, 0x434670CC, 0x43468BCC, 0xA9EE6062, 0),
      duck_amount(data, 0x434679F8, 0x434675F8, 0x3309C0CF, 0) {}

bool RemoteAnimationPreparationRuntime::allows_view_animation(void* player) const {
    //433BFF50's opaque arithmetic selects player slot363 and input slot32.
    return engine::invoke<std::int32_t>(player, 363, 0) == 2 ||
           engine::invoke<std::int32_t>(bindings.input, 32, -1) != 0;
}
void RemoteAnimationPreparationRuntime::prepare(void* player) const {
    const auto& b = bindings;
    if (engine::invoke<bool>(b.engine, 93))
        return;
    const auto& f = b.fields;
    if (b.frame == read<std::int32_t>(player, f.frame))
        return;
    write(player, f.ik_mask, 0xffffffffu);
    bool force_animation = read<std::int32_t>(player, f.frame) == 0;
    auto* const weapon = from_handle(b.entity_list, read<std::uint32_t>(player, f.active_weapon));
    auto* const animation = read<void*>(player, f.animation_state);
    if (animation && weapon && read<void*>(animation, 0x5c) != weapon) {
        force_animation = true;
        write(animation, 0x5c, weapon);
    }
    write(player, f.frame, b.frame);
    write(player, f.previous_flags, read<std::uint32_t>(player, f.flags));
    write(player, f.flags, 4u);

    auto* const networkable = static_cast<std::byte*>(player) + 8;
    auto* const renderable = static_cast<std::byte*>(player) + 4;
    if (!force_animation && !engine::invoke<bool>(networkable, 9) &&
        engine::invoke<bool>(renderable, 3) &&
        (!read<std::uint8_t>(player, f.view_gate) || allows_view_animation(player))) {
        auto* const local = b.local_player;
        const bool observing =
            local && read<std::int32_t>(local, f.observer_mode) == 4 &&
            from_handle(b.entity_list, read<std::uint32_t>(local, f.observer_target)) == player;
        if (!observing) {
            const auto eye = rage::target_eye_position(
                rage::TargetEyePositionOffsets{f.eye_origin, f.duck_amount}, player);
            const float x = eye[0] - b.view_origin[0];
            const float y = eye[1] - b.view_origin[1];
            const float z = eye[2] - b.view_origin[2];
            if (std::sqrt((x * x + y * y) + z * z) > 400.0f) {
                write(player, f.flags, read<std::uint32_t>(player, f.flags) | 1u);
                constexpr float radians = 0.01745329238474369f;
                const float yaw = b.view_angles[1] * radians;
                const float yaw_sine = b.trigonometry.sine(yaw);
                const float yaw_cosine = b.trigonometry.cosine(yaw);
                const float pitch = b.view_angles[0] * radians;
                const float pitch_sine = b.trigonometry.sine(pitch);
                const float pitch_cosine = b.trigonometry.cosine(pitch);
                const float forward_x = pitch_cosine * yaw_cosine;
                const float forward_y = pitch_cosine * yaw_sine;
                const float forward_z = -pitch_sine;
                constexpr float epsilon = 0x1p-23f;
                const float inverse_forward =
                    1.0f / (std::sqrt((forward_x * forward_x + forward_y * forward_y) +
                                      forward_z * forward_z) +
                            epsilon);
                const float inverse_distance =
                    1.0f / (std::sqrt((x * x + y * y) + z * z) + epsilon);
                const float cosine = ((y * inverse_distance) * (inverse_forward * forward_y) +
                                      (x * inverse_distance) * (inverse_forward * forward_x)) +
                                     (z * inverse_distance) * (inverse_forward * forward_z);
                // COMISS/JAE falls through for unordered operands as well.
                if (!(cosine >= -0.258819043636322f))
                    write(player, f.flags, read<std::uint32_t>(player, f.flags) | 2u);
            }
        }
    }

    if (weapon) {
        if (auto* const world =
                from_handle(b.entity_list, read<std::uint32_t>(weapon, f.world_model))) {
            write(world, f.flags, read<std::uint32_t>(player, f.flags));
            write(world, f.previous_flags, read<std::uint32_t>(player, f.previous_flags));
        }
    }
    if (!force_animation) {
        const auto flags = read<std::uint32_t>(player, f.flags);
        if ((flags & 1u) && !((read<std::uint32_t>(player, f.previous_flags) ^ flags) & 1u))
            write(player, f.ik_mask, 0x300u);
        if (!engine::invoke<bool>(networkable, 9) ||
            (read<std::uint32_t>(player, f.previous_flags) & 8u))
            return;
    }
    write(player, f.ik_mask, 0xffffffffu);
}
} // namespace skeet::animation

// ============================================================================
// Subcomponent: remote_setup_bones.cpp
// ============================================================================

namespace skeet::animation {
namespace {
RemoteSetupBonesRuntime* active_remote_runtime;
/* deduplicated helper */
/* deduplicated helper */
void copy_vector(RageVector& destination, const RageVector& source) noexcept {
    // Three sequential MOVSD instructions, including their alias behavior.
    for (std::size_t i = 0; i < 3; ++i)
        write(&destination, i * 4, read<std::uint32_t>(&source, i * 4));
}
void copy_bones(Matrix3x4* destination, const Matrix3x4* source, std::int32_t count) noexcept {
    // All three 16-byte rows are loaded before the first store for each bone.
    // A whole-range memcpy/memmove would change native overlap behavior.
    for (std::int32_t i = 0; i < count; ++i) {
        const auto first = _mm_loadu_ps(source[i].rows[0]);
        const auto second = _mm_loadu_ps(source[i].rows[1]);
        const auto third = _mm_loadu_ps(source[i].rows[2]);
        _mm_storeu_ps(destination[i].rows[0], first);
        _mm_storeu_ps(destination[i].rows[1], second);
        _mm_storeu_ps(destination[i].rows[2], third);
    }
}
}

std::uint8_t& native_remote_bone_thread_guard() noexcept {
    return *reinterpret_cast<std::uint8_t*>(std::uintptr_t(__readfsdword(0x18)) + 0x1B8);
}

void RemoteSetupBonesRuntime::refresh_attachments(void* player) const {
    const auto& b = bindings;
    if (!b.update_attachments)
        return;
    const auto flags = b.bones.fields.entity_flags_83a6cf2b;
    write(player, flags, read<std::uint32_t>(player, flags) | 8u);
    const auto saved_time = read<std::uint32_t>(&b.frame_time, 0);
    write(&b.frame_time, 0, std::uint32_t(0));
    const auto update = b.update_attachments;
    auto* wrapper = read<void*>(player, b.bones.fields.studio_wrapper_32e4daf1);
    if (wrapper && !read<void*>(wrapper, 0))
        wrapper = nullptr;
    update(player, wrapper);
    auto handle = read<std::uint32_t>(player, b.fields.transform_guard_49d93e62 - 0x14);
    while (handle != UINT32_MAX) {
        auto* linked = engine::invoke<void*>(b.bones.entity_list, 4, handle);
        if (!linked)
            break;
        write(linked, flags, read<std::uint32_t>(linked, flags) | 0x800u);
        engine::invoke<void>(linked, 10);
        handle = read<std::uint32_t>(linked, b.fields.transform_guard_49d93e62 - 0x10);
    }
    write(player, flags, read<std::uint32_t>(player, flags) & ~8u);
    write(&b.frame_time, 0, saved_time);
}

bool RemoteSetupBonesRuntime::setup_bones(void* renderable, Matrix3x4* output, std::int32_t maximum,
                                          std::uint32_t mask, float /*time*/) {
    const auto& b = bindings;
    auto* player = static_cast<std::byte*>(renderable) - 4;
    if (read<std::int32_t>(player, b.bones.fields.model_index_a25d8271) == -1)
        return false;
    const auto* wrapper = read<const void*>(player, b.bones.fields.studio_wrapper_32e4daf1);
    if (!wrapper)
        return false;
    const auto* header = read<const void*>(wrapper, 0);
    if (!header)
        return false;
    if (read<std::int32_t>(header, 0x150) && !read<const void*>(wrapper, 4))
        return false;
    if (b.prepare_on_engine_thread) {
        const auto predicate = b.on_engine_thread;
        if (predicate && predicate())
            b.prepare_on_engine_thread(player);
    }
    if (!output && maximum == -1 && mask == 0x200u)
        return (read<std::uint32_t>(player, b.bones.fields.bone_accessor_54783141 + 8) & 0x200u) !=
               0;

    const auto index = engine::invoke<std::uint32_t>(player + 8, 10);
    auto& state = b.players.get(index)->native;
    const auto counter = b.current_bone_counter;
    if (!counter || read<std::int32_t>(player, b.fields.recent_bone_counter_2b554ad0) != *counter) {
        const auto requested_frame = read<std::int32_t>(&state, 0x168);
        if (read<std::int32_t>(&state, 0xE0) != requested_frame) {
            if (!b.thread_guard()) {
                b.thread_guard() = 1;
                b.preparation.prepare(player);
                auto* origin = engine::invoke<RageVector*>(renderable, 1);
                auto& cached_origin =
                    *reinterpret_cast<RageVector*>(reinterpret_cast<std::byte*>(&state) + 0xD4);
                copy_vector(cached_origin, *origin);
                write(&state, 0xE0, requested_frame);
                auto* pitch = player + b.fields.view_angles_e23a9be1;
                const auto saved_pitch = read<std::uint32_t>(pitch, 0);
                write(pitch, 0, read<std::uint32_t>(&state, 0x1A8));
                const auto saved_yaw = read<std::uint32_t>(pitch, 4);
                write(pitch, 4, read<std::uint32_t>(&state, 0x1AC));
                std::int32_t bone_count =
                    read<std::int32_t>(player, b.fields.bone_cache_17978090 + 0xC);
                const auto options =
                    read<std::uint32_t>(player, b.bones.fields.setup_flags_bb78dc43);
                auto* bones = read<Matrix3x4*>(player, b.fields.bone_cache_17978090);
                const auto angles = *engine::invoke<RageVector*>(renderable, 2);
                const auto build_origin = *engine::invoke<RageVector*>(renderable, 1);
                const BoneRequest request{angles, build_origin, 0.0f, 0xFFF00u, -1, options, true};
                const auto succeeded = b.bones.build_bone_matrices(
                    player, request, {bones, maximum_bones}, &bone_count);
                if (!succeeded) {
                    b.thread_guard() = 0;
                    write(&state, 0xE0, std::int32_t(-1));
                    write(pitch, 4, saved_yaw);
                    write(pitch, 0, saved_pitch);
                    return false;
                }
                refresh_attachments(player);
                b.thread_guard() = 0;
                write(player, b.bones.fields.bone_accessor_54783141 + 8, 0xFFF00u);
                write(player, b.bones.fields.bone_accessor_54783141 + 0xC, 0xFFF00u);
                if (b.current_bone_counter)
                    write(player, b.fields.recent_bone_counter_2b554ad0, *b.current_bone_counter);
                write(pitch, 4, saved_yaw);
                write(pitch, 0, saved_pitch);
            }
        } else {
            if (counter)
                write(player, b.fields.recent_bone_counter_2b554ad0, *counter);
            const auto* origin = engine::invoke<RageVector*>(renderable, 1);
            auto& cached_origin =
                *reinterpret_cast<RageVector*>(reinterpret_cast<std::byte*>(&state) + 0xD4);
            const float x = (*origin)[0] - cached_origin[0];
            const float y = (*origin)[1] - cached_origin[1];
            const float z = (*origin)[2] - cached_origin[2];
            const float distance = (y * y + x * x) + z * z;
            if (distance > 0.00009999999747378752f) {
                auto* bones = read<Matrix3x4*>(player, b.fields.bone_cache_17978090);
                const auto count = read<std::int32_t>(player, b.fields.bone_cache_17978090 + 0xC);
                for (std::int32_t i = 0; i < count; ++i) {
                    bones[i].rows[0][3] = bones[i].rows[0][3] + x;
                    bones[i].rows[1][3] = y + bones[i].rows[1][3];
                    bones[i].rows[2][3] = bones[i].rows[2][3] + z;
                }
                copy_vector(cached_origin, *origin);
                refresh_attachments(player);
            }
        }
    }
    if (output) {
        const auto count = read<std::int32_t>(player, b.fields.bone_cache_17978090 + 0xC);
        if (maximum < count)
            return false;
        const auto* bones = read<const Matrix3x4*>(player, b.fields.bone_cache_17978090);
        copy_bones(output, bones, count);
    }
    return true;
}

#if defined(_M_IX86) || defined(__i386__)
#define SKEET_REMOTE_BONES_ENTRY __fastcall
#else
#define SKEET_REMOTE_BONES_ENTRY
#endif
namespace {
bool SKEET_REMOTE_BONES_ENTRY setup_bones_entry(void* renderable, void*, Matrix3x4* output,
                                                std::int32_t maximum, std::uint32_t mask,
                                                float time) {
    return active_remote_runtime->setup_bones(renderable, output, maximum, mask, time);
}
}
void RemoteSetupBonesRuntime::bind(engine::VtableHookMetadata& descriptor) noexcept {
    active_remote_runtime = this;
    descriptor.replacement_table[13] = source_entry();
}
void* RemoteSetupBonesRuntime::source_entry() noexcept {
    return reinterpret_cast<void*>(&setup_bones_entry);
}
#undef SKEET_REMOTE_BONES_ENTRY
} // namespace skeet::animation

// ============================================================================
// Subcomponent: runtime_bindings.cpp
// ============================================================================

namespace skeet::animation {

BoneRuntimeFields native_bone_runtime_fields(const engine::NativeOffsetGlobals& globals) {
    //43388720/433FD30D,433899B3,43341E47 and433FF63C. Pose parameters are
    //the24-float range decoded at4331ED7F and4341D407, not the layer vector.
    return {offset(globals, 0x43467ABCu, 0x4346A56Cu, 0xA25D8271u),
            offset(globals, 0x43467E8Cu, 0x43467900u, 0x32E4DAF1u),
            offset(globals, 0x434683B4u, 0x43467E50u, 0x57B2D535u),
            offset(globals, 0x4346703Cu, 0x43468FD4u, 0xAE5B982Eu),
            offset(globals, 0x43468B50u, 0x43467B48u, 0x38CA5228u),
            offset(globals, 0x43467634u, 0x43468F2Cu, 0xBB78DC43u),
            offset(globals, 0x43467A74u, 0x434678C0u, 0xCD812654u),
            offset(globals, 0x43467180u, 0x43467FA4u, 0xCA2B5FFBu),
            offset(globals, 0x43467F4Cu, 0x4346A8B0u, 0x3D52A1A7u),
            offset(globals, 0x43467CD8u, 0x43468DD8u, 0x01D0BACCu),
            offset(globals, 0x4346A660u, 0x434679E0u, 0xB538BBA5u),
            offset(globals, 0x43468368u, 0x43467FACu, 0x3B1F6ECCu),
            offset(globals, 0x434688D4u, 0x43469030u, 0xF8205EBAu),
            offset(globals, 0x43468D30u, 0x43468ED4u, 0x2A671B42u),
            offset(globals, 0x434670E0u, 0x43467D28u, 0x4E66BB34u),
            offset(globals, 0x43469224u, 0x4346718Cu, 0x83A6CF2Bu),
            offset(globals, 0x434670CCu, 0x43468BCCu, 0xA9EE6062u),
            offset(globals, 0x43468240u, 0x4346A84Cu, 0xC2B6FB10u),
            offset(globals, 0x43468E7Cu, 0x434674A8u, 0x54783141u),
            offset(globals, 0x43467D78u, 0x43467B80u, 0x1CAAF0A8u)};
}

RemoteSetupBonesFields
native_remote_setup_bones_fields(const engine::NativeOffsetGlobals& globals) {
    //433FF7F6 and the linked-entity traversal in4339C7A0. The -14/-10 child
    //handle displacements are applied by that consumer, not folded in here.
    return {offset(globals, 0x43469064u, 0x4346A484u, 0x17978090u),
            offset(globals, 0x4346A808u, 0x43468BE8u, 0x2B554AD0u),
            offset(globals, 0x43468EA0u, 0x4346A74Cu, 0xE23A9BE1u),
            offset(globals, 0x43469344u, 0x43467018u, 0x49D93E62u)};
}

BoneRuntime native_bone_runtime(const engine::NativeDataImage& data,
                                const engine::NativeOffsetGlobals& globals,
                                BoneAttachmentUpdate attachment_update,
                                const RageTrigonometry& trigonometry) {
    if (!attachment_update ||
        attachment_update != data.native_cell<BoneAttachmentUpdate>(0x43468F80u))
        throw std::invalid_argument(
            "Bone runtime requires the published build animation attachment entry");
    auto* const entity_list = data.native_cell<void*>(0x4346A890u);
    if (!entity_list || !trigonometry.sine || !trigonometry.cosine)
        throw std::invalid_argument(
            "Bone runtime requires its canonical entity list and CRT trigonometry");
    return {native_bone_runtime_fields(globals), entity_list, attachment_update, trigonometry};
}

RemoteSetupBonesBindings
native_remote_setup_bones_bindings(const engine::NativeDataImage& data,
                                   const engine::NativeOffsetGlobals& globals,
                                   const BoneRuntime& bones, client::PlayerStateCache& players,
                                   const RemoteAnimationPreparationRuntime& preparation) {
    if (bones.entity_list != data.native_cell<void*>(0x4346A890u) ||
        bones.update_attachments != data.native_cell<BoneAttachmentUpdate>(0x43468F80u))
        throw std::invalid_argument(
            "Remote bone setup must share canonical bone runtime dependencies");
    auto* const global_vars = data.native_cell<std::byte*>(0x43467B04u);
    if (!global_vars || reinterpret_cast<std::uintptr_t>(global_vars) % alignof(float))
        throw std::invalid_argument("Remote bone setup requires published aligned gpGlobals");
    return {bones,
            players,
            preparation,
            native_remote_setup_bones_fields(globals),
            data.native_cell<const std::int32_t*>(0x43469060u),
            data.native_cell<BoneEnginePrepare>(0x4346A38Cu),
            data.native_cell<BoneEngineThreadPredicate>(0x43476920u),
            data.native_cell<BoneEngineAttachments>(0x4346A6B0u),
            *reinterpret_cast<float*>(global_vars + 0x14),
            native_remote_bone_thread_guard};
}
} // namespace skeet::animation

// ============================================================================
// Subcomponent: runtime_owner.cpp
// ============================================================================

namespace skeet::animation {
namespace {
client::ClientRuntimeStorage& require_storage(client::ClientRuntimeStorage& storage,
                                              const client::EngineRuntimeBindings& engine) {
    const auto& data = storage.data();
    const auto core = engine.interfaces.core();
    if (&engine.variables.data() != &data || !core.engine_client || !core.entity_list ||
        core.engine_client != data.native_cell<void*>(0x4346A514u) ||
        core.entity_list != data.native_cell<void*>(0x4346A890u) ||
        !data.native_cell<void*>(0x4346A894u))
        throw std::invalid_argument(
            "Animation runtime requires its initialized canonical engine bundle");
    if (!engine.modules.at(u"client.dll") || !engine.modules.at(u"engine.dll") ||
        engine.modules.at(u"client.dll") != data.native_cell<void*>(0x4346A55Cu) ||
        engine.modules.at(u"engine.dll") != data.native_cell<void*>(0x43468D20u))
        throw std::invalid_argument(
            "Animation runtime engine modules belong to another data image");
    auto* const globals = data.native_cell<std::byte*>(0x43467B04u);
    if (!globals || reinterpret_cast<std::uintptr_t>(globals) % alignof(float) ||
        data.native_cell<void*>(0x434678F8u) != globals + 4 ||
        data.native_cell<void*>(0x43467EB8u) != globals + 0x10 ||
        data.native_cell<void*>(0x4346A338u) != globals + 0x20)
        throw std::invalid_argument(
            "Animation runtime requires the canonical gpGlobals member aliases");
    const auto& offsets = storage.offsets();
    const auto begin = reinterpret_cast<std::uintptr_t>(offsets.words);
    const auto cursor = reinterpret_cast<std::uintptr_t>(offsets.cursor);
    if (!begin || begin % alignof(std::uint32_t) || cursor < begin + 4 || cursor > begin + 0x1000 ||
        (cursor - begin) % 4 || offsets.globals.pointers.empty())
        throw std::invalid_argument("Animation runtime requires initialized native offset storage");
    for (const auto& [identity, pointer] : offsets.globals.pointers) {
        (void)identity;
        const auto address = reinterpret_cast<std::uintptr_t>(pointer);
        // Three published zero-offset sentinels intentionally use word0.
        if (address < begin || address >= cursor || (address - begin) % 4)
            throw std::invalid_argument(
                "Animation offset points outside its canonical published word allocation");
    }
    return storage;
}
RemoteAnimationPreparationBindings
preparation_bindings(client::ClientRuntimeStorage& storage,
                     const RemoteAnimationPreparationFields& fields,
                     const RageTrigonometry& trigonometry) {
    // Force all required operand identities to exist before publishing a
    // callable runtime; subsequent preparation calls still read these live.
    (void)std::array<std::ptrdiff_t, 12>{
        fields.frame,           fields.ik_mask,         fields.flags,      fields.previous_flags,
        fields.active_weapon,   fields.animation_state, fields.view_gate,  fields.observer_mode,
        fields.observer_target, fields.world_model,     fields.eye_origin, fields.duck_amount};
    auto& data = storage.data();
    auto* const globals = data.native_cell<std::byte*>(0x43467B04u);
    return {fields,
            trigonometry,
            data.native_cell<void*>(0x4346A514u),
            data.native_cell<void*>(0x4346A890u),
            data.native_cell<void*>(0x4346A894u),
            data.native_cell<void*>(0x43475808u),
            *reinterpret_cast<const std::int32_t*>(globals + 4),
            data.native_cell<RageVector>(0x43479CCCu),
            data.native_cell<RageVector>(0x43479CD8u)};
}
}
RuntimeOwner::RuntimeOwner(client::ClientRuntimeStorage& storage,
                           const client::EngineRuntimeBindings& engine,
                           client::PlayerStateCache& players)
    : storage_(require_storage(storage, engine)), players_(players),
      trigonometry_(native_sse_trigonometry()), preparation_fields_(storage_.offsets().globals),
      bones_(native_bone_runtime(storage_.data(), storage_.offsets().globals,
                                 engine.update_bone_attachments, trigonometry_)),
      preparation_(preparation_bindings(storage_, preparation_fields_, trigonometry_)),
      remote_bindings_(native_remote_setup_bones_bindings(
          storage_.data(), storage_.offsets().globals, bones_, players_, preparation_)),
      remote_(remote_bindings_) {}
} //namespace skeet::animation

// ============================================================================
// Subcomponent: state_copy.cpp
// ============================================================================

namespace skeet::animation {
namespace {
/* deduplicated helper */
/* deduplicated helper */
void purge(void* vector, void* allocator) {
    if (read<std::int32_t>(vector, 8) < 0)
        return;
    const auto address = read<std::uint32_t>(vector, 0);
    if (address) {
        engine::invoke<void>(allocator, 5, reinterpret_cast<void*>(std::uintptr_t(address)));
        write(vector, 0, std::uint32_t(0));
    }
    write(vector, 4, std::uint32_t(0));
}
}
void initialize_animation_state_vector(void* state) noexcept {
    for (std::size_t offset = 0x13c; offset <= 0x14c; offset += 4)
        write(state, offset, std::uint32_t(0));
}
void release_animation_state_vector(void* state, void* allocator) {
    auto* vector = static_cast<std::byte*>(state) + 0x13c;
    write(vector, 0xc, std::uint32_t(0));
    purge(vector, allocator);
    write(vector, 0x10, read<std::uint32_t>(vector, 0));
    purge(vector, allocator);
}
} // namespace skeet::animation

// ============================================================================
// Subcomponent: studio_bone_flags.cpp
// ============================================================================

namespace skeet::client {
void* PlayerPhysicsHooks::studio_header(void* player) {
    using namespace player_physics_detail;
    auto* header = original<void*>(player, 109);
    if (!header)
        return header;
    const auto* studio = field<const void*>(header, 0);
    const auto count = field<std::int32_t>(studio, 0x9C);
    for (std::int32_t index = 0; index < count; ++index) {
        auto* flags = field<std::uint32_t*>(header, 0x30);
        flags[index] &= ~4U;
    }
    return header;
}
} //namespace skeet::client
