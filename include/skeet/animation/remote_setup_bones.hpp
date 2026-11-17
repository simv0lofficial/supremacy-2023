#pragma once
#include "skeet/animation/bone_runtime.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/engine/hooks.hpp"

namespace skeet::animation {
class RemoteAnimationPreparationRuntime;

struct RemoteSetupBonesFields {
    std::ptrdiff_t bone_cache_17978090; // pointer +0, signed count +C
    std::ptrdiff_t recent_bone_counter_2b554ad0;
    std::ptrdiff_t view_angles_e23a9be1; // pitch +0, yaw +4
    std::ptrdiff_t transform_guard_49d93e62; // first child handle -14,next -10
};

#if defined(_M_IX86) || defined(__i386__)
using BoneEngineThreadPredicate = bool(__cdecl*)();
#else
using BoneEngineThreadPredicate = bool (*)();
#endif
using BoneEnginePrepare = engine::MemberFunction<void>;
using BoneEngineAttachments = engine::MemberFunction<void, void*>;

// Actual per-thread TEB byte used by433FF7F6, not a process-wide lock.
std::uint8_t& native_remote_bone_thread_guard() noexcept;

struct RemoteSetupBonesBindings {
    const BoneRuntime& bones;
    client::PlayerStateCache& players;
    const RemoteAnimationPreparationRuntime& preparation; // complete43402385
    RemoteSetupBonesFields fields;
    const std::int32_t* const& current_bone_counter; // optional43469060
    BoneEnginePrepare const& prepare_on_engine_thread; // optional4346A38C
    BoneEngineThreadPredicate const& on_engine_thread; // optional43476920
    BoneEngineAttachments const& update_attachments; // optional4346A6B0
    float& frame_time; //43467B04+14, live global frame duration
    std::uint8_t& (*thread_guard)() noexcept = native_remote_bone_thread_guard;
};

class RemoteSetupBonesRuntime {
public:
    explicit RemoteSetupBonesRuntime(const RemoteSetupBonesBindings& bindings) noexcept
        : bindings(bindings) {}
    const RemoteSetupBonesBindings& bindings;
    // Complete433FF7F6, native thunk4334AD2A. ECX is IClientRenderable
    // (player+4), stack output/max/mask/time, RET16. The incoming EBP copied
    // to EDX by the thunk and the time argument are unused by this body.
    bool setup_bones(void* renderable, Matrix3x4* output, std::int32_t maximum, std::uint32_t mask,
                     float time);
    // Complete4339C7A0. Calls the genuine optional engine attachment entry,
    // then invalidates each linked entity, preserving native field ordering.
    void refresh_attachments(void* player) const;
    // Only slot13 is owned here; slot37 has a different captured handler.
    void bind(engine::VtableHookMetadata&) noexcept;
    static void* source_entry() noexcept;
};
} // namespace skeet::animation
