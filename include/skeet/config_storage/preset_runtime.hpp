#pragma once
#include "skeet/config_storage/presets.hpp"
#include "skeet/menu/rage_clipboard.hpp"
#include "skeet/script/bridge.hpp"
#include "skeet/events/event_name_storage.hpp"

namespace skeet::script {
class ScriptManager;
class UiBindings;
}
namespace skeet::ui {
class RootMenu;
}
namespace skeet {
class SettingsPersistence;

// Concrete client lifecycle around preset serialization. The menu is attached
// after construction because its Presets group itself borrows the manager.
class PresetRuntime final : public ConfigPresetsServices {
public:
    PresetRuntime(ui::MenuWindow&, script::Bridge&, script::UiBindings&, SettingsPersistence&,
                  ui::RageClipboardApi&);
    // Explicit second phase, sharing the actual manager and its shutdown span.
    // Reading script defaults before this binding is an error.
    void bind_scripts(ConfigPresetsManager&, script::ScriptManager&);
    void bind_menu(ui::RootMenu&, ConfigPresetsManager&, script::ScriptManager&);
    void flush_active_weapon_group() override;
    void saving_event(bool before) override;
    void before_baseline_load() override;
    void after_baseline_load() override;
    void after_values_load() override;
    void loaded_event() override;
    void restore_script_defaults() override;
    void after_named_load() override;
    void persist_settings() override;
    std::string read_clipboard_export() override;
    void write_clipboard_export(std::string_view) override;

private:
    ui::MenuWindow& window_;
    script::Bridge& bridge_;
    script::UiBindings& script_ui_;
    ConfigStreamView script_defaults_ = ConfigStreamView::unbound();
    SettingsPersistence& persistence_;
    ui::RageClipboardApi& clipboard_;
    ui::RageWeaponGroupState* rage_ = nullptr;
    const std::uint32_t* selected_group_ = nullptr;
    ConfigPresetsManager* presets_ = nullptr;
    void event(events::EventName name);
    ui::RageWeaponGroupState& rage() const;
    const bool& preserve_hotkey_toggle() const;
};
}
