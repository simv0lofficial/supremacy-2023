#pragma once
#include "skeet/animation/remote_setup_bones.hpp"
#include "skeet/engine/native_data_image.hpp"
#include "skeet/engine/offset_storage.hpp"

namespace skeet::animation {
// Decode the actual pointer/key/salt operands after native offset publication.
// These value layouts are initialized once for the bound entity schema; they
// do not replace RemoteAnimationPreparationFields' live DecodedOffset members.
BoneRuntimeFields native_bone_runtime_fields(const engine::NativeOffsetGlobals&);
RemoteSetupBonesFields native_remote_setup_bones_fields(const engine::NativeOffsetGlobals&);

// attachment_update is the typed result returned by
// engine::initialize_build_animation_bindings. Its identity is checked against
// canonical43468F80; the entity list comes from canonical4346A890.
BoneRuntime native_bone_runtime(const engine::NativeDataImage&, const engine::NativeOffsetGlobals&,
                                BoneAttachmentUpdate attachment_update, const RageTrigonometry&);

// Call after game-state and animation function publication. The returned
// references borrow the actual optional function/counter cells and gpGlobals
// frame duration, so cell changes are visible to the remote handler. Keep the
// data image, global-vars object and supplied source runtimes alive throughout
// use; RemoteSetupBonesRuntime also borrows the returned bindings object.
RemoteSetupBonesBindings native_remote_setup_bones_bindings(
    const engine::NativeDataImage&, const engine::NativeOffsetGlobals&, const BoneRuntime&,
    client::PlayerStateCache&, const RemoteAnimationPreparationRuntime&);
} // namespace skeet::animation
