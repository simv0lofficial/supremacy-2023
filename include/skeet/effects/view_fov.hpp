#pragma once
#include "skeet/misc/config.hpp"
#include <cstdint>

namespace skeet::effects {
struct ViewFovOffsets {
    std::uint32_t life_state, movement_type, observer_mode, observer_target, fov;
};
// Canonical owner4347DDCD, secondary slot6. Its origin/angles parameters are
// unused; effective observer selection and actual player slot333 stay here.
class ViewFovRuntime {
public:
    const misc::FovConfig& settings;
    void* const& entity_list; //live4346A890
    void* const* local_player;
    ViewFovOffsets offsets;
    void update(float& field_of_view); // complete4333FD39
};
} // namespace skeet::effects
