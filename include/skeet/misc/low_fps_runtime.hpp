#pragma once
#include "skeet/client/command_frame.hpp"
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/rage/settings.hpp"

namespace skeet::hud {
struct PerformanceHudBinding;
}
namespace skeet::esp {
struct IndicatorLayout;
}
namespace skeet::script {
struct Bridge;
}

namespace skeet::misc {
#if defined(_M_IX86) || defined(__i386__)
using LowFpsClockQuery = std::int32_t(__stdcall*)(std::int64_t*);
#else
using LowFpsClockQuery = std::int32_t (*)(std::int64_t*);
#endif
struct LowFpsClock {
    LowFpsClockQuery counter; // recorded ntdll!RtlQueryPerformanceCounter
    LowFpsClockQuery frequency; // recorded ntdll!RtlQueryPerformanceFrequency
};
LowFpsClock native_low_fps_clock();

struct LowFpsBindings {
    const std::int32_t& frame_number; // **434678F8
    const float& tick_interval; // **4346A338
    // Optional startup records4346D560/4346D55C. Slot1 copies their+4 words;
    // neither word is consumed by the three native command methods.
    void* const& startup_first;
    void* const& startup_second;
    LowFpsClock clock;
};

// Semantic state of the one complete owner4347E6F2, not a second settings
// block. The native constructor deliberately leaves counters/history intact.
struct LowFpsState {
    float level; // complete+3C
    std::uint8_t low_fps; // complete+40
    const std::uint8_t* rage_enabled; // complete+44, published by slot1
    std::uint32_t startup_first_word; // complete+48
    std::uint32_t startup_second_word; // complete+4C
    std::uint64_t started_counter; // complete+50
    double counter_frequency; // complete+58
    double accumulated_seconds; // complete+60
    std::int32_t frame_number; // complete+68
    std::array<float, 3> previous_origin; // complete+6C
};

class LowFpsRuntime {
public:
    LowFpsRuntime(rage::PerformanceSettings&, const rage::GlobalSettings&, LowFpsBindings);
    rage::PerformanceSettings& config;
    const rage::GlobalSettings& rage;
    LowFpsBindings bindings;
    LowFpsState state;

    // Sparse4347E763..E793, called AFTER the three owner registrations and
    // the canonical descriptor initializer. Does not reset frame/history.
    void initialize();
    void bind_dependencies(); //4347E0FB, primary slot1
    void reset() noexcept; //433F2AC2, primary slot5 and slot7's433CF2F5
    bool active(InputBitmap) const; //433E8437
    void phase1(InputBitmap); //433654D5
    void phase4(client::CommandFrame&); //433229CE
    void phase7(const client::CommandFrame&, InputBitmap); //433B22CB
    static std::int32_t phase9(std::int32_t limit) noexcept {
        return limit;
    }
};

inline constexpr std::uint32_t low_fps_constructor = 0x4347E6F2u;
inline constexpr std::uint32_t low_fps_lifecycle_vtable = 0x4346EF74u;
inline constexpr std::uint32_t low_fps_command_vtable = 0x43472148u;
inline constexpr std::uint32_t low_fps_hud_vtable = 0x4346F010u;
hud::PerformanceHudBinding make_low_fps_hud_binding(LowFpsRuntime&, script::Bridge&,
                                                    const InputBitmap&, const esp::IndicatorLayout&,
                                                    const std::uint32_t& life_state_offset);

class LowFpsLifecycleOwner;
// Complete constructor composition. settings must be the UNREGISTERED
// canonical rage::PerformanceSettings, and must not also be passed through
// initialize_performance_settings/initialize_settings. This helper registers
// lifecycle -> command -> HUD, invokes that existing descriptor initializer
// once, and only then performs the sparse state/frequency initialization.
// The borrowed runtime pointer is cleared by lifecycle slot0; slot1 binds
// its real shared RAGE descriptor. Native slots2/3/4/6/8 are exactly RET.
LowFpsLifecycleOwner* initialize_low_fps_owner(
    engine::OwnerRegistryBinding lifecycle, client::CommandPhaseRegistry& command_owners,
    engine::OwnerRegistryBinding hud_owners, ConfigRegistries&, rage::PerformanceSettings&,
    const rage::GlobalSettings&, LowFpsBindings, script::Bridge&, const InputBitmap&,
    const esp::IndicatorLayout&, const std::uint32_t& life_state_offset, LowFpsRuntime*& active);
} // namespace skeet::misc
