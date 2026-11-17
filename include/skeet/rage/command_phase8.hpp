#pragma once
#include "skeet/client/command_frame.hpp"

namespace skeet::rage {
struct PreambleState;
// Complete secondary-vtable phase8 entry433B76DE. The same sixteen-frame
// history is consumed by prepare_rage_command; newest frame is frame_first.
void capture_rage_command_frame(PreambleState&, const client::CommandFrame&) noexcept;
} // namespace skeet::rage
