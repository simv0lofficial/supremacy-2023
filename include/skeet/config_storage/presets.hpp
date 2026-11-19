#pragma once
#include "skeet/config_storage/named_storage.hpp"
#include "skeet/config_storage/load_state.hpp"
#include "skeet/config_storage/config_stream_view.hpp"
#include "skeet/config_storage/settings_persistence.hpp"
#include <memory>

namespace skeet::ui {
class ConfigPresetsMenu;
}

namespace skeet {
class PreservedConfigStorage;

struct ConfigPresetEntry {
    std::u16string name;
    std::uint32_t marker = 0;
    RegisteredHotkey* hotkey = nullptr;
};

// Engine, script, window-refresh and clipboard boundaries surrounding the
// recovered preset controller. Implementations bind the existing runtime owners.
class ConfigPresetsServices {
public:
    virtual ~ConfigPresetsServices() = default;
    virtual void flush_active_weapon_group() = 0; // 433EC9A4
    virtual void saving_event(bool before) = 0; // script events around 433B5919
    virtual void before_baseline_load() = 0; // 4340B293
    virtual void after_baseline_load() = 0; // 433AB1C2
    virtual void after_values_load() = 0; // engine/window coordination through 4338A0E9
    virtual void loaded_event() = 0; // 43349E9A, ordered around marker refresh
    virtual void restore_script_defaults() = 0; // 4337ECFE script-control subset
    virtual void after_named_load() = 0; // 4332F0AC
    virtual void persist_settings() = 0; // SettingsPersistence::persist(0,0)
    virtual std::string read_clipboard_export() = 0;
    virtual void write_clipboard_export(std::string_view text) = 0;
    // Complete 4337ECFE, also used by restricted Lua configuration loads.
    void restore_defaults(ConfigLink&, std::span<const std::byte> baseline,
                          const bool& preserve_hotkey_toggle);
    void restore_defaults(ConfigLink&, PreservedConfigStorage&, const bool& preserve_hotkey_toggle);
};

std::uint32_t config_preset_hash(std::u16string_view name) noexcept;
std::string config_name_to_utf8(std::u16string_view name);
std::u16string config_name_from_utf8(std::string_view name);

class ConfigPresetsManager {
public:
    ConfigPresetsManager(ConfigRegistries& registries, NamedConfigStore& store,
                         std::span<const std::byte> baseline, ConfigPresetsServices& services,
                         SettingsStateBinding settings = {});
    ConfigPresetsManager(ConfigRegistries&, NamedConfigStore&, PreservedConfigStorage&,
                         ConfigPresetsServices&);
    ~ConfigPresetsManager();
    ConfigPresetsManager(const ConfigPresetsManager&) = delete;
    ConfigPresetsManager& operator=(const ConfigPresetsManager&) = delete;

    std::u16string name;
    std::uint32_t selected_index = 0;
    std::uint32_t active_hash = 0; // 4347C9F8
    bool& preserve_hotkey_toggle = config_loading_flag(); // native 43475A98

    std::span<const ConfigPresetEntry> entries() const noexcept {
        return entries_;
    }
    RegisteredHotkey* selected_hotkey() noexcept;
    // A bound native menu rebuilds its list and reapplies selection on refresh.
    // select copies a name only for an existing entry of at most 63 units.
    void refresh();
    void update_markers() noexcept;
    void select(std::uint32_t index);
    bool load();
    bool save();
    void erase();
    std::span<const std::byte> baseline_bytes() const;
    ConfigStreamView baseline_view() const;
    void restore_default_values(); //4337ECFE, retaining the current active hash/markers
    void reset();
    bool import_from_clipboard();
    bool export_to_clipboard();
    bool selected_for_startup() const;
    bool set_selected_startup(bool enabled);
    bool load_startup();

private:
    friend class ui::ConfigPresetsMenu;
    friend class PresetRuntime;
    struct PresetBinding {
        RegisteredHotkey hotkey;
        ConfigPresetsManager* manager;
    };
    static void SKEET_HOTKEY_CALLBACK preset_hotkey(void* context, RegisteredHotkey& hotkey);
    void apply(std::span<const std::byte> stream, bool defer_loaded_event = false);
    static void unlink(RegisteredHotkey& hotkey) noexcept;
    void mark_entries() noexcept;
    ConfigRegistries& registries_;
    NamedConfigStore& store_;
    std::span<const std::byte> baseline_;
    PreservedConfigStorage* preserved_ = nullptr;
    ConfigPresetsServices& services_;
    SettingsStateBinding settings_;
    ui::ConfigPresetsMenu* menu_ = nullptr;
    std::vector<ConfigPresetEntry> entries_;
    std::vector<std::unique_ptr<PresetBinding>> bindings_;
};

} // namespace skeet
