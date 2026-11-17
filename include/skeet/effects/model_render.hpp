#pragma once
#include "skeet/esp/model_color_config.hpp"
#include "skeet/esp/effects_config.hpp"
#include "skeet/rage_hitboxes.hpp"
#include "skeet/animation/bones.hpp"
#include "skeet/animation/bone_runtime.hpp"
#include "skeet/lagcomp/interpolation.hpp"
#include "skeet/players/state.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace skeet::engine {
class NativeDataImage;
}
namespace skeet::client {
class PlayerStateCache;
}
namespace skeet::effects {
class ModelRenderHooks;
class ModelShadowTimingSource;

// Genuine materialsystem internal fastcall recorded in43467A68. The supported
// body consumes material/name in ECX/EDX; it is not a PE export.
#if defined(_M_IX86) || defined(__i386__)
using CreateMaterialVariable = void*(__fastcall*)(void*, const char*);
#else
using CreateMaterialVariable = void* (*)(void*, const char*);
#endif
struct ModelMaterialInterfaces {
private:
    void* standalone_material_ = nullptr;
    CreateMaterialVariable standalone_create_ = nullptr;
    void* standalone_clock_ = nullptr;

public:
    ModelMaterialInterfaces() noexcept
        : material_system(standalone_material_), create_variable(standalone_create_),
          clock(standalone_clock_) {}
    ModelMaterialInterfaces(void* material, void* key_values, CreateMaterialVariable create,
                            void* timer = nullptr) noexcept
        : ModelMaterialInterfaces() {
        material_system = material;
        key_values_system = key_values;
        create_variable = create;
        clock = timer;
    }
    ModelMaterialInterfaces(engine::NativeDataImage&, void* key_values);
    ModelMaterialInterfaces(const ModelMaterialInterfaces&) = delete;
    ModelMaterialInterfaces& operator=(const ModelMaterialInterfaces&) = delete;
    void*& material_system; //live4346A4E0
    void* key_values_system = nullptr; // vstdlib!KeyValuesSystem()
    CreateMaterialVariable& create_variable; //live43467A68, optional native entry
    void*& clock; //live43467118, clock[5] returns double
};

// 0x44-byte native record, constructed by 0x433A0E96. Tokens belong to
// FindVarFast; these are not IMaterialVar pointers.
struct ModelMaterial {
    void* material = nullptr;
    std::array<std::uint32_t, 11> variable_tokens{};
    const IntegerConfig* kind = nullptr;
    const ColorConfig* secondary = nullptr;
    std::int32_t applied_kind = -1;
    std::uint32_t applied_color = UINT32_MAX;
    bool on_shot = false;
};

struct ModelDrawState {
    std::array<std::byte, 0x38> bytes{}; // studio render +0x240..+0x277
};
struct ModelPassSettings {
    // Native packet bits: local=1, special model=2, visible=4, occluded=8,
    // shadow/history=16. 0x433E655C selects these from the owner descriptors.
    std::uint8_t flags = 0;
    std::uint32_t visible_color = 0, occluded_color = 0;
    ModelMaterial* material = nullptr;
};
struct PlayerModelColors {
    std::uint8_t flags;
    std::uint32_t visible_color, occluded_color;
};
// Shared descriptor selection for drawing and opacity. Material resources are
// deliberately outside this helper: its three outputs contain only settings.
PlayerModelColors player_model_colors(const esp::ColoredModelsSettings&, bool local, bool teammate,
                                      bool history_available, bool ragdoll = false) noexcept;
struct ModelDrawArguments {
    void* results;
    void* info;
    void* bones;
    void* flex_weights;
    void* delayed_flex_weights;
    void* origin;
    std::uint32_t flags;
};
enum class RenderedModel : std::uint32_t {
    player = 0,
    viewmodel = 1,
    hands = 2,
    flagged_viewmodel = 3,
    other = 4,
    ragdoll = 5,
    player_attachment = 6,
    owned_weapon = 7,
    unowned_weapon = 8,
    world_weapon_model = 9,
};
class ModelRenderableTables {
public:
    ModelRenderableTables() noexcept = default;
    explicit ModelRenderableTables(engine::NativeDataImage& data) noexcept : data_(&data) {}
    void*& operator[](std::size_t);
    void* const& operator[](std::size_t) const;

private:
    engine::NativeDataImage* data_ = nullptr;
    std::array<void*, 9> standalone_{};
};
struct ModelClassification {
    // Recovered renderable vtables, in native comparison order. The first two
    // are 0x434758AC/0x434759A8; the others are 0x43468908, 0x43467190,
    // owner+0x73C, 0x43475B44, owner+0x738, 0x4346A444, 0x43469284.
    ModelRenderableTables renderable_vtables;
    void* entity_list = nullptr;
    std::uint32_t entity_flags; // *0x43468B50 ^ *0x43467B48 ^ 0x38CA5228
    std::uint32_t owner_handle; // *0x43468C9C ^ *0x43469280 ^ 0x4E36DFD7
};
struct ModelShotState {
private:
    double standalone_expires_ = 0;
    void* standalone_begin_ = nullptr;
    void* standalone_end_ = nullptr;
    std::array<float, 3> standalone_origin_{};

public:
    ModelShotState() noexcept
        : expires(standalone_expires_), bones_begin(standalone_begin_), bones_end(standalone_end_),
          origin(standalone_origin_) {}
    explicit ModelShotState(engine::NativeDataImage&);
    ModelShotState(const ModelShotState&) = delete;
    ModelShotState& operator=(const ModelShotState&) = delete;
    double& expires; //physical4347BDE8
    void*& bones_begin; //physical434797A0
    void*& bones_end; //physical434797A4
    std::array<float, 3>& origin; //physical4347BDF0
};
struct ModelEntityBindings {
    void* engine;
    void* entity_list;
    void* local_player;
    std::uint32_t life_state, move_type, observer_mode, observer_target;
    std::uint32_t team, survival_team;
    std::uint32_t move_parent; // resolved 0x49D93E62 property minus 0x18
    std::int32_t game_type;
    bool teammates_are_enemies;
};
struct FakeModelPose {
    bool available; // native 0x43475940
    std::int32_t capture_tick, current_tick;
    float blend; // 0x43475890
    float observed_yaw, reference_yaw; // record +0x258/+0x194
    RageVector observed_angles, reference_angles; // 0x43476414/0x4347A4F0
    RageVector transform_angles; // record +0x1B8
    float rotation_radians; // 0x43472534
    std::span<const Matrix3x4> bones; // record +0x520
};
// Complete 0x43369C23, with engine-captured fake pose inputs and recorded CRT
// sine/cosine semantics. Output capacity follows the captured bone-count contract.
bool build_fake_model_matrices(const FakeModelPose&, const RageVector& origin,
                               std::span<Matrix3x4> output, const RageTrigonometry&);
struct ModelShadowTiming {
    float interval, current_time;
    float update_rate, interpolation, interpolation_ratio;
    bool local_alive, prediction_enabled;
    std::int32_t tick_base;
    float prediction_counter; // native globals +0x24 is loaded with MOVSS
    float prediction_correction;
    std::array<float, 16> latency_samples;
    std::int32_t latency_sample_head, latency_sample_count;
    std::int32_t command_tick;
    const std::int32_t* previous_command;
    const std::int32_t* current_command;
    std::int32_t maximum_unlag_ms;
};
struct ModelShadowPose {
    const lagcomp::LagRecord* animation_record;
    float time;
    RageVector origin, angles;
};
// 43406265..43406674, including native timing helpers4331E237/43385D57.
// Ring records are nonnull, ordered from newest to oldest, with at most32.
bool select_model_shadow_pose(const lagcomp::LagTimeline&, const ModelShadowTiming&,
                              ModelShadowPose&, const lagcomp::RotationMath&);
// Explicit input profiles: production borrows the canonical cache and samples
// live timing; standalone algorithm fixtures borrow their supplied timeline.
class ModelShadowSource {
public:
    ModelShadowSource(client::PlayerStateCache&, const ModelShadowTimingSource&) noexcept;
    ModelShadowSource(const std::array<const lagcomp::LagTimeline*, 64>&,
                      const ModelShadowTiming&) noexcept;
    bool select(std::uint32_t entity_index, ModelShadowPose&, const lagcomp::RotationMath&) const;

private:
    client::PlayerStateCache* cache_ = nullptr;
    const ModelShadowTimingSource* live_ = nullptr;
    const std::array<const lagcomp::LagTimeline*, 64>* timelines_ = nullptr;
    const ModelShadowTiming* timing_ = nullptr;
};
struct ModelShadowBones {
    animation::BoneEntityState& state;
    const animation::BoneModel& model;
    const animation::BoneEngine& engine;
    const RageTrigonometry& trigonometry;
    void* layers;
    std::int32_t layer_count;
    float* pose_parameters;
};
struct ModelDrawFrame {
    const ModelClassification& classification;
    const ModelEntityBindings& entities;
    const RageTrigonometry& trigonometry;
    const lagcomp::RotationMath& rotation_math;
    std::uint32_t local_player_flag, immunity;
    std::uint32_t ragdoll_owner, attachment_owner;
    bool suppress_effects; // 0x43468E58
    bool player_effects; // 0x4346A65C
    const players::State& player_adjustments; // live4347BE09 + native index*32
    bool history_available; // resolved native history cvar condition
    bool fake_controller; // 0x434685B8 !=0
    const FakeModelPose* fake_pose; // absent native 0x43475940 is null
    ModelShadowSource shadows;
    const animation::BoneRuntime& bone_runtime;
    std::uint32_t shot_entity_handle; // 0x4346D674; -1 means absent
    const ModelShotState* shot; // null means the native shot vector is empty
    std::int32_t wall_transparency; // shared effects descriptor 0x9B4090D9
};
struct LocalTransparencyInterfaces {
    void* input;
    void* entity_list;
    void* model_cache;
};
struct LocalTransparencyNetvars {
    std::uint32_t field_af649dac;
    std::uint32_t field_cacaeecc; // native special guard also reads this minus0xC
    std::uint32_t field_c07d1150;
    std::uint32_t active_weapon;
};
struct ModelOcclusionRecord {
    void* renderable;
    std::uint16_t field_04;
    std::uint8_t field_06, flags, model_type;
    std::array<std::byte, 3> untouched;
};
struct ModelOcclusionStorage {
    std::array<std::byte, 0x14> untouched;
    std::array<std::array<ModelOcclusionRecord, 4096>, 3> buckets;
    std::array<std::int32_t, 3> counts;
};
struct ModelOcclusionItem {
    void* renderable;
    void* metadata;
    std::array<std::byte, 0x18 - 2 * sizeof(void*)> untouched;
    std::uint16_t flags;
    std::array<std::byte, 0x4E - 0x1A> remaining;
    std::uint16_t next;
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(ModelOcclusionRecord) == 0xC);
static_assert(offsetof(ModelOcclusionStorage, buckets) == 0x14);
static_assert(offsetof(ModelOcclusionStorage, counts) == 0x24014);
#endif
static_assert(sizeof(ModelOcclusionItem) == 0x50);
struct ModelOcclusionInterfaces {
    void* entity_list; // 0x4346A890
    void* model_info; // 0x4346A614
    void* model_type_registry; // 0x43469330
    void* player_renderable_vtable; // 0x434758AC
    std::uint32_t shot_entity_handle;
};
struct ModelPropBatch {
    std::array<std::byte, 8> untouched;
    void* vertices;
    std::int32_t count;
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(ModelPropBatch) == 0x10);
static_assert(offsetof(ModelPropBatch, vertices) == 8);
static_assert(offsetof(ModelPropBatch, count) == 0xC);
#endif

class ModelRenderRuntime {
public:
    ModelRenderHooks* hooks = nullptr;
    ModelRenderRuntime(ModelMaterialInterfaces& interfaces,
                       const esp::ColoredModelsSettings& settings);
    ModelMaterialInterfaces& interfaces;
    const esp::ColoredModelsSettings& settings;
    std::array<ModelMaterial, 8> materials;
    void* solid_material = nullptr; // owner +0x678; located by hash 0xF4A7A78A
    void* shadow_material = nullptr; // +0x680; hash 0x27DB215F
    void* base_texture = nullptr; // +0x694
    void* metallic_environment = nullptr; // +0x6A8
    void* glow_environment = nullptr; // +0x6BC
    void* fullscreen_texture = nullptr; // +0x6D0
    std::array<std::uint32_t, 2> material_name_hashes{0xF4A7A78Au, 0x27DB215Fu};
    std::array<std::string, 4> texture_names{"vgui/white", "editor/cube_vertigo_blurred_hdr",
                                             "models/effects/cube_white", "_rt_Fullscreen"};
    float local_alpha = 1.0f; // native owner+0x38
    bool local_alpha_overridden = false; // owner+0x3C

