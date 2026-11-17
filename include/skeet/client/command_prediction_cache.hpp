#pragma once
#include "skeet/lagcomp/entity_state.hpp"

namespace skeet::client {
struct CommandBoneBackup {
    const Matrix3x4* storage;
    std::int32_t count;
};
// Inline433D569F..433D575F. Native copies the live cache count when it is at
// least the captured count. The backing storage must cover that live count,
// including retained capacity after a smaller captured logical size.
void restore_command_prediction_cache(std::uint32_t frame_flags, void* player,
                                      const lagcomp::EntityNetvars&, const CommandBoneBackup&);
} // namespace skeet::client
