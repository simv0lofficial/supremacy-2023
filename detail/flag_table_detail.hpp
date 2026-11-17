#pragma once
#include "skeet/esp/flags.hpp"

namespace skeet::esp::detail {
// The single decoded value recipe is shared by the ordinary native-table
// constructor and the explicit snapshot helper. It owns no second table.
FlagEntry initial_flag_entry(std::size_t index) noexcept;
}
