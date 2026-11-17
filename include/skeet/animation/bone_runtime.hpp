#pragma once
#include "skeet/animation/bones.hpp"
#include "skeet/engine/interface_call.hpp"

namespace skeet::animation {
// Decoded entity fields used by43388720. Layer storage is a native vector:
// data at+0, count at+C, stride38; layer12's weight is at+2C0.
struct BoneRuntimeFields {
    std::ptrdiff_t model_index_a25d8271;
    std::ptrdiff_t studio_wrapper_32e4daf1;
    std::ptrdiff_t interpolation_flags_57b2d535;
    std::ptrdiff_t animation_frame_ae5b982e;
    std::ptrdiff_t effects_38ca5228;
    std::ptrdiff_t setup_flags_bb78dc43;
    std::ptrdiff_t owner_handle_cd812654;
    std::ptrdiff_t related_handle_ca2b5ffb;
    std::ptrdiff_t animation_state_3d52a1a7;
    std::ptrdiff_t ik_mask_01d0bacc;
    std::ptrdiff_t animation_layers_b538bba5;
    std::ptrdiff_t angles_3b1f6ecc;
    std::ptrdiff_t angles_f8205eba;
    std::ptrdiff_t origin_2a671b42;
    std::ptrdiff_t origin_4e66bb34;
    std::ptrdiff_t entity_flags_83a6cf2b;
    std::ptrdiff_t bone_origin_a9ee6062;
    std::ptrdiff_t hitbox_set_c2b6fb10;
    std::ptrdiff_t bone_accessor_54783141;
    std::ptrdiff_t pose_parameters;
};

using BoneAttachmentUpdate = engine::MemberFunction<void, Matrix3x4*, std::uint32_t>;

// Owns only the small hitbox-index projection; studio arrays remain live engine
// storage. Call model() after constructing the context so its hitbox span points
// into this object. A negative native hitbox count is retained in hitbox_count.
struct BoneModelContext {
    const void* wrapper = nullptr;
    const void* header = nullptr;
    bool virtual_model_required = false;
    bool virtual_model = false;
    std::span<const std::uint32_t> flags;
    std::span<const std::int32_t> parents;
    std::int32_t hitbox_count = 0;
    std::array<std::uint32_t, 19> hitbox_bones{};
    BoneModel model() const noexcept;
};

struct BoneRuntime {
    BoneRuntimeFields fields;
    void* entity_list;
    // Optional genuine engine entry43468F80, as tested by43341E47.
    BoneAttachmentUpdate update_attachments;
    const RageTrigonometry& trigonometry;

    // Complete433FD30D and studio/hitbox-set descriptor projection. Every call
    // reads the supplied player's current wrapper, header and selected set.
    BoneModelContext load_model(const void* player) const;
    std::span<float, 24> pose_parameters(void* player) const noexcept;

    // Complete live43388720 transaction. Count is zeroed before the renderable
    // model virtual and published after owner/IK resolution, exactly natively.
    // Engine pose accumulation is the actual entity virtual+338 (slot206).
    bool build_bone_matrices(void* player, const BoneRequest&, std::span<Matrix3x4>,
                             std::int32_t* count = nullptr) const;
    // Native callers can supply the player's cache pointer directly. Its
    // output extent is formed from the same fresh hierarchy read used by the
    // build body, after pose accumulation; no early model/count is sampled.
    bool build_bone_matrices(void* player, const BoneRequest&, Matrix3x4* output,
                             std::int32_t* count = nullptr) const;
    // Complete433899B3, with a fresh studio/set read after the preceding build.
    std::int32_t select_hitbox_bone_matrices(const void* player, std::span<const Matrix3x4>,
                                             std::span<Matrix3x4>) const;
    // Complete43407EA3 and43341E47. Scatter leaves all other scratch matrices
    // intact; attachment updates temporarily force only the two accessor masks.
    void scatter_hitbox_bone_matrices(const void* player, std::span<const Matrix3x4> hitboxes,
                                      std::span<Matrix3x4> bones) const;
    void scatter_hitbox_bone_matrices(const void* player, std::span<const Matrix3x4> hitboxes,
                                      Matrix3x4* bones) const;
    void update_bone_attachments(void* player, std::span<Matrix3x4>, std::uint32_t mask) const;
    // Complete4338AC35: scatter, genuine attachment update, then a fresh model
    // selection. The engine may change the selected studio/set during update.
    std::int32_t refresh_hitbox_attachments(void* player, std::span<Matrix3x4, 19>,
                                            std::span<Matrix3x4, 256> scratch) const;
    // Complete433FF63C: fresh A9EE6062 origin, yaw-only angles, zero time,
    // mask80300, all bones, then a fresh433899B3 model selection.
    std::int32_t build_resolver_hitbox_matrices(void* player, float yaw,
                                                std::span<Matrix3x4>) const;
};
} // namespace skeet::animation
