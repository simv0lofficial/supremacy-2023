#pragma once
#include "skeet/menu_window.hpp"
#include <cstdint>

namespace skeet {
class SettingsPersistence;
}
namespace skeet::engine {
class NativeDataImage;
}
namespace skeet::misc {
class UnloadRuntime;
struct ResetLayoutTargets {
private:
    std::array<std::uint32_t, 2> standalone_defaults_{};

public:
    ResetLayoutTargets() noexcept : additional_defaults(standalone_defaults_) {}
    explicit ResetLayoutTargets(std::array<std::uint32_t, 2>& defaults) noexcept
        : additional_defaults(defaults) {}
    ResetLayoutTargets(const ResetLayoutTargets&) = delete;
    ResetLayoutTargets& operator=(const ResetLayoutTargets&) = delete;
    ResetLayoutTargets(ResetLayoutTargets&&) = delete;
    ResetLayoutTargets& operator=(ResetLayoutTargets&&) = delete;
    // Source order is fixed by 0x43342FF1. Native +0x70 stores each category's
    // packed four-byte grid. All targets are live, owned menu categories.
    std::array<ui::Category*, 15> categories{};
    std::array<ui::Category*, 2> additional_categories{};
    std::array<std::uint32_t, 2>& additional_defaults; //43476B34/38
};
class SettingsRuntime {
    std::int32_t standalone_previous_dpi_ = 0;
    std::uint8_t standalone_previous_lock_ = 1, standalone_previous_custom_events_ = 0;
    void* standalone_engine_ = nullptr;
    void* standalone_event_manager_ = nullptr;
    std::array<std::int32_t, 21> standalone_layout_{9};

public:
    SettingsRuntime() noexcept;
    SettingsRuntime(std::int32_t& previous_dpi, std::uint8_t& previous_lock,
                    std::uint8_t& previous_custom_events, void*& engine, void*& event_manager,
                    std::array<std::int32_t, 21>& default_layout,
                    std::array<std::uint32_t, 2>& additional_defaults) noexcept;
    SettingsRuntime(const SettingsRuntime&) = delete;
    SettingsRuntime& operator=(const SettingsRuntime&) = delete;
    SettingsRuntime(SettingsRuntime&&) = delete;
    SettingsRuntime& operator=(SettingsRuntime&&) = delete;
    ui::GuiDpiState* gui_dpi = nullptr;
    ui::MenuWindow* main_menu = nullptr;
    ui::Slider* fakelag_limit = nullptr;
    ResetLayoutTargets reset_targets;
    skeet::SettingsPersistence* persistence = nullptr;
    UnloadRuntime* unload = nullptr;
    const std::int32_t* dpi_scale = nullptr;
    const std::uint8_t* lock_layout = nullptr;
    const std::uint8_t* allow_custom_events = nullptr;
    std::int32_t& previous_dpi_scale; // live4347CA04
    std::uint8_t& previous_lock_layout; // live4346D550
    std::uint8_t& previous_custom_events; // live4347CA38
    void*& engine; // live4346A514
    void*& custom_event_state; // live43468364, byte +A0
    std::array<std::int32_t, 21>& default_layout; // live43468CB0

    void dpi_scale_changed(); // source reset precedes this, 0x433379D4
    void lock_layout_changed(); // complete 0x4332A6C5 coordination
    void allow_custom_events_changed(); // complete 0x4333D25B
    void reset_menu_layout(); // 0x43342FF1, native built-in layout block
    void request_unload(); // 0x433A0637 coordination
    void max_usercmd_ticks_changed(std::int32_t value); // complete 0x433EE188
};

// Exact original static bytes for a new zeroed source image. The layout starts
// with tag9 and zero records;433ACDC7 later captures real constructed grids.
// Binding a runtime never repeats this initialization over live settings.
void initialize_source_settings_runtime_data(engine::NativeDataImage&);
} // namespace skeet::misc
