#pragma once
#include "skeet/engine/native_data_image.hpp"
#include "skeet/menu/misc_menu.hpp"
#include "skeet/menu/scripts_menu.hpp"
#include "skeet/menu_window.hpp"

namespace skeet::ui {
struct RootMenuGlobals;

// One process-lifetime view of the canonical menu inputs. This creates no
// descriptors, restores no defaults and produces no configuration stream.
// CRT/owner constructors still initialize the same source data cells.
class MenuStartupData {
    std::shared_ptr<engine::NativeDataImage> image_;

public:
    explicit MenuStartupData(std::shared_ptr<engine::NativeDataImage>);
    MenuStartupData(const MenuStartupData&) = delete;
    MenuStartupData& operator=(const MenuStartupData&) = delete;
    MenuStartupData(MenuStartupData&&) = delete;
    MenuStartupData& operator=(MenuStartupData&&) = delete;

    // Selection aliases43475A94. Renderer and scripting bindings borrow this
    // same DPI object, so no consumer retains an independent selection copy.
    GuiDpiState dpi;
    SettingsMenuValues settings_values() const;
    ScriptsMenuValues script_values() const;
    WindowEventState& window_events() const;
    RootMenuGlobals& root_globals() const;
    std::span<const std::byte> baseline_a() const; //43478CB8
    std::span<const std::byte> baseline_b() const; //43479770
    engine::NativeDataImage& data() const noexcept {
        return *image_;
    }
};
} //namespace skeet::ui
