#pragma once
#include "named_storage.hpp"
#include "settings_persistence.hpp"
#include <memory>

namespace skeet::script {
struct Bridge;
class UiBindings;
class ScriptManager;
}
namespace skeet::ui {
class MenuWindow;
class RootMenu;
struct SettingsMenuValues;
struct ScriptsMenuValues;
struct RageClipboardApi;
}

namespace skeet {

struct NativeConfigPath {
    std::uint16_t length = 0, maximum_length = 0;
    char16_t* buffer = nullptr;
};
// Shared ordinary433EBC7E reader; the skins schema and preset store pass their
// already initialized NT names to this same body.
std::optional<ConfigBytes> read_native_file(const NativeConfigPath&);

// The path is the already initialized native434798CC NT name. Borrowing the
// actual name cell preserves its lifetime and replacement; no path is chosen
// by these operations.
class NativeNamedConfigStorage final : public NamedConfigStorageBackend {
public:
    explicit NativeNamedConfigStorage(const NativeConfigPath& native_path) noexcept
        : native_path_(native_path) {}
    std::optional<ConfigBytes> read_container() override; //433EBC7E
    void write_container(std::span<const std::byte>) override; //433F9C97/WriteFile
    void remove_container() override; //433E1D67
private:
    const NativeConfigPath& native_path_;
};

// Concrete OS boundaries consumed by SettingsPersistence4340B9C9. Its module
// predicate and snapshot serializer remain the existing recovered source.
class NativeSettingsPersistenceBackend final : public SettingsPersistenceBackend {
public:
    void enumerate_loaded_modules(ModuleVisitor, void*) override;
    void write_registry_value(std::uint32_t relative_to, std::u16string_view path,
                              std::u16string_view value_name, std::uint32_t type,
                              std::span<const std::byte> data) override;
};

class ConfigPresetsManager;
class PresetRuntime;
class MenuSettingsCapture;
class PreservedConfigStorage;

// Production composition borrows the canonical settings/path cells without
// requiring their later startup allocations. settings_state() still requires
// the real settings owner; it never constructs a substitute on first access.
// Script defaults are bound to the actual ScriptManager in bind_menu(), once
// both owners exist; no captured configuration buffer crosses construction.
class ConfigStorageOwner {
public:
    ConfigStorageOwner(ConfigRegistries&, std::shared_ptr<PreservedConfigStorage>, ui::MenuWindow&,
                       script::Bridge&, script::UiBindings&, ui::SettingsMenuValues,
                       ui::ScriptsMenuValues);
    ~ConfigStorageOwner();
    ConfigStorageOwner(const ConfigStorageOwner&) = delete;
    ConfigStorageOwner& operator=(const ConfigStorageOwner&) = delete;
    ConfigRegistries& configuration() noexcept;
    SettingsPersistenceState& settings_state();
    PreservedConfigStorage& native_storage() noexcept;
    NamedConfigStore& named() noexcept;
    ConfigPresetsManager& presets() noexcept;
    PresetRuntime& runtime() noexcept;
    MenuSettingsCapture& capture() noexcept;
    SettingsPersistence& persistence() noexcept;
    ui::RageClipboardApi& clipboard() noexcept;
    // Typed composition after RootMenu construction, before any native
    // lifecycle callback can persist settings or load a preset.
    void bind_menu(ui::RootMenu&, script::ScriptManager&);
    bool menu_bound_to(const ui::RootMenu&) const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace skeet
