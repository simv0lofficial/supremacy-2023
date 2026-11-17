#pragma once
#include "skeet/client_menu.hpp"
#include "skeet/button.hpp"
#include "skeet/listbox.hpp"
#include "skeet/textbox.hpp"

namespace skeet {
class ConfigPresetsManager;
}

namespace skeet::ui {

// CONFIG / Presets, 0x4348CF67..0x4348D102. The manager and supplied tab
// outlive the controls. The native list has no registered setting pointer.
class ConfigPresetsMenu final : public BoundMenuControls {
public:
    ConfigPresetsMenu(Tab&, WidgetIdentities&, ConfigPresetsManager&);
    ~ConfigPresetsMenu();
    Category& category() noexcept {
        return category_;
    }
    const Category& category() const noexcept {
        return category_;
    }
    Listbox& list() noexcept {
        return *list_;
    }
    HotkeyWidget& hotkey() noexcept {
        return *hotkey_;
    }
    Textbox& name_editor() noexcept {
        return *name_;
    }
    // 0x4335C747 rebuilds the list and reapplies its selected row.
    void refresh();
    void update_markers() noexcept; // 0x433971CD: preserves selection/edit buffer.
    void load_hotkey(RegisteredHotkey&); // 0x4336750C: actual UI selection before load.
private:
    friend class skeet::ConfigPresetsManager;
    static void selection_changed(void*, Widget&);
    static void load(void*, Widget&);
    static void save(void*, Widget&);
    static void erase(void*, Widget&);
    static void reset(void*, Widget&);
    static void import_from_clipboard(void*, Widget&);
    static void export_to_clipboard(void*, Widget&);
    void read_name();
    void synchronize_from_manager();
    void synchronize_selection();
    Tab& tab_;
    Category category_;
    ConfigPresetsManager& manager_;
    Listbox* list_ = nullptr;
    HotkeyWidget* hotkey_ = nullptr;
    Textbox* name_ = nullptr;
};

} // namespace skeet::ui
