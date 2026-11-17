#pragma once
#include "skeet/anti_aim/command_phase.hpp"
#include "skeet/anti_aim/precalculation.hpp"
#include "skeet/anti_aim/finisher.hpp"

namespace skeet::anti_aim {
// Native constructor stages separated by its string allocation.
void initialize_command_state_prefix(AngleStageState&, PrecalculationState&,
                                     AngleParameters&) noexcept;
void initialize_command_animation_state(AngleStageState&, FinisherState&) noexcept;
void initialize_command_pattern(AngleParameters&) noexcept;
// Runtime portion of owner43490435. The two state references must alias the
// supplied AngleStageState. Native-untouched fields retain their input bytes;
// reset_state is the separate native reset method, rather than this constructor.
void initialize_command_state(AngleStageState&, PrecalculationState&, FinisherState&,
                              AngleParameters&) noexcept;
} // namespace skeet::anti_aim
