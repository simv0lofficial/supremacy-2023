#pragma once
#include "fix_animations.hpp"
#include "skeet/client/startup_reset.hpp"

namespace skeet::animation {
struct FixAnimationBackupGrowth {
    float maximum_load; //4346D6F0, native initial1
    float multiplier; //4346D6F4, native initial2
    std::uint32_t next_resize; //4346D6F8
};
static_assert(sizeof(FixAnimationBackupGrowth) == 12);
struct FixAnimationBackupInsert {
    FixAnimationBackupNode* node;
    FixAnimationBackupNode** bucket;
    bool inserted;
};
// References are the SAME4346D6E4/E8/EC and policy words read by
// FixAnimationsRuntime::restore_animation_backup. There is no second map.
// Native initial buckets point at{nullptr,FFFFFFFF}, count1,size0.
struct FixAnimationBackupBindings {
    FixAnimationBackupNode**& buckets;
    std::uint32_t& bucket_count;
    std::uint32_t& size;
    FixAnimationBackupGrowth& growth;
    const std::int32_t& mode; //same4347C9EC transition status
    const std::int32_t& prediction_frame; //43467B04+4
    const BoneRuntime& bones;
    FixAnimationEntityView entity;
    std::ptrdiff_t life_state_9d920e35;
    engine::JobMemory memory;
    // Actual recorded CRT ceil77512A30. This is an import, not a map helper.
    double (*ceiling)(double);
};
struct FixAnimationBackupDetachBindings {
    engine::VtableHookRegistry& hooks; //same UnloadState::hooks sentinel
    const std::ptrdiff_t& extra_interface_offset; //43479CE4
    std::byte* player_settings; //4347BE00, stride20; marker at+4
    client::TargetResetState target; //canonical shared AA/command globals
};

class FixAnimationBackupRuntime {
public:
    explicit FixAnimationBackupRuntime(const FixAnimationBackupBindings& bindings,
                                       const FixAnimationBackupDetachBindings& detach) noexcept
        : bindings(bindings), detach(detach) {}
    const FixAnimationBackupBindings& bindings;
    const FixAnimationBackupDetachBindings& detach;

    // Full43329CDF/433C140F/4332C319; callbacks are only actual heap imports.
    FixAnimationBackupNode* create_node(const std::uint32_t* key);
    FixAnimationBackupInsert find_or_insert(const std::uint32_t* key);
    void rehash(std::uint32_t bucket_count);
    // Full43393973. The live animation pointer is required under its native
    // nonzero-mode/alive contract. Capture clears the borrowed vector header.
    void capture(void* player);
    // Full433CB965..433CBAD2: unhook three actual interface getters, optional
    // extra interface and base entity, capture, player marker, target reset.
    void detach_entity(void* player);

private:
    struct GrowthDecision {
        bool rehash;
        std::uint32_t bucket_count;
    };
    GrowthDecision plan_growth(std::uint32_t buckets, std::uint32_t size);
    void* allocate(std::size_t);
    void free(void*);
};

// Called by the same preserved initialization/hook bridge that routes the
// ordinary captured43393973/433CB965 entries to their concrete source bodies.
// Binding does not introduce a new frame phase or reinterpret VM operands.
void bind_fix_animation_backup(FixAnimationBackupRuntime&) noexcept;
#if defined(_M_IX86) || defined(__i386__)
void __fastcall capture_fix_animation_backup(void* player, void* ignored_edx);
void __fastcall detach_fix_animation_entity(void* player, void* ignored_edx);
#else
void capture_fix_animation_backup(void* player, void* ignored_edx);
void detach_fix_animation_entity(void* player, void* ignored_edx);
#endif
} // namespace skeet::animation
