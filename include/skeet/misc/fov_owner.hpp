#pragma once
#include "skeet/misc/fov.hpp"
#include "skeet/effects/view_dispatch.hpp"

namespace skeet::misc {
class FovOwner;
struct FovOwnerBindings {
    const ZoomSensitivityBindings& sensitivity;
    void* const& entity_list; //live4346A890
    effects::ViewFovOffsets view_offsets;
};
std::size_t fov_owner_size() noexcept;
// Whole4347DDCD: primary append, update append, both vtable assignments, then
// FC225A4F=90 and CA80F9ED=100. Configuration must be unregistered canonical
// storage. The primary slot0 consumes a native43387448 allocation header.
FovOwner& construct_fov_owner(void* storage, engine::OwnerRegistryBinding lifecycle,
                              engine::OwnerRegistryBinding updates, ConfigRegistries&, FovConfig&,
                              FovOwnerBindings);
ZoomSensitivityRuntime& fov_sensitivity_runtime(FovOwner&) noexcept;
effects::ViewFovRuntime& fov_view_runtime(FovOwner&) noexcept;
} //namespace skeet::misc
