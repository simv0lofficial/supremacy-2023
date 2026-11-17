#pragma once
#include "skeet/combobox.hpp"
#include <bit>
#include <intrin.h>

namespace skeet::skins {
// Complete43398ECF. This atomic setter is used by glove-paint replacement and
// automatic LEGIT group selection; ComboBox::set_value is the distinct433F49B3.
inline void set_combo_value_atomic(ui::ComboBox& combo, std::uint32_t requested) {
    const auto previous = combo.value ? *combo.value : 0;
    if (previous == requested)
        return;
    if (combo.value) {
        static_assert(sizeof(long) == sizeof(std::uint32_t));
        _InterlockedExchange(reinterpret_cast<volatile long*>(combo.value),
                             std::bit_cast<long>(requested));
    }
    combo.value_changed();
}
} //namespace skeet::skins
