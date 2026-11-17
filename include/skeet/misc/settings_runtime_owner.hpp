#pragma once
#include "settings_runtime.hpp"

namespace skeet {
class ConfigStorageOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
}
namespace skeet::ui {
class MainMenuWindowOwner;
class RootMenu;
}

namespace skeet::misc {
// Canonical settings-menu callbacks. Construct after the real early window,
// CONFIG and selected unload request contexts exist. RootMenu itself binds
// the category targets, fake-lag slider and setting widgets later.
// Keep this owner and its borrowed contexts alive while those callbacks run.
class SettingsRuntimeOwner {
public:
    SettingsRuntimeOwner(client::ClientRuntimeStorage&, client::ConfigurationRuntimeOwner&,
                         ui::MainMenuWindowOwner&, ConfigStorageOwner&, UnloadRuntime&);
    SettingsRuntimeOwner(const SettingsRuntimeOwner&) = delete;
    SettingsRuntimeOwner& operator=(const SettingsRuntimeOwner&) = delete;
    SettingsRuntimeOwner(SettingsRuntimeOwner&&) = delete;
    SettingsRuntimeOwner& operator=(SettingsRuntimeOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    ui::MainMenuWindowOwner& window() const noexcept {
        return window_;
    }
    ConfigStorageOwner& configuration() const noexcept {
        return configuration_;
    }
    SettingsRuntime& runtime() noexcept {
        return runtime_;
    }
    // Complete433ACDC7 producer: capture the actual constructed menu grids
    // into43468CB0 and43476B34/38 once. The startup caller supplies its proven
    // invocation cut; no captured process layout is substituted here.
    void capture_default_layout(ui::RootMenu&);
    bool default_layout_captured() const noexcept {
        return layout_captured_;
    }

private:
    client::ClientRuntimeStorage& storage_;
    ui::MainMenuWindowOwner& window_;
    ConfigStorageOwner& configuration_;
    SettingsRuntime runtime_;
    bool layout_captured_ = false;
};
} // namespace skeet::misc
