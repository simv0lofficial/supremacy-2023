#pragma once
#include "skeet/menu_window.hpp"
#include "skeet/menu/rage_clipboard.hpp"
#include "skeet/menu/misc_menu.hpp"
#include "skeet/menu/scripts_menu.hpp"
#include <memory>

namespace skeet {
class ConfigPresetsManager;
}
namespace skeet::effects {
class CameraRuntime;
}
namespace skeet::misc {
class Runtime;
class SettingsRuntime;
}
namespace skeet::skins {
class NativeCatalog;
class Refresh;
}
namespace skeet::players {
struct State;
struct Bindings;
}
namespace skeet::script {
class ScriptModulesOwner;
class ScriptOwner;
}
namespace skeet::input {
class Runtime;
}
namespace skeet::rage {
class GroupStorage;
}

namespace skeet::ui {
class IconCache;
class RendererContainer;
class RageWeaponMenu;
class SkinsMenu;
class PlayersMenu;
class LegitMenu;
class ConfigPresetsMenu;
// Native 43342275 transfers the tab into the window's owning vector.
Tab& append_menu_tab(MenuWindow& window, std::unique_ptr<Tab> tab);

struct RootMenuLayoutTargets {
    // RAGE Aimbot/Other; AA angles/Other/Fake lag; LEGIT Aimbot/Triggerbot/Other;
    // VISUALS Player/Colored models/Other/Effects; MISC Miscellaneous/Movement/
    // Settings; CONFIG Presets/Lua. This is the native capture order.
    std::array<Category*, 17> categories{};
    std::array<Category*, 2> players{};
};
void capture_menu_layout(const RootMenuLayoutTargets& targets,
                         std::array<std::int32_t, 21>& destination, bool built_in,
                         std::array<std::uint32_t, 2>& player_defaults);
// 433815C4 reads engine slot26 and the decoded43468EA4 ConVar parent chain.
void update_rage_hit_chance_visibility(Widget& hit_chance, void* engine,
                                       const void* accuracy_variable);

enum class RootTab : std::size_t {
    rage,
    anti_aim,
    legit,
    visuals,
    misc,
    skins,
    players,
    config,
    lua,
    count
};
struct RootMenuGlobals {
    std::int32_t previous_max_usercmd_ticks = 0; // 4347CA3C
};
struct RootMenuBindings {
    ConfigRegistries& configuration;
    WidgetIdentities& identities;
    rage::GroupStorage& rage_groups;
    ConfigStreamView baseline_a, baseline_b;
    RageClipboardApi& clipboard;
    effects::CameraRuntime& camera;
    misc::Runtime& miscellaneous;
    SettingsMenuValues settings_values;
    misc::SettingsRuntime& settings_runtime;
    std::uint8_t& upgrade_tablet;
    bool expose_tablet;
    skins::NativeCatalog& skins_catalog;
    skins::Refresh& skins_refresh;
    players::State& player_state;
    players::Bindings& player_bindings;
    ConfigPresetsManager& presets;
    ScriptsMenuValues script_values;
    script::ScriptOwner& scripts;
    IconCache& icons;
    GuiDpiState& dpi;
    RendererContainer& renderer;
    RootMenuGlobals& globals;
    void* engine;
    const void* accuracy_variable; // decoded43468EA4 ConVar
    input::Runtime& input;
};

// 4348704A..4348D40C. The initialized window owns the allocated concrete tabs.
// Retain this controller while its navigation span and menu accessors are used;
// destroy_main_menu_window deletes the tabs and their concrete menu groups.
class RootMenu {
public:
    RootMenu(MenuWindow& window, RootMenuBindings bindings);
    ~RootMenu();
    RootMenu(const RootMenu&) = delete;
    RootMenu& operator=(const RootMenu&) = delete;
    Tab& tab(RootTab index) const;
    RageWeaponMenu& rage_weapon_menu() const;
    RageWeaponGroupState& rage_group_state() const;
    std::uint32_t& selected_rage_group() const;
    SkinsMenu& skins_menu() const;
    LegitMenu& legit_menu() const;
    PlayersMenu& players_menu() const;
    ConfigPresetsMenu& presets_menu() const;
    ConfigPresetsManager& preset_manager() const noexcept;
    ScriptsMenu& scripts_menu() const;
    script::ScriptModulesOwner& script_modules() const noexcept;
    script::ScriptOwner& script_owner() const noexcept;
    const std::uintptr_t& native_window() const noexcept;
    MenuWindow& window() const noexcept;
    void initialize_renderer_resources(RendererContainer& renderer); //43485A05
    std::uint32_t tab_atlas_texture() const noexcept;
    RootMenuLayoutTargets layout_targets() const;
    void capture_layout(std::array<std::int32_t, 21>& destination);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
}
