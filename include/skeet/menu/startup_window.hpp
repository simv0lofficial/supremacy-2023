#pragma once
#include "skeet/menu/window_lifetime.hpp"
#include "skeet/menu/startup_window_name.hpp"

namespace skeet::client {
class ClientRuntimeStorage;
}
namespace skeet {
class ConfigStorageOwner;
}
namespace skeet::engine {
class BuildStartupConVarBindings;
}
namespace skeet::effects {
class CameraRuntime;
}
namespace skeet::misc {
class Runtime;
class SettingsRuntime;
}
namespace skeet::rage {
class GroupStorage;
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
class ScriptOwner;
}
namespace skeet::ui {
class MenuStartupData;
class RootMenu;
class StartupRenderer;
class IconCache;

// Borrow the actual feature lifetimes already constructed by source startup.
// Raw menu settings, baselines, HWND, DPI and engine globals are derived from
// their canonical owners by compose(); callers do not supply another copy.
struct MainMenuFeatureOwners {
    WidgetIdentities& identities;
    rage::GroupStorage& rage_groups;
    effects::CameraRuntime& camera;
    misc::Runtime& miscellaneous;
    misc::SettingsRuntime& settings_runtime;
    skins::NativeCatalog& skins_catalog;
    skins::Refresh& skins_refresh;
    players::State& players;
    players::Bindings& player_bindings;
    ConfigStorageOwner& configuration;
    script::ScriptOwner& scripts;
    IconCache& icons;
};
#if defined(_M_IX86) || defined(__i386__)
using FindMainGameWindow = std::uintptr_t(__stdcall*)(const char*, const char*);
#else
using FindMainGameWindow = std::uintptr_t (*)(const char*, const char*);
#endif
struct MainMenuWindowImports {
    FindMainGameWindow find_window;
};
MainMenuWindowImports native_main_menu_window_imports();

// Stable early window lifetime. CONFIG and ScriptOwner borrow window() before
// their later RootMenu controller is constructed. The real HWND is resolved
// once by the supplied loader's exact Valve001/class-title query; consumers
// borrow this field through hook removal and the final timer cancellation.
// Keep this owner alive until ShutdownRuntime and UnloadRuntime are gone.
class MainMenuWindowOwner {
public:
    explicit MainMenuWindowOwner(client::ClientRuntimeStorage&);
    MainMenuWindowOwner(client::ClientRuntimeStorage&, MainMenuWindowImports);
    ~MainMenuWindowOwner();
    MainMenuWindowOwner(const MainMenuWindowOwner&) = delete;
    MainMenuWindowOwner& operator=(const MainMenuWindowOwner&) = delete;
    MainMenuWindowOwner(MainMenuWindowOwner&&) = delete;
    MainMenuWindowOwner& operator=(MainMenuWindowOwner&&) = delete;
    MenuWindow& window() noexcept {
        return window_;
    }
    WindowCallbackStorage& callbacks() noexcept {
        return callbacks_;
    }
    MainMenuRegistryName& registry_name() noexcept {
        return registry_name_;
    }
    std::span<std::uint32_t, 128> startup_scripts() noexcept {
        return startup_scripts_;
    }
    std::span<const std::uint32_t, 128> startup_scripts() const noexcept {
        return startup_scripts_;
    }
    MenuStartupData& menu_data() const noexcept;
    RootMenu& compose(MainMenuFeatureOwners, StartupRenderer&,
                      const engine::BuildStartupConVarBindings&);
    RootMenu& controller() const;
    // Source-only controller links must be detached before primary slot0
    // frees ScriptOwner. Native tabs remain owned until release_gui's cut.
    void release_controller() noexcept;
    void release_gui(WindowTitleString&, const WindowLifetimeImports&) noexcept;
    static constexpr Point initial_size{660, 560}; //actual VM call4352219E
private:
    client::ClientRuntimeStorage& storage_;
    WindowCallbackStorage callbacks_;
    MenuWindow window_;
    // Native derived-constructor phase writes this32-unit buffer. Its producer
    // must finish before binding it to source settings persistence.
    MainMenuRegistryName registry_name_;
    std::array<std::uint32_t, 128> startup_scripts_{}; //actual VM zeroes main-window+120..+320
    std::unique_ptr<RootMenu> controller_;
    bool window_alive_ = false;
    void release_window_views() noexcept;
};
} //namespace skeet::ui
