#pragma once
#include "fix_animations.hpp"
#include "fix_animation_backup.hpp"
#include "skeet/engine/offset_storage.hpp"

namespace skeet::client {
class ConfigurationRuntimeOwner;
}
namespace skeet::animation {
class RuntimeOwner;
FixAnimationFields native_fix_animation_fields(const engine::NativeOffsetGlobals&);
FixAnimationEntityFields native_fix_animation_entity_fields(const engine::NativeOffsetGlobals&);
// Stable source owners supply all interfaces, settings and live global cells.
// Keep data_update_fields and opacity alive for the returned binding. The
// final owner additionally verifies the actual HookRuntimeOwner identity.
FixAnimationsBindings native_fix_animations_bindings(RuntimeOwner&,
                                                     client::ConfigurationRuntimeOwner&,
                                                     const client::PlayerDataUpdateOffsets&,
                                                     const effects::PlayerOpacityBindings&);
FixAnimationBackupBindings native_fix_animation_backup_bindings(RuntimeOwner&,
                                                                const FixAnimationsBindings&);
// Actual loaded msvcrt.dll!ceil, using its ordinary double(double) CRT ABI.
// No new module is loaded and no replacement math implementation is selected.
using AnimationCeiling = double (*)(double);
AnimationCeiling native_animation_ceiling();
} //namespace skeet::animation
