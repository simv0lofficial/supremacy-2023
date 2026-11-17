#pragma once
#include "skeet/client_menu.hpp"
#include "skeet/button.hpp"

namespace skeet::misc {
class Runtime;
class SettingsRuntime;
}

namespace skeet::ui {

// 0x4348B064..0x4348B5BB. The optional tablet control is constructed only
// when the native build's feature byte 0x434758D4 is nonzero.
class MiscellaneousMenu final : public BoundMenuControls {
public:
    MiscellaneousMenu(Tab&, ConfigRegistries&, WidgetIdentities&, misc::Runtime&,
                      std::uint8_t& upgrade_tablet, bool expose_tablet);
    ~MiscellaneousMenu();
    Category& category() noexcept {
        return category_;
    }
    const Category& category() const noexcept {
        return category_;
    }

private:
    static void auto_accept_changed(void*, Widget&);
    static void ping_spike_changed(void*, Widget&);
    static void upgrade_tablet_changed(void*, Widget&);
    static void steal_player_name(void*, Widget&);
    static void dump_matchmaking_wins(void*, Widget&);
    Tab& tab_;
    Category category_;
    misc::Runtime& runtime_;
    std::array<VisibilityLink, 4> dependencies_{};
};

// These settings alias native global cells. Multithreading is the value of
// descriptor434683EC; the other entries are raw cells or a registered hotkey.
// Their backing belongs to the caller, as do the four menu booleans.
struct SettingsMenuValues {
    RegisteredHotkey& menu_key; // 0x43478DB8
    std::uint32_t& menu_color; // 0x43468FB0
    std::int32_t& dpi_scale; // 0x43475A94
    std::uint8_t& lock_layout; // 0x43467E4B
    std::uint8_t& multithreading; // 0x434683FC
    std::int32_t& maximum_unlag; // 0x43468230
    std::int32_t& maximum_usercmd_ticks; // 0x43469274
    std::int32_t& clock_correction; // 0x43468290
    std::uint8_t& hold_aim; // 0x43468D7C
    std::uint8_t& fix_viewmodel_lag; // 0x4346A328
    std::uint8_t& fix_networking; // 0x43467CC0
    std::uint8_t& fix_animations; // 0x4346A527
    std::uint8_t& faster_grenade_toss; // 0x43467560
};

// 0x4348BA55..0x4348BF4D. The native constructor creates the debug controls
// and then hides its ten-element vector; configuration loading keeps them
// hidden because they have no visibility links.
class SettingsMenu final : public BoundMenuControls {
public:
    SettingsMenu(Tab&, ConfigRegistries&, WidgetIdentities&, SettingsMenuValues,
                 misc::SettingsRuntime&);
    ~SettingsMenu();
    Category& category() noexcept {
        return category_;
    }
    const Category& category() const noexcept {
        return category_;
    }
    Slider* max_usercmd_ticks_slider() noexcept {
        return maximum_usercmd_ticks_;
    }

private:
    static void dpi_scale_changed(void*, Widget&);
    static void allow_custom_events_changed(void*, Widget&);
    static void lock_layout_changed(void*, Widget&);
    static void reset_layout(void*, Widget&);
    static void unload(void*, Widget&);
    static void maximum_unlag_changed(void*, Widget&);
    static void maximum_usercmd_ticks_changed(void*, Widget&);
    Tab& tab_;
    Category category_;
    misc::SettingsRuntime& runtime_;
    Slider* maximum_usercmd_ticks_ = nullptr; // native MISC Tab +0xD8
};

} // namespace skeet::ui
