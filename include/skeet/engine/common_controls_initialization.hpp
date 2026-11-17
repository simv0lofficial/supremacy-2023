#pragma once
#include <cstddef>
#include <cstdint>

namespace skeet::engine {
struct ForwardedExportEnvironment;

struct CommonControlsExports {
    std::byte* default_subclass_proc = nullptr; // Native 0x43476910, ordinal 413.
    std::byte* remove_window_subclass = nullptr; // Native 0x43476914, ordinal 412.
    std::byte* set_window_subclass = nullptr; // Native 0x43476918, ordinal 410.
};

// Devirtualized startup imports. The fixed ordinal/destination pairs are
// independently confirmed by the working client's dump and PE export table.
// No protected metadata accessor or captured process address is called.
void initialize_startup_common_controls(CommonControlsExports& imports,
                                        const ForwardedExportEnvironment& environment);
}
