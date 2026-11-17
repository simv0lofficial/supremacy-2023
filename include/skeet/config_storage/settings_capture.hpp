#pragma once
#include "skeet/config_storage/settings_persistence.hpp"

namespace skeet::ui {
class MenuWindow;
class RootMenu;
struct SettingsMenuValues;
struct ScriptsMenuValues;
}
namespace skeet::script {
class ScriptManager;
}

namespace skeet {
// Concrete live-input portion of433EEC8B. The settings owner/serializer retains
// its stored values; this object refers to the same globals used by the menus.
// Bind after RootMenu construction, before startup or any persistent callback.
class MenuSettingsCapture {
public:
    MenuSettingsCapture(const ui::MenuWindow&, ui::SettingsMenuValues,
                        ui::ScriptsMenuValues) noexcept;
    MenuSettingsCapture(const MenuSettingsCapture&) = delete;
    MenuSettingsCapture& operator=(const MenuSettingsCapture&) = delete;
    void bind_menu(const ui::RootMenu&, const script::ScriptManager&);
    SettingsPersistenceInputs operator()() const;

private:
    const ui::MenuWindow& window_;
    const std::uint32_t& menu_color_; //43468FB0
    const RegisteredHotkey& menu_key_; //binding.packed43478DC8
    const std::uint8_t& lock_layout_; //43467E4B
    const std::int32_t& dpi_scale_; //43475A94
    const std::uint8_t& allow_unsafe_; //43475798
    const ui::RootMenu* menu_ = nullptr;
    const script::ScriptManager* scripts_ = nullptr;
};
}
