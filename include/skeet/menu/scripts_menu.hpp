#pragma once
#include "skeet/client_menu.hpp"
#include "skeet/button.hpp"
#include "skeet/listbox.hpp"

namespace skeet::script {
class ScriptManager;
}

namespace skeet::ui {

struct ScriptsMenuValues {
    std::uint8_t& enabled; // 0x434675D4
    std::uint8_t& effective_enabled; // 0x4346D544
    std::uint8_t& allow_unsafe; // 0x43475798
    std::uint8_t& effective_unsafe; // 0x43478C6E
    std::uint8_t& reload_requested; // 0x4347CA28
};

std::uint64_t script_menu_time(void*);
std::u16string script_update_age(std::uint64_t modified_time, std::uint64_t now);

// 0x43485BAD installs the Lua category's native resize override 0x43359167.
class ScriptsCategory final : public Category {
public:
    ScriptsCategory(Tab&, WidgetIdentities&);
    Listbox* scripts = nullptr;
    void resize(const SliderMetrics&) override;
};

// CONFIG / Lua, 0x4348D119..0x4348D2FF. Direct global bytes and script runtime
// records retain their original owners; this group registers no descriptors.
class ScriptsMenu final : public BoundMenuControls {
public:
    ScriptsMenu(Tab&, WidgetIdentities&, ScriptsMenuValues, script::ScriptManager&);
    ~ScriptsMenu();
    ScriptsCategory& category() noexcept {
        return category_;
    }
    const ScriptsCategory& category() const noexcept {
        return category_;
    }
    Listbox& list() noexcept {
        return *list_;
    }
    Checkbox& startup() noexcept {
        return *startup_;
    }
    const Label& updated_label() const noexcept {
        return *updated_;
    }
    void* clock_context = nullptr;
    std::uint64_t (*read_time)(void*) = script_menu_time;
    void refresh();
    void activate();
    // 0x433DF55B dispatch and the distinct startup controller 0x4341AABA.
    void initialize_startup(const char* command_line, std::span<const std::uint32_t, 128> hashes);

private:
    static void enabled_changed(void*, Widget&);
    static void unsafe_changed(void*, Widget&);
    static void reload_active(void*, Widget&);
    static void selection_changed(void*, Widget&);
    static void startup_changed(void*, Widget&);
    static void load(void*, Widget&);
    static void unload(void*, Widget&);
    void update_selection();
    Tab& tab_;
    ScriptsCategory category_;
    ScriptsMenuValues values_;
    script::ScriptManager& manager_;
    Listbox* list_ = nullptr;
    Label* updated_ = nullptr;
    Checkbox* startup_ = nullptr;
    Button* load_ = nullptr;
    Button* unload_ = nullptr;
};

} // namespace skeet::ui