    // Full resource setup 0x433F0977, texture refs 0x433378F3 and material
    // enumeration 0x433468AF. Successful lookup consumes its native name/hash.
    void initialize_resources();

    // Complete lazy material/settings chain, 0x433C6694 and 0x4336BB4B.
    void* prepare_material(ModelMaterial* material);
    void invalidate_material(void* material); // complete4333CDB7, Lua material refresh
    // Complete native color/alpha helpers 0x433F4C0E / 0x433ADE29.
    static void modulate(void* studio_render, std::uint32_t color);
    static void limit_alpha(void* studio_render, std::uint32_t color);
    static ModelDrawState save_state(void* studio_render);
    static void restore_state(void* studio_render, const ModelDrawState& saved);
    // Complete 0x4332CFC4; studio header name resolver 0x433C0864 is included.
    static RenderedModel classify(const void* draw_info, const ModelClassification& bindings);
    // Selects the exact packet for player/teammate/local/ragdoll branches.
    // The entity classification and teammate predicate are supplied as their
    // results, without replacing either engine/entity implementation.
    ModelPassSettings player_pass(bool local, bool teammate, bool history_available,
                                  bool ragdoll = false) noexcept;
    // Visible/occluded body 0x43406770..0x43406A56. History bone interpolation
    // precedes this body in the original and has its own owning subsystem.
    void apply_player_passes(void* studio_render, const ModelDrawState& saved,
                             const ModelDrawArguments& args, const ModelPassSettings& pass,
                             std::int32_t wall_transparency = 100);
    void apply_single_pass(void* studio_render, std::size_t group);
    // Complete hands/viewmodel branches of 0x433E655C, including the two
    // material-index overrides and save/draw/restore owner sequence.
    bool draw_viewmodel(void* studio_render, const ModelDrawArguments&, RenderedModel category);
    // 0x434181CD and dependencies 0x43359AFA / 0x4339EF8C, valid entity contract.
    static bool teammate(void* entity, const ModelEntityBindings&);
    // Complete owned/world-weapon branches of 0x433E655C, including walking
    // the move-parent chain and using the viewmodel settings for local owners.
    bool draw_weapon(void* studio_render, const ModelDrawArguments&, RenderedModel,
                     const ModelEntityBindings&);
    // Complete 0x43327FF7. Capture of the shot's matrices/origin/expiry belongs
    // to the event owner; the native draw helper only consumes this state.
    void draw_on_shot(void* studio_render, const ModelDrawState& saved,
                      const ModelDrawArguments& args, const ModelShotState& shot);
    // 0x433E6E13..0x433E6F05; caller has passed native player eligibility,
    // active fake-controller and local-entity gates. Matrix generation is the
    // complete 43369C23 above, rather than a captured-output replacement.
    void draw_local_fake(void* studio_render, const ModelDrawState& saved,
                         const ModelDrawArguments&, const FakeModelPose&, const RageTrigonometry&);
    // 43406499..43406770 composed with restored animation and bone owners.
    // Returns false when engine/model readiness rejects the native bone build.
    bool draw_shadow(void* studio_render, const ModelDrawArguments&, const ModelShadowPose&,
                     ModelShadowBones&);
    // Live production join: studio, model, hierarchy and temporary entity state
    // are sampled by43388720 for this entity/call, under selected animation.
    bool draw_shadow(void* studio_render, const ModelDrawArguments&, const ModelShadowPose&,
                     void* player, const animation::BoneRuntime&);
    // Complete top-level 433E655C selection, using the concrete restored
    // material/fake/history owners above and actual engine entity interfaces.
    void draw_model(void* studio_render, const ModelDrawArguments&, const ModelDrawFrame&);
    // Complete433CA178 and4336AC57/4337E85B. renderable is the actual
    // player+4 interface, provided explicitly for host pointer-width verification.
    void update_local_transparency(void* player, void* renderable,
                                   const LocalTransparencyInterfaces&,
                                   const LocalTransparencyNetvars&, bool suppressed,
                                   bool player_effects, float frame_time);
    // Complete43368F23: original list builder, native linked-item traversal,
    // classification, duplicate/capacity handling, and model-type queries.
    void build_unoccluded_list(void* renderable_list, void* view, const ModelOcclusionInterfaces&);
    // Complete prop-lighting hooks433B0A83/433C930C/4337D6DC/43401A33.
    // The original engine methods remain in their hook descriptor tables.
    void draw_prop_records_50(void* renderer, std::uintptr_t argument, std::int32_t count,
                              void* records, std::uintptr_t last, const esp::EffectsSettings&,
                              bool suppressed);
    void draw_prop_records_48(void* renderer, std::uintptr_t argument, std::int32_t count,
                              void* records, std::int32_t stride, std::uintptr_t last,
                              const esp::EffectsSettings&, bool suppressed);
    void draw_prop_batches_60(void* renderer, std::uintptr_t argument, std::int32_t count,
                              ModelPropBatch* batches, std::int32_t stride, std::uint32_t flags,
                              void* ragdoll_renderable_vtable, const esp::EffectsSettings&,
                              bool suppressed);
    void draw_prop_studio(void* studio_render, std::uintptr_t first, std::uintptr_t second,
                          std::uintptr_t third, const esp::EffectsSettings&, bool suppressed);

private:
    void draw_shadow_matrices(void* studio_render, const ModelDrawArguments&,
                              const ModelShadowPose&, std::span<Matrix3x4>);
    void* create_material();
    void resolve_variable(void* material, const char* name, std::uint32_t& token);
    void apply_gated_player(void* studio_render, const ModelDrawState&, const ModelDrawArguments&,
                            void* entity, const ModelPassSettings&, const ModelDrawFrame&);
};

} // namespace skeet::effects
