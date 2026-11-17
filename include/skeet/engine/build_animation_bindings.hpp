#pragma once
#include "loaded_modules.hpp"
#include "native_data_image.hpp"
#include "skeet/animation/bone_runtime.hpp"

namespace skeet::engine {
// Observed client 65272025 / tier0 63DC10BA build binding policy. It finds actual
// loaded modules, verifies their identities/sections/code and follows native
// animation call operands. ThreadInMainThread is obtained from tier0's named
// export and checked against the client's actual import slot.
//
// Publishes 4346A38C,4346A6B0,43469060,43476920,43468F80 and returns that same typed
// attachment entry for subsequent BoneRuntime construction. Other consumers
// borrow the cells directly. Every target exists in this build: unsupported
// or missing recovery is an error, rather than an invented optional null.
// Runs no client/tier0 function and loads no module. Call before bone consumers;
// both genuine modules must remain loaded throughout their use.
animation::BoneAttachmentUpdate initialize_build_animation_bindings(NativeDataImage&,
                                                                    const LoadedModuleEnvironment&);

// Narrow prerequisite for the startup search over the native animation Update
// body. Two real client call sites independently supply the animation-state
// receiver and XMM1/XMM2 angles to this same RET4 entry. Publishes only434693F0;
// returns readable genuine client code without adapting or calling its SIMD ABI.
// Subsequent startup pattern-search outputs belong to their own operation.
const std::byte* initialize_build_animation_update_code(NativeDataImage&,
                                                        const LoadedModuleEnvironment&);

// Next startup code input43467F0C. Follows Update's real direct call to its
// vector helper, then verifies the interior block and branch to the32-byte
// hash window at block+4C. The result is inspected code, not a callable entry.
// Publishes only43467F0C and performs no hash-result or later VM publication.
const std::byte* initialize_build_animation_velocity_code(NativeDataImage&,
                                                          const LoadedModuleEnvironment&);
} // namespace skeet::engine
